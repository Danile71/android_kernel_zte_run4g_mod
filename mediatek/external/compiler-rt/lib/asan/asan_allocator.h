//===-- asan_allocator.h ----------------------------------------*- C++ -*-===//
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
// ASan-private header for asan_allocator2.cc.
//===----------------------------------------------------------------------===//

#ifndef ASAN_ALLOCATOR_H
#define ASAN_ALLOCATOR_H

#include "asan_internal.h"
#include "asan_interceptors.h"
#include "sanitizer_common/sanitizer_list.h"

namespace __asan {

enum AllocType {
  FROM_MALLOC = 1,  // Memory block came from malloc, calloc, realloc, etc.
  FROM_NEW = 2,     // Memory block came from operator new.
  FROM_NEW_BR = 3   // Memory block came from operator new [ ]
};

static const uptr kNumberOfSizeClasses = 255;
struct AsanChunk;

void InitializeAllocator();

class AsanChunkView {
 public:
  explicit AsanChunkView(AsanChunk *chunk) : chunk_(chunk) {}
  bool IsValid() { return chunk_ != 0; }
  uptr Beg();       // first byte of user memory.
  uptr End();       // last byte of user memory.
  uptr UsedSize();  // size requested by the user.
  uptr AllocTid();
  uptr FreeTid();
  void GetAllocStack(StackTrace *stack);
  void GetFreeStack(StackTrace *stack);
  bool AddrIsInside(uptr addr, uptr access_size, sptr *offset) {
    if (addr >= Beg() && (addr + access_size) <= End()) {
      *offset = addr - Beg();
      return true;
    }
    return false;
  }
  bool AddrIsAtLeft(uptr addr, uptr access_size, sptr *offset) {
    (void)access_size;
    if (addr < Beg()) {
      *offset = Beg() - addr;
      return true;
    }
    return false;
  }
  bool AddrIsAtRight(uptr addr, uptr access_size, sptr *offset) {
    if (addr + access_size > End()) {
      *offset = addr - End();
      return true;
    }
    return false;
  }

 private:
  AsanChunk *const chunk_;
};

AsanChunkView FindHeapChunkByAddress(uptr address);

// List of AsanChunks with total size.
class AsanChunkFifoList: public IntrusiveList<AsanChunk> {
 public:
  explicit AsanChunkFifoList(LinkerInitialized) { }
  AsanChunkFifoList() { clear(); }
  void Push(AsanChunk *n);
  void PushList(AsanChunkFifoList *q);
  AsanChunk *Pop();
  uptr size() { return size_; }
  void clear() {
    IntrusiveList<AsanChunk>::clear();
    size_ = 0;
  }
 private:
  uptr size_;
};

struct AsanThreadLocalMallocStorage {
  explicit AsanThreadLocalMallocStorage(LinkerInitialized x)
      { }
  AsanThreadLocalMallocStorage() {
    CHECK(REAL(memset));
    REAL(memset)(this, 0, sizeof(AsanThreadLocalMallocStorage));
  }

  uptr quarantine_cache[16];
  uptr allocator2_cache[96 * (512 * 8 + 16)];  // Opaque.
  void CommitBack();
};

SANITIZER_INTERFACE_ATTRIBUTE
void *asan_memalign(uptr alignment, uptr size, StackTrace *stack,
                    AllocType alloc_type);
SANITIZER_INTERFACE_ATTRIBUTE
void asan_free(void *ptr, StackTrace *stack, AllocType alloc_type);

SANITIZER_INTERFACE_ATTRIBUTE
void *asan_malloc(uptr size, StackTrace *stack);
void *asan_calloc(uptr nmemb, uptr size, StackTrace *stack);
void *asan_realloc(void *p, uptr size, StackTrace *stack);
void *asan_valloc(uptr size, StackTrace *stack);
void *asan_pvalloc(uptr size, StackTrace *stack);

int asan_posix_memalign(void **memptr, uptr alignment, uptr size,
                          StackTrace *stack);
SANITIZER_INTERFACE_ATTRIBUTE
uptr asan_malloc_usable_size(void *ptr, StackTrace *stack);

uptr asan_mz_size(const void *ptr);
void asan_mz_force_lock();
void asan_mz_force_unlock();

void PrintInternalAllocatorStats();

}  // namespace __asan
#endif  // ASAN_ALLOCATOR_H
