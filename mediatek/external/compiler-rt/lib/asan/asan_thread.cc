//===-- asan_thread.cc ----------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of AddressSanitizer, an address sanity checker.
//
// Thread-related code.
//===----------------------------------------------------------------------===//
#include "asan_allocator.h"
#include "asan_interceptors.h"
#include "asan_poisoning.h"
#include "asan_stack.h"
#include "asan_thread.h"
#include "asan_mapping.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_placement_new.h"
#include "lsan/lsan_common.h"

namespace __asan {

// AsanThreadContext implementation.

void AsanThreadContext::OnCreated(void *arg) {
  CreateThreadContextArgs *args = static_cast<CreateThreadContextArgs*>(arg);
  if (args->stack) {
    internal_memcpy(&stack, args->stack, sizeof(stack));
  }
  thread = args->thread;
  thread->set_context(this);
}

void AsanThreadContext::OnFinished() {
  // Drop the link to the AsanThread object.
  thread = 0;
}

// MIPS requires aligned address
static ALIGNED(16) char thread_registry_placeholder[sizeof(ThreadRegistry)];
static ThreadRegistry *asan_thread_registry;

static ThreadContextBase *GetAsanThreadContext(u32 tid) {
  void *mem = MmapOrDie(sizeof(AsanThreadContext), "AsanThreadContext");
  return new(mem) AsanThreadContext(tid);
}

ThreadRegistry &asanThreadRegistry() {
  static bool initialized;
  // Don't worry about thread_safety - this should be called when there is
  // a single thread.
  if (!initialized) {
    // Never reuse ASan threads: we store pointer to AsanThreadContext
    // in TSD and can't reliably tell when no more TSD destructors will
    // be called. It would be wrong to reuse AsanThreadContext for another
    // thread before all TSD destructors will be called for it.
    asan_thread_registry = new(thread_registry_placeholder) ThreadRegistry(
        GetAsanThreadContext, kMaxNumberOfThreads, kMaxNumberOfThreads);
    initialized = true;
  }
  return *asan_thread_registry;
}

AsanThreadContext *GetThreadContextByTidLocked(u32 tid) {
  return static_cast<AsanThreadContext *>(
      asanThreadRegistry().GetThreadLocked(tid));
}

// AsanThread implementation.

AsanThread *AsanThread::Create(thread_callback_t start_routine,
                               void *arg) {
  uptr PageSize = GetPageSizeCached();
  uptr size = RoundUpTo(sizeof(AsanThread), PageSize);
  AsanThread *thread = (AsanThread*)MmapOrDie(size, __FUNCTION__);
  thread->start_routine_ = start_routine;
  thread->arg_ = arg;
  thread->context_ = 0;

  return thread;
}

void AsanThread::TSDDtor(void *tsd) {
  AsanThreadContext *context = (AsanThreadContext*)tsd;
  if (flags()->verbosity >= 1)
    Report("T%d TSDDtor\n", context->tid);
  if (context->thread)
    context->thread->Destroy();
}

void AsanThread::Destroy() {
  if (flags()->verbosity >= 1) {
    Report("T%d exited\n", tid());
  }

  asanThreadRegistry().FinishThread(tid());
  FlushToAccumulatedStats(&stats_);
  // We also clear the shadow on thread destruction because
  // some code may still be executing in later TSD destructors
  // and we don't want it to have any poisoned stack.
  ClearShadowForThreadStackAndTLS();
  DeleteFakeStack();
  uptr size = RoundUpTo(sizeof(AsanThread), GetPageSizeCached());
  UnmapOrDie(this, size);
}

void AsanThread::Init() {
  SetThreadStackAndTls();
  CHECK(AddrIsInMem(stack_bottom_));
  CHECK(AddrIsInMem(stack_top_ - 1));
  ClearShadowForThreadStackAndTLS();
  if (flags()->verbosity >= 1) {
    int local = 0;
    Report("T%d: stack [%p,%p) size 0x%zx; local=%p\n",
           tid(), (void*)stack_bottom_, (void*)stack_top_,
           stack_top_ - stack_bottom_, &local);
  }
  fake_stack_ = 0;  // Will be initialized lazily if needed.
  AsanPlatformThreadInit();
}

thread_return_t AsanThread::ThreadStart(uptr os_id) {
  Init();
  asanThreadRegistry().StartThread(tid(), os_id, 0);
  if (flags()->use_sigaltstack) SetAlternateSignalStack();

  if (!start_routine_) {
    // start_routine_ == 0 if we're on the main thread or on one of the
    // OS X libdispatch worker threads. But nobody is supposed to call
    // ThreadStart() for the worker threads.
    CHECK_EQ(tid(), 0);
    return 0;
  }

  thread_return_t res = start_routine_(arg_);
  malloc_storage().CommitBack();
  if (flags()->use_sigaltstack) UnsetAlternateSignalStack();

  this->Destroy();

  return res;
}

void AsanThread::SetThreadStackAndTls() {
  uptr stack_size = 0, tls_size = 0;
  GetThreadStackAndTls(tid() == 0, &stack_bottom_, &stack_size, &tls_begin_,
                       &tls_size);
  stack_top_ = stack_bottom_ + stack_size;
  tls_end_ = tls_begin_ + tls_size;

  int local;
  CHECK(AddrIsInStack((uptr)&local));
}

void AsanThread::ClearShadowForThreadStackAndTLS() {
  PoisonShadow(stack_bottom_, stack_top_ - stack_bottom_, 0);
  if (tls_begin_ != tls_end_)
    PoisonShadow(tls_begin_, tls_end_ - tls_begin_, 0);
}

const char *AsanThread::GetFrameNameByAddr(uptr addr, uptr *offset,
                                           uptr *frame_pc) {
  uptr bottom = 0;
  if (AddrIsInStack(addr)) {
    bottom = stack_bottom();
  } else if (fake_stack()) {
    bottom = fake_stack()->AddrIsInFakeStack(addr);
    CHECK(bottom);
    *offset = addr - bottom;
    *frame_pc = ((uptr*)bottom)[2];
    return  (const char *)((uptr*)bottom)[1];
  }
  uptr aligned_addr = addr & ~(SANITIZER_WORDSIZE/8 - 1);  // align addr.
  u8 *shadow_ptr = (u8*)MemToShadow(aligned_addr);
  u8 *shadow_bottom = (u8*)MemToShadow(bottom);

  while (shadow_ptr >= shadow_bottom &&
         *shadow_ptr != kAsanStackLeftRedzoneMagic) {
    shadow_ptr--;
  }

  while (shadow_ptr >= shadow_bottom &&
         *shadow_ptr == kAsanStackLeftRedzoneMagic) {
    shadow_ptr--;
  }

  if (shadow_ptr < shadow_bottom) {
    *offset = 0;
    return "UNKNOWN";
  }

  uptr* ptr = (uptr*)SHADOW_TO_MEM((uptr)(shadow_ptr + 1));
  CHECK(ptr[0] == kCurrentStackFrameMagic);
  *offset = addr - (uptr)ptr;
  *frame_pc = ptr[2];
  return (const char*)ptr[1];
}

static bool ThreadStackContainsAddress(ThreadContextBase *tctx_base,
                                       void *addr) {
  AsanThreadContext *tctx = static_cast<AsanThreadContext*>(tctx_base);
  AsanThread *t = tctx->thread;
  if (!t) return false;
  if (t->AddrIsInStack((uptr)addr)) return true;
  if (t->fake_stack() && t->fake_stack()->AddrIsInFakeStack((uptr)addr))
    return true;
  return false;
}

AsanThread *GetCurrentThread() {
  AsanThreadContext *context =
      reinterpret_cast<AsanThreadContext *>(AsanTSDGet());
  if (!context) {
    if (SANITIZER_ANDROID) {
      // On Android, libc constructor is called _after_ asan_init, and cleans up
      // TSD. Try to figure out if this is still the main thread by the stack
      // address. We are not entirely sure that we have correct main thread
      // limits, so only do this magic on Android, and only if the found thread
      // is the main thread.
      AsanThreadContext *tctx = GetThreadContextByTidLocked(0);
      if (ThreadStackContainsAddress(tctx, &context)) {
        SetCurrentThread(tctx->thread);
        return tctx->thread;
      }
    }
    return 0;
  }
  return context->thread;
}

void SetCurrentThread(AsanThread *t) {
  CHECK(t->context());
  if (flags()->verbosity >= 2) {
    Report("SetCurrentThread: %p for thread %p\n",
           t->context(), (void*)GetThreadSelf());
  }
  // Make sure we do not reset the current AsanThread.
  CHECK_EQ(0, AsanTSDGet());
  AsanTSDSet(t->context());
  CHECK_EQ(t->context(), AsanTSDGet());
}

u32 GetCurrentTidOrInvalid() {
  AsanThread *t = GetCurrentThread();
  return t ? t->tid() : kInvalidTid;
}

AsanThread *FindThreadByStackAddress(uptr addr) {
  asanThreadRegistry().CheckLocked();
  AsanThreadContext *tctx = static_cast<AsanThreadContext *>(
      asanThreadRegistry().FindThreadContextLocked(ThreadStackContainsAddress,
                                                   (void *)addr));
  return tctx ? tctx->thread : 0;
}

void EnsureMainThreadIDIsCorrect() {
  AsanThreadContext *context =
      reinterpret_cast<AsanThreadContext *>(AsanTSDGet());
  if (context && (context->tid == 0))
    context->os_id = GetTid();
}
}  // namespace __asan

// --- Implementation of LSan-specific functions --- {{{1
namespace __lsan {
bool GetThreadRangesLocked(uptr os_id, uptr *stack_begin, uptr *stack_end,
                           uptr *tls_begin, uptr *tls_end,
                           uptr *cache_begin, uptr *cache_end) {
  __asan::AsanThreadContext *context = static_cast<__asan::AsanThreadContext *>(
      __asan::asanThreadRegistry().FindThreadContextByOsIDLocked(os_id));
  if (!context) return false;
  __asan::AsanThread *t = context->thread;
  if (!t) return false;
  *stack_begin = t->stack_bottom();
  *stack_end = t->stack_top();
  *tls_begin = t->tls_begin();
  *tls_end = t->tls_end();
  // ASan doesn't keep allocator caches in TLS, so these are unused.
  *cache_begin = 0;
  *cache_end = 0;
  return true;
}

void LockThreadRegistry() {
  __asan::asanThreadRegistry().Lock();
}

void UnlockThreadRegistry() {
  __asan::asanThreadRegistry().Unlock();
}

void EnsureMainThreadIDIsCorrect() {
  __asan::EnsureMainThreadIDIsCorrect();
}
}  // namespace __lsan
