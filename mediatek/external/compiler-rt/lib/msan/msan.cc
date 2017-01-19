//===-- msan.cc -----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of MemorySanitizer.
//
// MemorySanitizer runtime.
//===----------------------------------------------------------------------===//

#include "msan.h"
#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_libc.h"
#include "sanitizer_common/sanitizer_procmaps.h"
#include "sanitizer_common/sanitizer_stacktrace.h"
#include "sanitizer_common/sanitizer_symbolizer.h"

#include "interception/interception.h"

// ACHTUNG! No system header includes in this file.

using namespace __sanitizer;

// Globals.
static THREADLOCAL int msan_expect_umr = 0;
static THREADLOCAL int msan_expected_umr_found = 0;

static int msan_running_under_dr = 0;

SANITIZER_INTERFACE_ATTRIBUTE
THREADLOCAL u64 __msan_param_tls[kMsanParamTlsSizeInWords];

SANITIZER_INTERFACE_ATTRIBUTE
THREADLOCAL u32 __msan_param_origin_tls[kMsanParamTlsSizeInWords];

SANITIZER_INTERFACE_ATTRIBUTE
THREADLOCAL u64 __msan_retval_tls[kMsanRetvalTlsSizeInWords];

SANITIZER_INTERFACE_ATTRIBUTE
THREADLOCAL u32 __msan_retval_origin_tls;

SANITIZER_INTERFACE_ATTRIBUTE
THREADLOCAL u64 __msan_va_arg_tls[kMsanParamTlsSizeInWords];

SANITIZER_INTERFACE_ATTRIBUTE
THREADLOCAL u64 __msan_va_arg_overflow_size_tls;

SANITIZER_INTERFACE_ATTRIBUTE
THREADLOCAL u32 __msan_origin_tls;

static THREADLOCAL struct {
  uptr stack_top, stack_bottom;
} __msan_stack_bounds;

static THREADLOCAL bool is_in_symbolizer;
static THREADLOCAL bool is_in_loader;

extern "C" SANITIZER_WEAK_ATTRIBUTE const int __msan_track_origins;

int __msan_get_track_origins() {
  return &__msan_track_origins ? __msan_track_origins : 0;
}

extern "C" SANITIZER_WEAK_ATTRIBUTE const int __msan_keep_going;

namespace __msan {

static bool IsRunningUnderDr() {
  bool result = false;
  MemoryMappingLayout proc_maps(/*cache_enabled*/true);
  const sptr kBufSize = 4095;
  char *filename = (char*)MmapOrDie(kBufSize, __FUNCTION__);
  while (proc_maps.Next(/* start */0, /* end */0, /* file_offset */0,
                        filename, kBufSize, /* protection */0)) {
    if (internal_strstr(filename, "libdynamorio") != 0) {
      result = true;
      break;
    }
  }
  UnmapOrDie(filename, kBufSize);
  return result;
}

void EnterSymbolizer() { is_in_symbolizer = true; }
void ExitSymbolizer()  { is_in_symbolizer = false; }
bool IsInSymbolizer() { return is_in_symbolizer; }

void EnterLoader() { is_in_loader = true; }
void ExitLoader()  { is_in_loader = false; }

extern "C" {
SANITIZER_INTERFACE_ATTRIBUTE
bool __msan_is_in_loader() { return is_in_loader; }
}

static Flags msan_flags;

Flags *flags() {
  return &msan_flags;
}

int msan_inited = 0;
bool msan_init_is_running;

int msan_report_count = 0;

// Array of stack origins.
// FIXME: make it resizable.
static const uptr kNumStackOriginDescrs = 1024 * 1024;
static const char *StackOriginDescr[kNumStackOriginDescrs];
static atomic_uint32_t NumStackOriginDescrs;

static void ParseFlagsFromString(Flags *f, const char *str) {
  ParseCommonFlagsFromString(str);
  ParseFlag(str, &f->poison_heap_with_zeroes, "poison_heap_with_zeroes");
  ParseFlag(str, &f->poison_stack_with_zeroes, "poison_stack_with_zeroes");
  ParseFlag(str, &f->poison_in_malloc, "poison_in_malloc");
  ParseFlag(str, &f->exit_code, "exit_code");
  if (f->exit_code < 0 || f->exit_code > 127) {
    Printf("Exit code not in [0, 128) range: %d\n", f->exit_code);
    Die();
  }
  ParseFlag(str, &f->report_umrs, "report_umrs");
  ParseFlag(str, &f->verbosity, "verbosity");
  ParseFlag(str, &f->wrap_signals, "wrap_signals");

  // keep_going is an old name for halt_on_error,
  // and it has inverse meaning.
  f->halt_on_error = !f->halt_on_error;
  ParseFlag(str, &f->halt_on_error, "keep_going");
  f->halt_on_error = !f->halt_on_error;
  ParseFlag(str, &f->halt_on_error, "halt_on_error");
}

static void InitializeFlags(Flags *f, const char *options) {
  CommonFlags *cf = common_flags();
  cf->external_symbolizer_path = GetEnv("MSAN_SYMBOLIZER_PATH");
  cf->strip_path_prefix = "";
  cf->fast_unwind_on_fatal = false;
  cf->fast_unwind_on_malloc = true;
  cf->malloc_context_size = 20;
  cf->handle_ioctl = true;
  cf->log_path = 0;

  internal_memset(f, 0, sizeof(*f));
  f->poison_heap_with_zeroes = false;
  f->poison_stack_with_zeroes = false;
  f->poison_in_malloc = true;
  f->exit_code = 77;
  f->report_umrs = true;
  f->verbosity = 0;
  f->wrap_signals = true;
  f->halt_on_error = !&__msan_keep_going;

  // Override from user-specified string.
  if (__msan_default_options)
    ParseFlagsFromString(f, __msan_default_options());
  ParseFlagsFromString(f, options);
}

static void GetCurrentStackBounds(uptr *stack_top, uptr *stack_bottom) {
  if (__msan_stack_bounds.stack_top == 0) {
    // Break recursion (GetStackTrace -> GetThreadStackTopAndBottom ->
    // realloc -> GetStackTrace).
    __msan_stack_bounds.stack_top = __msan_stack_bounds.stack_bottom = 1;
    GetThreadStackTopAndBottom(/* at_initialization */false,
                               &__msan_stack_bounds.stack_top,
                               &__msan_stack_bounds.stack_bottom);
  }
  *stack_top = __msan_stack_bounds.stack_top;
  *stack_bottom = __msan_stack_bounds.stack_bottom;
}

void GetStackTrace(StackTrace *stack, uptr max_s, uptr pc, uptr bp,
                   bool fast) {
  if (!fast) {
    // Block reports from our interceptors during _Unwind_Backtrace.
    SymbolizerScope sym_scope;
    return stack->SlowUnwindStack(pc, max_s);
  }

  uptr stack_top, stack_bottom;
  GetCurrentStackBounds(&stack_top, &stack_bottom);
  stack->size = 0;
  stack->trace[0] = pc;
  stack->max_size = max_s;
  stack->FastUnwindStack(pc, bp, stack_top, stack_bottom);
}

void PrintWarning(uptr pc, uptr bp) {
  PrintWarningWithOrigin(pc, bp, __msan_origin_tls);
}

bool OriginIsValid(u32 origin) {
  return origin != 0 && origin != (u32)-1;
}

void PrintWarningWithOrigin(uptr pc, uptr bp, u32 origin) {
  if (msan_expect_umr) {
    // Printf("Expected UMR\n");
    __msan_origin_tls = origin;
    msan_expected_umr_found = 1;
    return;
  }

  ++msan_report_count;

  StackTrace stack;
  GetStackTrace(&stack, kStackTraceMax, pc, bp,
                common_flags()->fast_unwind_on_fatal);

  u32 report_origin =
    (__msan_get_track_origins() && OriginIsValid(origin)) ? origin : 0;
  ReportUMR(&stack, report_origin);

  if (__msan_get_track_origins() && !OriginIsValid(origin)) {
    Printf("  ORIGIN: invalid (%x). Might be a bug in MemorySanitizer, "
           "please report to MemorySanitizer developers.\n",
           origin);
  }
}

void UnpoisonParam(uptr n) {
  internal_memset(__msan_param_tls, 0, n * sizeof(*__msan_param_tls));
}

}  // namespace __msan

// Interface.

using namespace __msan;

void __msan_warning() {
  GET_CALLER_PC_BP_SP;
  (void)sp;
  PrintWarning(pc, bp);
  if (__msan::flags()->halt_on_error) {
    Printf("Exiting\n");
    Die();
  }
}

void __msan_warning_noreturn() {
  GET_CALLER_PC_BP_SP;
  (void)sp;
  PrintWarning(pc, bp);
  Printf("Exiting\n");
  Die();
}

void __msan_init() {
  if (msan_inited) return;
  msan_init_is_running = 1;
  SanitizerToolName = "MemorySanitizer";

  InstallAtExitHandler();
  SetDieCallback(MsanDie);
  InitTlsSize();
  InitializeInterceptors();

  if (MSAN_REPLACE_OPERATORS_NEW_AND_DELETE)
    ReplaceOperatorsNewAndDelete();
  const char *msan_options = GetEnv("MSAN_OPTIONS");
  InitializeFlags(&msan_flags, msan_options);
  __sanitizer_set_report_path(common_flags()->log_path);
  if (StackSizeIsUnlimited()) {
    if (flags()->verbosity)
      Printf("Unlimited stack, doing reexec\n");
    // A reasonably large stack size. It is bigger than the usual 8Mb, because,
    // well, the program could have been run with unlimited stack for a reason.
    SetStackSizeLimitInBytes(32 * 1024 * 1024);
    ReExec();
  }

  if (flags()->verbosity)
    Printf("MSAN_OPTIONS: %s\n", msan_options ? msan_options : "<empty>");

  msan_running_under_dr = IsRunningUnderDr();
  __msan_clear_on_return();
  if (__msan_get_track_origins() && flags()->verbosity > 0)
    Printf("msan_track_origins\n");
  if (!InitShadow(/* prot1 */ false, /* prot2 */ true, /* map_shadow */ true,
                  __msan_get_track_origins())) {
    // FIXME: prot1 = false is only required when running under DR.
    Printf("FATAL: MemorySanitizer can not mmap the shadow memory.\n");
    Printf("FATAL: Make sure to compile with -fPIE and to link with -pie.\n");
    Printf("FATAL: Disabling ASLR is known to cause this error.\n");
    Printf("FATAL: If running under GDB, try "
           "'set disable-randomization off'.\n");
    DumpProcessMap();
    Die();
  }

  const char *external_symbolizer = common_flags()->external_symbolizer_path;
  if (external_symbolizer && external_symbolizer[0]) {
    CHECK(InitializeExternalSymbolizer(external_symbolizer));
  }

  GetThreadStackTopAndBottom(/* at_initialization */true,
                             &__msan_stack_bounds.stack_top,
                             &__msan_stack_bounds.stack_bottom);
  if (flags()->verbosity)
    Printf("MemorySanitizer init done\n");
  msan_init_is_running = 0;
  msan_inited = 1;
}

void __msan_set_exit_code(int exit_code) {
  flags()->exit_code = exit_code;
}

void __msan_set_keep_going(int keep_going) {
  flags()->halt_on_error = !keep_going;
}

void __msan_set_expect_umr(int expect_umr) {
  if (expect_umr) {
    msan_expected_umr_found = 0;
  } else if (!msan_expected_umr_found) {
    GET_CALLER_PC_BP_SP;
    (void)sp;
    StackTrace stack;
    GetStackTrace(&stack, kStackTraceMax, pc, bp,
                  common_flags()->fast_unwind_on_fatal);
    ReportExpectedUMRNotFound(&stack);
    Die();
  }
  msan_expect_umr = expect_umr;
}

void __msan_print_shadow(const void *x, uptr size) {
  unsigned char *s = (unsigned char*)MEM_TO_SHADOW(x);
  u32 *o = (u32*)MEM_TO_ORIGIN(x);
  for (uptr i = 0; i < size; i++) {
    Printf("%x%x ", s[i] >> 4, s[i] & 0xf);
  }
  Printf("\n");
  if (__msan_get_track_origins()) {
    for (uptr i = 0; i < size / 4; i++) {
      Printf(" o: %x ", o[i]);
    }
    Printf("\n");
  }
}

void __msan_print_param_shadow() {
  for (int i = 0; i < 16; i++) {
    Printf("#%d:%zx ", i, __msan_param_tls[i]);
  }
  Printf("\n");
}

sptr __msan_test_shadow(const void *x, uptr size) {
  unsigned char *s = (unsigned char*)MEM_TO_SHADOW((uptr)x);
  for (uptr i = 0; i < size; ++i)
    if (s[i])
      return i;
  return -1;
}

int __msan_set_poison_in_malloc(int do_poison) {
  int old = flags()->poison_in_malloc;
  flags()->poison_in_malloc = do_poison;
  return old;
}

int  __msan_has_dynamic_component() {
  return msan_running_under_dr;
}

NOINLINE
void __msan_clear_on_return() {
  __msan_param_tls[0] = 0;
}

static void* get_tls_base() {
  u64 p;
  asm("mov %%fs:0, %0"
      : "=r"(p) ::);
  return (void*)p;
}

int __msan_get_retval_tls_offset() {
  // volatile here is needed to avoid UB, because the compiler thinks that we
  // are doing address arithmetics on unrelated pointers, and takes some
  // shortcuts
  volatile sptr retval_tls_p = (sptr)&__msan_retval_tls;
  volatile sptr tls_base_p = (sptr)get_tls_base();
  return retval_tls_p - tls_base_p;
}

int __msan_get_param_tls_offset() {
  // volatile here is needed to avoid UB, because the compiler thinks that we
  // are doing address arithmetics on unrelated pointers, and takes some
  // shortcuts
  volatile sptr param_tls_p = (sptr)&__msan_param_tls;
  volatile sptr tls_base_p = (sptr)get_tls_base();
  return param_tls_p - tls_base_p;
}

void __msan_partial_poison(const void* data, void* shadow, uptr size) {
  internal_memcpy((void*)MEM_TO_SHADOW((uptr)data), shadow, size);
}

void __msan_load_unpoisoned(void *src, uptr size, void *dst) {
  internal_memcpy(dst, src, size);
  __msan_unpoison(dst, size);
}

void __msan_set_origin(const void *a, uptr size, u32 origin) {
  // Origin mapping is 4 bytes per 4 bytes of application memory.
  // Here we extend the range such that its left and right bounds are both
  // 4 byte aligned.
  if (!__msan_get_track_origins()) return;
  uptr x = MEM_TO_ORIGIN((uptr)a);
  uptr beg = x & ~3UL;  // align down.
  uptr end = (x + size + 3) & ~3UL;  // align up.
  u64 origin64 = ((u64)origin << 32) | origin;
  // This is like memset, but the value is 32-bit. We unroll by 2 two write
  // 64-bits at once. May want to unroll further to get 128-bit stores.
  if (beg & 7ULL) {
    *(u32*)beg = origin;
    beg += 4;
  }
  for (uptr addr = beg; addr < (end & ~7UL); addr += 8)
    *(u64*)addr = origin64;
  if (end & 7ULL)
    *(u32*)(end - 4) = origin;
}

// 'descr' is created at compile time and contains '----' in the beginning.
// When we see descr for the first time we replace '----' with a uniq id
// and set the origin to (id | (31-th bit)).
void __msan_set_alloca_origin(void *a, uptr size, const char *descr) {
  static const u32 dash = '-';
  static const u32 first_timer =
      dash + (dash << 8) + (dash << 16) + (dash << 24);
  u32 *id_ptr = (u32*)descr;
  bool print = false;  // internal_strstr(descr + 4, "AllocaTOTest") != 0;
  u32 id = *id_ptr;
  if (id == first_timer) {
    id = atomic_fetch_add(&NumStackOriginDescrs,
                          1, memory_order_relaxed);
    *id_ptr = id;
    CHECK_LT(id, kNumStackOriginDescrs);
    StackOriginDescr[id] = descr + 4;
    if (print)
      Printf("First time: id=%d %s \n", id, descr + 4);
  }
  id |= 1U << 31;
  if (print)
    Printf("__msan_set_alloca_origin: descr=%s id=%x\n", descr + 4, id);
  __msan_set_origin(a, size, id);
}

const char *__msan_get_origin_descr_if_stack(u32 id) {
  if ((id >> 31) == 0) return 0;
  id &= (1U << 31) - 1;
  CHECK_LT(id, kNumStackOriginDescrs);
  return StackOriginDescr[id];
}


u32 __msan_get_origin(const void *a) {
  if (!__msan_get_track_origins()) return 0;
  uptr x = (uptr)a;
  uptr aligned = x & ~3ULL;
  uptr origin_ptr = MEM_TO_ORIGIN(aligned);
  return *(u32*)origin_ptr;
}

u32 __msan_get_umr_origin() {
  return __msan_origin_tls;
}

u16 __sanitizer_unaligned_load16(const uu16 *p) {
  __msan_retval_tls[0] = *(uu16 *)MEM_TO_SHADOW((uptr)p);
  return *p;
}
u32 __sanitizer_unaligned_load32(const uu32 *p) {
  __msan_retval_tls[0] = *(uu32 *)MEM_TO_SHADOW((uptr)p);
  return *p;
}
u64 __sanitizer_unaligned_load64(const uu64 *p) {
  __msan_retval_tls[0] = *(uu64 *)MEM_TO_SHADOW((uptr)p);
  return *p;
}
void __sanitizer_unaligned_store16(uu16 *p, u16 x) {
  *(uu16 *)MEM_TO_SHADOW((uptr)p) = __msan_param_tls[1];
  *p = x;
}
void __sanitizer_unaligned_store32(uu32 *p, u32 x) {
  *(uu32 *)MEM_TO_SHADOW((uptr)p) = __msan_param_tls[1];
  *p = x;
}
void __sanitizer_unaligned_store64(uu64 *p, u64 x) {
  *(uu64 *)MEM_TO_SHADOW((uptr)p) = __msan_param_tls[1];
  *p = x;
}

#if !SANITIZER_SUPPORTS_WEAK_HOOKS
extern "C" {
SANITIZER_INTERFACE_ATTRIBUTE SANITIZER_WEAK_ATTRIBUTE
const char* __msan_default_options() { return ""; }
}  // extern "C"
#endif

