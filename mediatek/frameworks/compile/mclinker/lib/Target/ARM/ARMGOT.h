//===- ARMGOT.h -----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ARM_GOT_H
#define MCLD_ARM_GOT_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <mcld/Target/GOT.h>
#include <mcld/Support/MemoryRegion.h>
#include <llvm/ADT/DenseMap.h>
#include <vector>

namespace mcld {

class LDSection;

/** \class ARMGOTEntry
 *  \brief GOT Entry with size of 4 bytes
 */
class ARMGOTEntry : public GOT::Entry<4>
{
public:
  ARMGOTEntry(uint64_t pContent, SectionData* pParent)
   : GOT::Entry<4>(pContent, pParent)
  {}
};

/** \class ARMGOT
 *  \brief ARM Global Offset Table.
 *
 *  ARM GOT integrates traditional .got.plt and .got sections into one.
 *  Traditional .got.plt is placed in the front part of GOT (PLTGOT), and
 *  traditional .got is placed in the rear part of GOT (GOT).
 *
 *  ARM .got
 *            +--------------+
 *            |    GOT0      |
 *            +--------------+
 *            |    GOTPLT    |
 *            +--------------+
 *            |    GOT       |
 *            +--------------+
 *
 */
class ARMGOT : public GOT
{
public:
  ARMGOT(LDSection &pSection);

  ~ARMGOT();

  ARMGOTEntry* createGOT();
  ARMGOTEntry* createGOTPLT();

  void finalizeSectionSize();

  uint64_t emit(MemoryRegion& pRegion);

  void applyGOT0(uint64_t pAddress);

  void applyGOTPLT(uint64_t pPLTBase);

  bool hasGOT1() const;

private:
  typedef std::vector<ARMGOTEntry*> EntryListType;
  typedef EntryListType::iterator entry_iterator;
  typedef EntryListType::const_iterator const_entry_iterator;

private:
  ARMGOTEntry* m_pGOTPLTFront;
  ARMGOTEntry* m_pGOTFront;

  /// m_GOTPLTEntries - a list of gotplt entries
  EntryListType m_GOTPLT;

  /// m_GOTEntris - a list of got entries
  EntryListType m_GOT;
};

} // namespace of mcld

#endif

