//===- SearchDirCmd.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_SEARCHDIRCMD_H
#define MCLD_SCRIPT_SEARCHDIRCMD_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <mcld/Script/ScriptCommand.h>
#include <string>

namespace mcld
{

class Module;

/** \class SearchDirCmd
 *  \brief This class defines the interfaces to SEARCH_DIR command.
 */

class SearchDirCmd : public ScriptCommand
{
public:
  SearchDirCmd(const std::string& pPath);
  ~SearchDirCmd();

  void dump() const;

  void activate(Module& pModule);

  static bool classof(const ScriptCommand* pCmd)
  {
    return pCmd->getKind() == ScriptCommand::SEARCH_DIR;
  }

private:
  std::string m_Path;
};

} // namespace of mcld

#endif

