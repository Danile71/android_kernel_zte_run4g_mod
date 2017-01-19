//===- FileToken.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_FILETOKEN_H
#define MCLD_SCRIPT_FILETOKEN_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <mcld/Script/InputToken.h>
#include <mcld/Support/Allocators.h>
#include <mcld/Config/Config.h>

namespace mcld
{

/** \class FileToken
 *  \brief This class defines the interfaces to a filename in INPUT/GROUP
 *         command.
 */

class FileToken : public InputToken
{
private:
  friend class Chunk<FileToken, MCLD_SYMBOLS_PER_INPUT>;
  FileToken();
  FileToken(const std::string& pName, bool pAsNeeded);

public:
  ~FileToken();

  static bool classof(const InputToken* pToken)
  {
    return pToken->type() == InputToken::File;
  }

  /* factory method */
  static FileToken* create(const std::string& pName, bool pAsNeeded);
  static void destroy(FileToken*& pToken);
  static void clear();
};

} // namepsace of mcld

#endif
