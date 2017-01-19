//===- THMToTHMStub.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef MCLD_ARM_THMToTHMStub_H
#define MCLD_ARM_THMToTHMStub_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <llvm/Support/DataTypes.h>
#include <mcld/Fragment/Stub.h>
#include <string>
#include <vector>

namespace mcld
{

class Relocation;
class ResolveInfo;

/** \class THMToTHMStub
 *  \brief ARM stub for long call from ARM source to ARM target
 *
 */
class THMToTHMStub : public Stub
{
public:
  THMToTHMStub(bool pIsOutputPIC);

  ~THMToTHMStub();

  // isMyDuty
  bool isMyDuty(const class Relocation& pReloc,
                uint64_t pSource,
                uint64_t pTargetSymValue) const;

  // observers
  const std::string& name() const;

  const uint8_t* getContent() const;

  size_t size() const;

  size_t alignment() const;

  // for T bit of this stub
  uint64_t initSymValue() const;

private:
  THMToTHMStub(const THMToTHMStub&);

  THMToTHMStub& operator=(const THMToTHMStub&);

  /// for doClone
  THMToTHMStub(const uint32_t* pData,
               size_t pSize,
               const_fixup_iterator pBegin,
               const_fixup_iterator pEnd);

  /// doClone
  Stub* doClone();

private:
  std::string m_Name;
  static const uint32_t PIC_TEMPLATE[];
  static const uint32_t TEMPLATE[];
  const uint32_t* m_pData;
  size_t m_Size;
};

} // namespace of mcld

#endif
