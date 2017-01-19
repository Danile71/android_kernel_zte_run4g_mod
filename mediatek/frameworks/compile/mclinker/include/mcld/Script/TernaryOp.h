//===- TernaryOp.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_TERNARYOP_H
#define MCLD_SCRIPT_TERNARYOP_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <mcld/Script/Operator.h>
#include <cstddef>

namespace mcld
{

class Operand;
class IntOperand;
class Module;
class TargetLDBackend;

/** \class TernaryOP
 *  \brief This class defines the interfaces to an binary operator token.
 */

template<Operator::Type TYPE>
class TernaryOp : public Operator
{
private:
  friend class Operator;

  TernaryOp()
    : Operator(Operator::TERNARY, TYPE)
  {
    m_pOperand[0] = m_pOperand[1] = m_pOperand[2] = NULL;
  }

public:
  ~TernaryOp()
  {}

  IntOperand* eval(const Module& pModule, const TargetLDBackend& pBackend);

  void appendOperand(Operand* pOperand)
  {
    m_pOperand[m_Size++] = pOperand;
    if (m_Size == 3)
      m_Size = 0;
  }

private:
  size_t m_Size;
  Operand* m_pOperand[3];
};

template<>
IntOperand* TernaryOp<Operator::TERNARY_IF>::eval(const Module&,
                                                  const TargetLDBackend&);

template<>
IntOperand*
TernaryOp<Operator::DATA_SEGMENT_ALIGN>::eval(const Module&,
                                              const TargetLDBackend&);

} // namespace of mcld

#endif
