//===-- sanitizer_symbolizer_win.cc ---------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is shared between AddressSanitizer and ThreadSanitizer
// run-time libraries.
// Windows-specific implementation of symbolizer parts.
//===----------------------------------------------------------------------===//

#include "sanitizer_platform.h"
#if SANITIZER_WINDOWS
#include <windows.h>

#include "sanitizer_internal_defs.h"
#include "sanitizer_symbolizer.h"

namespace __sanitizer {

bool StartSymbolizerSubprocess(const char *path_to_symbolizer,
                               int *input_fd, int *output_fd) {
  UNIMPLEMENTED();
}

uptr GetListOfModules(LoadedModule *modules, uptr max_modules,
                      string_predicate_t filter) {
  UNIMPLEMENTED();
};

void SymbolizerPrepareForSandboxing() {
  // Do nothing on Windows.
}

const char *DemangleCXXABI(const char *name) {
  return name;
}

}  // namespace __sanitizer

#endif  // _WIN32
