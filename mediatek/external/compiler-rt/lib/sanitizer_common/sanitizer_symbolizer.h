//===-- sanitizer_symbolizer.h ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Symbolizer is intended to be used by both
// AddressSanitizer and ThreadSanitizer to symbolize a given
// address. It is an analogue of addr2line utility and allows to map
// instruction address to a location in source code at run-time.
//
// Symbolizer is planned to use debug information (in DWARF format)
// in a binary via interface defined in "llvm/DebugInfo/DIContext.h"
//
// Symbolizer code should be called from the run-time library of
// dynamic tools, and generally should not call memory allocation
// routines or other system library functions intercepted by those tools.
// Instead, Symbolizer code should use their replacements, defined in
// "compiler-rt/lib/sanitizer_common/sanitizer_libc.h".
//===----------------------------------------------------------------------===//
#ifndef SANITIZER_SYMBOLIZER_H
#define SANITIZER_SYMBOLIZER_H

#include "sanitizer_internal_defs.h"
#include "sanitizer_libc.h"
// WARNING: Do not include system headers here. See details above.

namespace __sanitizer {

struct AddressInfo {
  uptr address;
  char *module;
  uptr module_offset;
  char *function;
  char *file;
  int line;
  int column;

  AddressInfo() {
    internal_memset(this, 0, sizeof(AddressInfo));
  }
  // Deletes all strings and sets all fields to zero.
  SANITIZER_WEAK_ATTRIBUTE void Clear();

  void FillAddressAndModuleInfo(uptr addr, const char *mod_name,
                                uptr mod_offset) {
    address = addr;
    module = internal_strdup(mod_name);
    module_offset = mod_offset;
  }
};

struct DataInfo {
  uptr address;
  char *module;
  uptr module_offset;
  char *name;
  uptr start;
  uptr size;
};

// Fills at most "max_frames" elements of "frames" with descriptions
// for a given address (in all inlined functions). Returns the number
// of descriptions actually filled.
// This function should NOT be called from two threads simultaneously.
SANITIZER_WEAK_ATTRIBUTE
uptr SymbolizeCode(uptr address, AddressInfo *frames, uptr max_frames);
bool SymbolizeData(uptr address, DataInfo *info);

bool IsSymbolizerAvailable();
void FlushSymbolizer();  // releases internal caches (if any)

// Attempts to demangle the provided C++ mangled name.
const char *Demangle(const char *name);
// Attempts to demangle the name via __cxa_demangle from __cxxabiv1.
const char *DemangleCXXABI(const char *name);

// Starts external symbolizer program in a subprocess. Sanitizer communicates
// with external symbolizer via pipes.
bool InitializeExternalSymbolizer(const char *path_to_symbolizer);

const int kSymbolizerStartupTimeMillis = 10;

class LoadedModule {
 public:
  LoadedModule(const char *module_name, uptr base_address);
  void addAddressRange(uptr beg, uptr end);
  bool containsAddress(uptr address) const;

  const char *full_name() const { return full_name_; }
  uptr base_address() const { return base_address_; }

 private:
  struct AddressRange {
    uptr beg;
    uptr end;
  };
  char *full_name_;
  uptr base_address_;
  static const uptr kMaxNumberOfAddressRanges = 6;
  AddressRange ranges_[kMaxNumberOfAddressRanges];
  uptr n_ranges_;
};

// Creates external symbolizer connected via pipe, user should write
// to output_fd and read from input_fd.
bool StartSymbolizerSubprocess(const char *path_to_symbolizer,
                               int *input_fd, int *output_fd);

// OS-dependent function that fills array with descriptions of at most
// "max_modules" currently loaded modules. Returns the number of
// initialized modules. If filter is nonzero, ignores modules for which
// filter(full_name) is false.
typedef bool (*string_predicate_t)(const char *);
uptr GetListOfModules(LoadedModule *modules, uptr max_modules,
                      string_predicate_t filter);

void SymbolizerPrepareForSandboxing();

}  // namespace __sanitizer

#endif  // SANITIZER_SYMBOLIZER_H
