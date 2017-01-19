//===--- Format.cpp - Format C++ code -------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements functions declared in Format.h. This will be
/// split into separate files as we go.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "format-formatter"

#include "ContinuationIndenter.h"
#include "TokenAnnotator.h"
#include "UnwrappedLineParser.h"
#include "WhitespaceManager.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Format/Format.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/YAMLTraits.h"
#include <queue>
#include <string>

namespace llvm {
namespace yaml {
template <>
struct ScalarEnumerationTraits<clang::format::FormatStyle::LanguageStandard> {
  static void enumeration(IO &IO,
                          clang::format::FormatStyle::LanguageStandard &Value) {
    IO.enumCase(Value, "C++03", clang::format::FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "C++11", clang::format::FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "Auto", clang::format::FormatStyle::LS_Auto);
  }
};

template <>
struct ScalarEnumerationTraits<clang::format::FormatStyle::BraceBreakingStyle> {
  static void
  enumeration(IO &IO, clang::format::FormatStyle::BraceBreakingStyle &Value) {
    IO.enumCase(Value, "Attach", clang::format::FormatStyle::BS_Attach);
    IO.enumCase(Value, "Linux", clang::format::FormatStyle::BS_Linux);
    IO.enumCase(Value, "Stroustrup", clang::format::FormatStyle::BS_Stroustrup);
    IO.enumCase(Value, "Allman", clang::format::FormatStyle::BS_Allman);
  }
};

template <>
struct ScalarEnumerationTraits<
    clang::format::FormatStyle::NamespaceIndentationKind> {
  static void
  enumeration(IO &IO,
              clang::format::FormatStyle::NamespaceIndentationKind &Value) {
    IO.enumCase(Value, "None", clang::format::FormatStyle::NI_None);
    IO.enumCase(Value, "Inner", clang::format::FormatStyle::NI_Inner);
    IO.enumCase(Value, "All", clang::format::FormatStyle::NI_All);
  }
};

template <> struct MappingTraits<clang::format::FormatStyle> {
  static void mapping(llvm::yaml::IO &IO, clang::format::FormatStyle &Style) {
    if (IO.outputting()) {
      StringRef StylesArray[] = { "LLVM", "Google", "Chromium", "Mozilla" };
      ArrayRef<StringRef> Styles(StylesArray);
      for (size_t i = 0, e = Styles.size(); i < e; ++i) {
        StringRef StyleName(Styles[i]);
        clang::format::FormatStyle PredefinedStyle;
        if (clang::format::getPredefinedStyle(StyleName, &PredefinedStyle) &&
            Style == PredefinedStyle) {
          IO.mapOptional("# BasedOnStyle", StyleName);
          break;
        }
      }
    } else {
      StringRef BasedOnStyle;
      IO.mapOptional("BasedOnStyle", BasedOnStyle);
      if (!BasedOnStyle.empty())
        if (!clang::format::getPredefinedStyle(BasedOnStyle, &Style)) {
          IO.setError(Twine("Unknown value for BasedOnStyle: ", BasedOnStyle));
          return;
        }
    }

    IO.mapOptional("AccessModifierOffset", Style.AccessModifierOffset);
    IO.mapOptional("ConstructorInitializerIndentWidth",
                   Style.ConstructorInitializerIndentWidth);
    IO.mapOptional("AlignEscapedNewlinesLeft", Style.AlignEscapedNewlinesLeft);
    IO.mapOptional("AlignTrailingComments", Style.AlignTrailingComments);
    IO.mapOptional("AllowAllParametersOfDeclarationOnNextLine",
                   Style.AllowAllParametersOfDeclarationOnNextLine);
    IO.mapOptional("AllowShortIfStatementsOnASingleLine",
                   Style.AllowShortIfStatementsOnASingleLine);
    IO.mapOptional("AllowShortLoopsOnASingleLine",
                   Style.AllowShortLoopsOnASingleLine);
    IO.mapOptional("AlwaysBreakTemplateDeclarations",
                   Style.AlwaysBreakTemplateDeclarations);
    IO.mapOptional("AlwaysBreakBeforeMultilineStrings",
                   Style.AlwaysBreakBeforeMultilineStrings);
    IO.mapOptional("BreakBeforeBinaryOperators",
                   Style.BreakBeforeBinaryOperators);
    IO.mapOptional("BreakConstructorInitializersBeforeComma",
                   Style.BreakConstructorInitializersBeforeComma);
    IO.mapOptional("BinPackParameters", Style.BinPackParameters);
    IO.mapOptional("ColumnLimit", Style.ColumnLimit);
    IO.mapOptional("ConstructorInitializerAllOnOneLineOrOnePerLine",
                   Style.ConstructorInitializerAllOnOneLineOrOnePerLine);
    IO.mapOptional("DerivePointerBinding", Style.DerivePointerBinding);
    IO.mapOptional("ExperimentalAutoDetectBinPacking",
                   Style.ExperimentalAutoDetectBinPacking);
    IO.mapOptional("IndentCaseLabels", Style.IndentCaseLabels);
    IO.mapOptional("MaxEmptyLinesToKeep", Style.MaxEmptyLinesToKeep);
    IO.mapOptional("NamespaceIndentation", Style.NamespaceIndentation);
    IO.mapOptional("ObjCSpaceBeforeProtocolList",
                   Style.ObjCSpaceBeforeProtocolList);
    IO.mapOptional("PenaltyBreakComment", Style.PenaltyBreakComment);
    IO.mapOptional("PenaltyBreakString", Style.PenaltyBreakString);
    IO.mapOptional("PenaltyBreakFirstLessLess",
                   Style.PenaltyBreakFirstLessLess);
    IO.mapOptional("PenaltyExcessCharacter", Style.PenaltyExcessCharacter);
    IO.mapOptional("PenaltyReturnTypeOnItsOwnLine",
                   Style.PenaltyReturnTypeOnItsOwnLine);
    IO.mapOptional("PointerBindsToType", Style.PointerBindsToType);
    IO.mapOptional("SpacesBeforeTrailingComments",
                   Style.SpacesBeforeTrailingComments);
    IO.mapOptional("Cpp11BracedListStyle", Style.Cpp11BracedListStyle);
    IO.mapOptional("Standard", Style.Standard);
    IO.mapOptional("IndentWidth", Style.IndentWidth);
    IO.mapOptional("UseTab", Style.UseTab);
    IO.mapOptional("BreakBeforeBraces", Style.BreakBeforeBraces);
    IO.mapOptional("IndentFunctionDeclarationAfterType",
                   Style.IndentFunctionDeclarationAfterType);
  }
};
}
}

namespace clang {
namespace format {

void setDefaultPenalties(FormatStyle &Style) {
  Style.PenaltyBreakComment = 45;
  Style.PenaltyBreakFirstLessLess = 120;
  Style.PenaltyBreakString = 1000;
  Style.PenaltyExcessCharacter = 1000000;
}

FormatStyle getLLVMStyle() {
  FormatStyle LLVMStyle;
  LLVMStyle.AccessModifierOffset = -2;
  LLVMStyle.AlignEscapedNewlinesLeft = false;
  LLVMStyle.AlignTrailingComments = true;
  LLVMStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  LLVMStyle.AllowShortIfStatementsOnASingleLine = false;
  LLVMStyle.AllowShortLoopsOnASingleLine = false;
  LLVMStyle.AlwaysBreakBeforeMultilineStrings = false;
  LLVMStyle.AlwaysBreakTemplateDeclarations = false;
  LLVMStyle.BinPackParameters = true;
  LLVMStyle.BreakBeforeBinaryOperators = false;
  LLVMStyle.BreakBeforeBraces = FormatStyle::BS_Attach;
  LLVMStyle.BreakConstructorInitializersBeforeComma = false;
  LLVMStyle.ColumnLimit = 80;
  LLVMStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
  LLVMStyle.ConstructorInitializerIndentWidth = 4;
  LLVMStyle.Cpp11BracedListStyle = false;
  LLVMStyle.DerivePointerBinding = false;
  LLVMStyle.ExperimentalAutoDetectBinPacking = false;
  LLVMStyle.IndentCaseLabels = false;
  LLVMStyle.IndentFunctionDeclarationAfterType = false;
  LLVMStyle.IndentWidth = 2;
  LLVMStyle.MaxEmptyLinesToKeep = 1;
  LLVMStyle.NamespaceIndentation = FormatStyle::NI_None;
  LLVMStyle.ObjCSpaceBeforeProtocolList = true;
  LLVMStyle.PointerBindsToType = false;
  LLVMStyle.SpacesBeforeTrailingComments = 1;
  LLVMStyle.Standard = FormatStyle::LS_Cpp03;
  LLVMStyle.UseTab = false;

  setDefaultPenalties(LLVMStyle);
  LLVMStyle.PenaltyReturnTypeOnItsOwnLine = 60;

  return LLVMStyle;
}

FormatStyle getGoogleStyle() {
  FormatStyle GoogleStyle;
  GoogleStyle.AccessModifierOffset = -1;
  GoogleStyle.AlignEscapedNewlinesLeft = true;
  GoogleStyle.AlignTrailingComments = true;
  GoogleStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  GoogleStyle.AllowShortIfStatementsOnASingleLine = true;
  GoogleStyle.AllowShortLoopsOnASingleLine = true;
  GoogleStyle.AlwaysBreakBeforeMultilineStrings = true;
  GoogleStyle.AlwaysBreakTemplateDeclarations = true;
  GoogleStyle.BinPackParameters = true;
  GoogleStyle.BreakBeforeBinaryOperators = false;
  GoogleStyle.BreakBeforeBraces = FormatStyle::BS_Attach;
  GoogleStyle.BreakConstructorInitializersBeforeComma = false;
  GoogleStyle.ColumnLimit = 80;
  GoogleStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
  GoogleStyle.ConstructorInitializerIndentWidth = 4;
  GoogleStyle.Cpp11BracedListStyle = true;
  GoogleStyle.DerivePointerBinding = true;
  GoogleStyle.ExperimentalAutoDetectBinPacking = false;
  GoogleStyle.IndentCaseLabels = true;
  GoogleStyle.IndentFunctionDeclarationAfterType = true;
  GoogleStyle.IndentWidth = 2;
  GoogleStyle.MaxEmptyLinesToKeep = 1;
  GoogleStyle.NamespaceIndentation = FormatStyle::NI_None;
  GoogleStyle.ObjCSpaceBeforeProtocolList = false;
  GoogleStyle.PointerBindsToType = true;
  GoogleStyle.SpacesBeforeTrailingComments = 2;
  GoogleStyle.Standard = FormatStyle::LS_Auto;
  GoogleStyle.UseTab = false;

  setDefaultPenalties(GoogleStyle);
  GoogleStyle.PenaltyReturnTypeOnItsOwnLine = 200;

  return GoogleStyle;
}

FormatStyle getChromiumStyle() {
  FormatStyle ChromiumStyle = getGoogleStyle();
  ChromiumStyle.AllowAllParametersOfDeclarationOnNextLine = false;
  ChromiumStyle.AllowShortIfStatementsOnASingleLine = false;
  ChromiumStyle.AllowShortLoopsOnASingleLine = false;
  ChromiumStyle.BinPackParameters = false;
  ChromiumStyle.DerivePointerBinding = false;
  ChromiumStyle.Standard = FormatStyle::LS_Cpp03;
  return ChromiumStyle;
}

FormatStyle getMozillaStyle() {
  FormatStyle MozillaStyle = getLLVMStyle();
  MozillaStyle.AllowAllParametersOfDeclarationOnNextLine = false;
  MozillaStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
  MozillaStyle.DerivePointerBinding = true;
  MozillaStyle.IndentCaseLabels = true;
  MozillaStyle.ObjCSpaceBeforeProtocolList = false;
  MozillaStyle.PenaltyReturnTypeOnItsOwnLine = 200;
  MozillaStyle.PointerBindsToType = true;
  return MozillaStyle;
}

FormatStyle getWebKitStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.AccessModifierOffset = -4;
  Style.AlignTrailingComments = false;
  Style.BreakBeforeBinaryOperators = true;
  Style.BreakBeforeBraces = FormatStyle::BS_Stroustrup;
  Style.BreakConstructorInitializersBeforeComma = true;
  Style.ColumnLimit = 0;
  Style.IndentWidth = 4;
  Style.NamespaceIndentation = FormatStyle::NI_Inner;
  Style.PointerBindsToType = true;
  return Style;
}

bool getPredefinedStyle(StringRef Name, FormatStyle *Style) {
  if (Name.equals_lower("llvm"))
    *Style = getLLVMStyle();
  else if (Name.equals_lower("chromium"))
    *Style = getChromiumStyle();
  else if (Name.equals_lower("mozilla"))
    *Style = getMozillaStyle();
  else if (Name.equals_lower("google"))
    *Style = getGoogleStyle();
  else if (Name.equals_lower("webkit"))
    *Style = getWebKitStyle();
  else
    return false;

  return true;
}

llvm::error_code parseConfiguration(StringRef Text, FormatStyle *Style) {
  if (Text.trim().empty())
    return llvm::make_error_code(llvm::errc::invalid_argument);
  llvm::yaml::Input Input(Text);
  Input >> *Style;
  return Input.error();
}

std::string configurationAsText(const FormatStyle &Style) {
  std::string Text;
  llvm::raw_string_ostream Stream(Text);
  llvm::yaml::Output Output(Stream);
  // We use the same mapping method for input and output, so we need a non-const
  // reference here.
  FormatStyle NonConstStyle = Style;
  Output << NonConstStyle;
  return Stream.str();
}

namespace {

class NoColumnLimitFormatter {
public:
  NoColumnLimitFormatter(ContinuationIndenter *Indenter)
      : Indenter(Indenter) {}

  /// \brief Formats the line starting at \p State, simply keeping all of the
  /// input's line breaking decisions.
  void format() {
    LineState State = Indenter->getInitialState();
    while (State.NextToken != NULL) {
      bool Newline =
          Indenter->mustBreak(State) ||
          (Indenter->canBreak(State) && State.NextToken->NewlinesBefore > 0);
      Indenter->addTokenToState(State, Newline, /*DryRun=*/false);
    }
  }
private:
  ContinuationIndenter *Indenter;
};

class UnwrappedLineFormatter {
public:
  UnwrappedLineFormatter(ContinuationIndenter *Indenter,
                         const FormatStyle &Style, const AnnotatedLine &Line)
      : Indenter(Indenter), Style(Style), Line(Line), Count(0) {}

  /// \brief Formats an \c UnwrappedLine.
  void format() {
    LineState State = Indenter->getInitialState();

    // If the ObjC method declaration does not fit on a line, we should format
    // it with one arg per line.
    if (Line.Type == LT_ObjCMethodDecl)
      State.Stack.back().BreakBeforeParameter = true;

    // Find best solution in solution space.
    analyzeSolutionSpace(State);
  }

private:
  /// \brief An edge in the solution space from \c Previous->State to \c State,
  /// inserting a newline dependent on the \c NewLine.
  struct StateNode {
    StateNode(const LineState &State, bool NewLine, StateNode *Previous)
        : State(State), NewLine(NewLine), Previous(Previous) {}
    LineState State;
    bool NewLine;
    StateNode *Previous;
  };

  /// \brief A pair of <penalty, count> that is used to prioritize the BFS on.
  ///
  /// In case of equal penalties, we want to prefer states that were inserted
  /// first. During state generation we make sure that we insert states first
  /// that break the line as late as possible.
  typedef std::pair<unsigned, unsigned> OrderedPenalty;

  /// \brief An item in the prioritized BFS search queue. The \c StateNode's
  /// \c State has the given \c OrderedPenalty.
  typedef std::pair<OrderedPenalty, StateNode *> QueueItem;

  /// \brief The BFS queue type.
  typedef std::priority_queue<QueueItem, std::vector<QueueItem>,
                              std::greater<QueueItem> > QueueType;

  /// \brief Analyze the entire solution space starting from \p InitialState.
  ///
  /// This implements a variant of Dijkstra's algorithm on the graph that spans
  /// the solution space (\c LineStates are the nodes). The algorithm tries to
  /// find the shortest path (the one with lowest penalty) from \p InitialState
  /// to a state where all tokens are placed.
  void analyzeSolutionSpace(LineState &InitialState) {
    std::set<LineState> Seen;

    // Insert start element into queue.
    StateNode *Node =
        new (Allocator.Allocate()) StateNode(InitialState, false, NULL);
    Queue.push(QueueItem(OrderedPenalty(0, Count), Node));
    ++Count;

    // While not empty, take first element and follow edges.
    while (!Queue.empty()) {
      unsigned Penalty = Queue.top().first.first;
      StateNode *Node = Queue.top().second;
      if (Node->State.NextToken == NULL) {
        DEBUG(llvm::dbgs() << "\n---\nPenalty for line: " << Penalty << "\n");
        break;
      }
      Queue.pop();

      // Cut off the analysis of certain solutions if the analysis gets too
      // complex. See description of IgnoreStackForComparison.
      if (Count > 10000)
        Node->State.IgnoreStackForComparison = true;

      if (!Seen.insert(Node->State).second)
        // State already examined with lower penalty.
        continue;

      addNextStateToQueue(Penalty, Node, /*NewLine=*/false);
      addNextStateToQueue(Penalty, Node, /*NewLine=*/true);
    }

    if (Queue.empty())
      // We were unable to find a solution, do nothing.
      // FIXME: Add diagnostic?
      return;

    // Reconstruct the solution.
    reconstructPath(InitialState, Queue.top().second);
    DEBUG(llvm::dbgs() << "Total number of analyzed states: " << Count << "\n");
    DEBUG(llvm::dbgs() << "---\n");
  }

  void reconstructPath(LineState &State, StateNode *Current) {
    std::deque<StateNode *> Path;
    // We do not need a break before the initial token.
    while (Current->Previous) {
      Path.push_front(Current);
      Current = Current->Previous;
    }
    for (std::deque<StateNode *>::iterator I = Path.begin(), E = Path.end();
         I != E; ++I) {
      DEBUG({
        if ((*I)->NewLine) {
          llvm::dbgs() << "Penalty for splitting before "
                       << (*I)->Previous->State.NextToken->Tok.getName() << ": "
                       << (*I)->Previous->State.NextToken->SplitPenalty << "\n";
        }
      });
      Indenter->addTokenToState(State, (*I)->NewLine, false);
    }
  }

  /// \brief Add the following state to the analysis queue \c Queue.
  ///
  /// Assume the current state is \p PreviousNode and has been reached with a
  /// penalty of \p Penalty. Insert a line break if \p NewLine is \c true.
  void addNextStateToQueue(unsigned Penalty, StateNode *PreviousNode,
                           bool NewLine) {
    if (NewLine && !Indenter->canBreak(PreviousNode->State))
      return;
    if (!NewLine && Indenter->mustBreak(PreviousNode->State))
      return;
    if (NewLine) {
      if (!PreviousNode->State.Stack.back().ContainsLineBreak)
        Penalty += 15;
      Penalty += PreviousNode->State.NextToken->SplitPenalty;
    }

    StateNode *Node = new (Allocator.Allocate())
        StateNode(PreviousNode->State, NewLine, PreviousNode);
    Penalty += Indenter->addTokenToState(Node->State, NewLine, true);
    if (Node->State.Column > Indenter->getColumnLimit()) {
      unsigned ExcessCharacters =
          Node->State.Column - Indenter->getColumnLimit();
      Penalty += Style.PenaltyExcessCharacter * ExcessCharacters;
    }

    Queue.push(QueueItem(OrderedPenalty(Penalty, Count), Node));
    ++Count;
  }

  ContinuationIndenter *Indenter;
  FormatStyle Style;
  const AnnotatedLine &Line;

  llvm::SpecificBumpPtrAllocator<StateNode> Allocator;
  QueueType Queue;
  // Increasing count of \c StateNode items we have created. This is used
  // to create a deterministic order independent of the container.
  unsigned Count;
};

class FormatTokenLexer {
public:
  FormatTokenLexer(Lexer &Lex, SourceManager &SourceMgr,
                   encoding::Encoding Encoding)
      : FormatTok(NULL), GreaterStashed(false), TrailingWhitespace(0), Lex(Lex),
        SourceMgr(SourceMgr), IdentTable(getFormattingLangOpts()),
        Encoding(Encoding) {
    Lex.SetKeepWhitespaceMode(true);
  }

  ArrayRef<FormatToken *> lex() {
    assert(Tokens.empty());
    do {
      Tokens.push_back(getNextToken());
    } while (Tokens.back()->Tok.isNot(tok::eof));
    return Tokens;
  }

  IdentifierTable &getIdentTable() { return IdentTable; }

private:
  FormatToken *getNextToken() {
    if (GreaterStashed) {
      // Create a synthesized second '>' token.
      Token Greater = FormatTok->Tok;
      FormatTok = new (Allocator.Allocate()) FormatToken;
      FormatTok->Tok = Greater;
      SourceLocation GreaterLocation =
          FormatTok->Tok.getLocation().getLocWithOffset(1);
      FormatTok->WhitespaceRange =
          SourceRange(GreaterLocation, GreaterLocation);
      FormatTok->TokenText = ">";
      FormatTok->CodePointCount = 1;
      GreaterStashed = false;
      return FormatTok;
    }

    FormatTok = new (Allocator.Allocate()) FormatToken;
    readRawToken(*FormatTok);
    SourceLocation WhitespaceStart =
        FormatTok->Tok.getLocation().getLocWithOffset(-TrailingWhitespace);
    if (SourceMgr.getFileOffset(WhitespaceStart) == 0)
      FormatTok->IsFirst = true;

    // Consume and record whitespace until we find a significant token.
    unsigned WhitespaceLength = TrailingWhitespace;
    while (FormatTok->Tok.is(tok::unknown)) {
      unsigned Newlines = FormatTok->TokenText.count('\n');
      if (Newlines > 0)
        FormatTok->LastNewlineOffset =
            WhitespaceLength + FormatTok->TokenText.rfind('\n') + 1;
      FormatTok->NewlinesBefore += Newlines;
      unsigned EscapedNewlines = FormatTok->TokenText.count("\\\n");
      FormatTok->HasUnescapedNewline |= EscapedNewlines != Newlines;
      WhitespaceLength += FormatTok->Tok.getLength();

      readRawToken(*FormatTok);
    }

    // In case the token starts with escaped newlines, we want to
    // take them into account as whitespace - this pattern is quite frequent
    // in macro definitions.
    // FIXME: What do we want to do with other escaped spaces, and escaped
    // spaces or newlines in the middle of tokens?
    // FIXME: Add a more explicit test.
    while (FormatTok->TokenText.size() > 1 && FormatTok->TokenText[0] == '\\' &&
           FormatTok->TokenText[1] == '\n') {
      // FIXME: ++FormatTok->NewlinesBefore is missing...
      WhitespaceLength += 2;
      FormatTok->TokenText = FormatTok->TokenText.substr(2);
    }

    TrailingWhitespace = 0;
    if (FormatTok->Tok.is(tok::comment)) {
      StringRef UntrimmedText = FormatTok->TokenText;
      FormatTok->TokenText = FormatTok->TokenText.rtrim();
      TrailingWhitespace = UntrimmedText.size() - FormatTok->TokenText.size();
    } else if (FormatTok->Tok.is(tok::raw_identifier)) {
      IdentifierInfo &Info = IdentTable.get(FormatTok->TokenText);
      FormatTok->Tok.setIdentifierInfo(&Info);
      FormatTok->Tok.setKind(Info.getTokenID());
    } else if (FormatTok->Tok.is(tok::greatergreater)) {
      FormatTok->Tok.setKind(tok::greater);
      FormatTok->TokenText = FormatTok->TokenText.substr(0, 1);
      GreaterStashed = true;
    }

    // Now FormatTok is the next non-whitespace token.
    FormatTok->CodePointCount =
        encoding::getCodePointCount(FormatTok->TokenText, Encoding);

    FormatTok->WhitespaceRange = SourceRange(
        WhitespaceStart, WhitespaceStart.getLocWithOffset(WhitespaceLength));
    return FormatTok;
  }

  FormatToken *FormatTok;
  bool GreaterStashed;
  unsigned TrailingWhitespace;
  Lexer &Lex;
  SourceManager &SourceMgr;
  IdentifierTable IdentTable;
  encoding::Encoding Encoding;
  llvm::SpecificBumpPtrAllocator<FormatToken> Allocator;
  SmallVector<FormatToken *, 16> Tokens;

  void readRawToken(FormatToken &Tok) {
    Lex.LexFromRawLexer(Tok.Tok);
    Tok.TokenText = StringRef(SourceMgr.getCharacterData(Tok.Tok.getLocation()),
                              Tok.Tok.getLength());

    // For formatting, treat unterminated string literals like normal string
    // literals.
    if (Tok.is(tok::unknown) && !Tok.TokenText.empty() &&
        Tok.TokenText[0] == '"') {
      Tok.Tok.setKind(tok::string_literal);
      Tok.IsUnterminatedLiteral = true;
    }
  }
};

class Formatter : public UnwrappedLineConsumer {
public:
  Formatter(const FormatStyle &Style, Lexer &Lex, SourceManager &SourceMgr,
            const std::vector<CharSourceRange> &Ranges)
      : Style(Style), Lex(Lex), SourceMgr(SourceMgr),
        Whitespaces(SourceMgr, Style), Ranges(Ranges),
        Encoding(encoding::detectEncoding(Lex.getBuffer())) {
    DEBUG(llvm::dbgs() << "File encoding: "
                       << (Encoding == encoding::Encoding_UTF8 ? "UTF8"
                                                               : "unknown")
                       << "\n");
  }

  virtual ~Formatter() {}

  tooling::Replacements format() {
    FormatTokenLexer Tokens(Lex, SourceMgr, Encoding);

    UnwrappedLineParser Parser(Style, Tokens.lex(), *this);
    bool StructuralError = Parser.parse();
    TokenAnnotator Annotator(Style, Tokens.getIdentTable().get("in"));
    for (unsigned i = 0, e = AnnotatedLines.size(); i != e; ++i) {
      Annotator.annotate(AnnotatedLines[i]);
    }
    deriveLocalStyle();
    for (unsigned i = 0, e = AnnotatedLines.size(); i != e; ++i) {
      Annotator.calculateFormattingInformation(AnnotatedLines[i]);
    }

    // Adapt level to the next line if this is a comment.
    // FIXME: Can/should this be done in the UnwrappedLineParser?
    const AnnotatedLine *NextNonCommentLine = NULL;
    for (unsigned i = AnnotatedLines.size() - 1; i > 0; --i) {
      if (NextNonCommentLine && AnnotatedLines[i].First->is(tok::comment) &&
          !AnnotatedLines[i].First->Next)
        AnnotatedLines[i].Level = NextNonCommentLine->Level;
      else
        NextNonCommentLine = AnnotatedLines[i].First->isNot(tok::r_brace)
                                 ? &AnnotatedLines[i]
                                 : NULL;
    }

    std::vector<int> IndentForLevel;
    bool PreviousLineWasTouched = false;
    const FormatToken *PreviousLineLastToken = 0;
    bool FormatPPDirective = false;
    for (std::vector<AnnotatedLine>::iterator I = AnnotatedLines.begin(),
                                              E = AnnotatedLines.end();
         I != E; ++I) {
      const AnnotatedLine &TheLine = *I;
      const FormatToken *FirstTok = TheLine.First;
      int Offset = getIndentOffset(*TheLine.First);

      // Check whether this line is part of a formatted preprocessor directive.
      if (FirstTok->HasUnescapedNewline)
        FormatPPDirective = false;
      if (!FormatPPDirective && TheLine.InPPDirective &&
          (touchesLine(TheLine) || touchesPPDirective(I + 1, E)))
        FormatPPDirective = true;

      // Determine indent and try to merge multiple unwrapped lines.
      while (IndentForLevel.size() <= TheLine.Level)
        IndentForLevel.push_back(-1);
      IndentForLevel.resize(TheLine.Level + 1);
      unsigned Indent = getIndent(IndentForLevel, TheLine.Level);
      if (static_cast<int>(Indent) + Offset >= 0)
        Indent += Offset;
      tryFitMultipleLinesInOne(Indent, I, E);

      bool WasMoved = PreviousLineWasTouched && FirstTok->NewlinesBefore == 0;
      if (TheLine.First->is(tok::eof)) {
        if (PreviousLineWasTouched) {
          unsigned NewLines = std::min(FirstTok->NewlinesBefore, 1u);
          Whitespaces.replaceWhitespace(*TheLine.First, NewLines, /*Indent*/ 0,
                                        /*TargetColumn*/ 0);
        }
      } else if (TheLine.Type != LT_Invalid &&
                 (WasMoved || FormatPPDirective || touchesLine(TheLine))) {
        unsigned LevelIndent = getIndent(IndentForLevel, TheLine.Level);
        if (FirstTok->WhitespaceRange.isValid() &&
            // Insert a break even if there is a structural error in case where
            // we break apart a line consisting of multiple unwrapped lines.
            (FirstTok->NewlinesBefore == 0 || !StructuralError)) {
          formatFirstToken(*TheLine.First, PreviousLineLastToken, Indent,
                           TheLine.InPPDirective);
        } else {
          Indent = LevelIndent =
              SourceMgr.getSpellingColumnNumber(FirstTok->Tok.getLocation()) -
              1;
        }
        ContinuationIndenter Indenter(Style, SourceMgr, TheLine, Indent,
                                      Whitespaces, Encoding,
                                      BinPackInconclusiveFunctions);

        // If everything fits on a single line, just put it there.
        unsigned ColumnLimit = Style.ColumnLimit;
        if ((I + 1) != E && (I + 1)->InPPDirective &&
            !(I + 1)->First->HasUnescapedNewline)
          ColumnLimit = Indenter.getColumnLimit();

        if (I->Last->TotalLength + Indent <= ColumnLimit) {
          LineState State = Indenter.getInitialState();
          while (State.NextToken != NULL)
            Indenter.addTokenToState(State, false, false);
        } else if (Style.ColumnLimit == 0) {
          NoColumnLimitFormatter Formatter(&Indenter);
          Formatter.format();
        } else {
          UnwrappedLineFormatter Formatter(&Indenter, Style, TheLine);
          Formatter.format();
        }

        IndentForLevel[TheLine.Level] = LevelIndent;
        PreviousLineWasTouched = true;
      } else {
        // Format the first token if necessary, and notify the WhitespaceManager
        // about the unchanged whitespace.
        for (const FormatToken *Tok = TheLine.First; Tok != NULL;
             Tok = Tok->Next) {
          if (Tok == TheLine.First &&
              (Tok->NewlinesBefore > 0 || Tok->IsFirst)) {
            unsigned LevelIndent =
                SourceMgr.getSpellingColumnNumber(Tok->Tok.getLocation()) - 1;
            // Remove trailing whitespace of the previous line if it was
            // touched.
            if (PreviousLineWasTouched || touchesEmptyLineBefore(TheLine)) {
              formatFirstToken(*Tok, PreviousLineLastToken, LevelIndent,
                               TheLine.InPPDirective);
            } else {
              Whitespaces.addUntouchableToken(*Tok, TheLine.InPPDirective);
            }

            if (static_cast<int>(LevelIndent) - Offset >= 0)
              LevelIndent -= Offset;
            if (Tok->isNot(tok::comment))
              IndentForLevel[TheLine.Level] = LevelIndent;
          } else {
            Whitespaces.addUntouchableToken(*Tok, TheLine.InPPDirective);
          }
        }
        // If we did not reformat this unwrapped line, the column at the end of
        // the last token is unchanged - thus, we can calculate the end of the
        // last token.
        PreviousLineWasTouched = false;
      }
      PreviousLineLastToken = I->Last;
    }
    return Whitespaces.generateReplacements();
  }

private:
  void deriveLocalStyle() {
    unsigned CountBoundToVariable = 0;
    unsigned CountBoundToType = 0;
    bool HasCpp03IncompatibleFormat = false;
    bool HasBinPackedFunction = false;
    bool HasOnePerLineFunction = false;
    for (unsigned i = 0, e = AnnotatedLines.size(); i != e; ++i) {
      if (!AnnotatedLines[i].First->Next)
        continue;
      FormatToken *Tok = AnnotatedLines[i].First->Next;
      while (Tok->Next) {
        if (Tok->Type == TT_PointerOrReference) {
          bool SpacesBefore =
              Tok->WhitespaceRange.getBegin() != Tok->WhitespaceRange.getEnd();
          bool SpacesAfter = Tok->Next->WhitespaceRange.getBegin() !=
                             Tok->Next->WhitespaceRange.getEnd();
          if (SpacesBefore && !SpacesAfter)
            ++CountBoundToVariable;
          else if (!SpacesBefore && SpacesAfter)
            ++CountBoundToType;
        }

        if (Tok->Type == TT_TemplateCloser &&
            Tok->Previous->Type == TT_TemplateCloser &&
            Tok->WhitespaceRange.getBegin() == Tok->WhitespaceRange.getEnd())
          HasCpp03IncompatibleFormat = true;

        if (Tok->PackingKind == PPK_BinPacked)
          HasBinPackedFunction = true;
        if (Tok->PackingKind == PPK_OnePerLine)
          HasOnePerLineFunction = true;

        Tok = Tok->Next;
      }
    }
    if (Style.DerivePointerBinding) {
      if (CountBoundToType > CountBoundToVariable)
        Style.PointerBindsToType = true;
      else if (CountBoundToType < CountBoundToVariable)
        Style.PointerBindsToType = false;
    }
    if (Style.Standard == FormatStyle::LS_Auto) {
      Style.Standard = HasCpp03IncompatibleFormat ? FormatStyle::LS_Cpp11
                                                  : FormatStyle::LS_Cpp03;
    }
    BinPackInconclusiveFunctions =
        HasBinPackedFunction || !HasOnePerLineFunction;
  }

  /// \brief Get the indent of \p Level from \p IndentForLevel.
  ///
  /// \p IndentForLevel must contain the indent for the level \c l
  /// at \p IndentForLevel[l], or a value < 0 if the indent for
  /// that level is unknown.
  unsigned getIndent(const std::vector<int> IndentForLevel, unsigned Level) {
    if (IndentForLevel[Level] != -1)
      return IndentForLevel[Level];
    if (Level == 0)
      return 0;
    return getIndent(IndentForLevel, Level - 1) + Style.IndentWidth;
  }

  /// \brief Get the offset of the line relatively to the level.
  ///
  /// For example, 'public:' labels in classes are offset by 1 or 2
  /// characters to the left from their level.
  int getIndentOffset(const FormatToken &RootToken) {
    if (RootToken.isAccessSpecifier(false) || RootToken.isObjCAccessSpecifier())
      return Style.AccessModifierOffset;
    return 0;
  }

  /// \brief Tries to merge lines into one.
  ///
  /// This will change \c Line and \c AnnotatedLine to contain the merged line,
  /// if possible; note that \c I will be incremented when lines are merged.
  void tryFitMultipleLinesInOne(unsigned Indent,
                                std::vector<AnnotatedLine>::iterator &I,
                                std::vector<AnnotatedLine>::iterator E) {
    // We can never merge stuff if there are trailing line comments.
    if (I->Last->Type == TT_LineComment)
      return;

    if (Indent > Style.ColumnLimit)
      return;

    unsigned Limit = Style.ColumnLimit - Indent;
    // If we already exceed the column limit, we set 'Limit' to 0. The different
    // tryMerge..() functions can then decide whether to still do merging.
    Limit = I->Last->TotalLength > Limit ? 0 : Limit - I->Last->TotalLength;

    if (I + 1 == E || (I + 1)->Type == LT_Invalid)
      return;

    if (I->Last->is(tok::l_brace)) {
      tryMergeSimpleBlock(I, E, Limit);
    } else if (Style.AllowShortIfStatementsOnASingleLine &&
               I->First->is(tok::kw_if)) {
      tryMergeSimpleControlStatement(I, E, Limit);
    } else if (Style.AllowShortLoopsOnASingleLine &&
               I->First->isOneOf(tok::kw_for, tok::kw_while)) {
      tryMergeSimpleControlStatement(I, E, Limit);
    } else if (I->InPPDirective &&
               (I->First->HasUnescapedNewline || I->First->IsFirst)) {
      tryMergeSimplePPDirective(I, E, Limit);
    }
  }

  void tryMergeSimplePPDirective(std::vector<AnnotatedLine>::iterator &I,
                                 std::vector<AnnotatedLine>::iterator E,
                                 unsigned Limit) {
    if (Limit == 0)
      return;
    AnnotatedLine &Line = *I;
    if (!(I + 1)->InPPDirective || (I + 1)->First->HasUnescapedNewline)
      return;
    if (I + 2 != E && (I + 2)->InPPDirective &&
        !(I + 2)->First->HasUnescapedNewline)
      return;
    if (1 + (I + 1)->Last->TotalLength > Limit)
      return;
    join(Line, *(++I));
  }

  void tryMergeSimpleControlStatement(std::vector<AnnotatedLine>::iterator &I,
                                      std::vector<AnnotatedLine>::iterator E,
                                      unsigned Limit) {
    if (Limit == 0)
      return;
    if (Style.BreakBeforeBraces == FormatStyle::BS_Allman &&
        (I + 1)->First->is(tok::l_brace))
      return;
    if ((I + 1)->InPPDirective != I->InPPDirective ||
        ((I + 1)->InPPDirective && (I + 1)->First->HasUnescapedNewline))
      return;
    AnnotatedLine &Line = *I;
    if (Line.Last->isNot(tok::r_paren))
      return;
    if (1 + (I + 1)->Last->TotalLength > Limit)
      return;
    if ((I + 1)->First->isOneOf(tok::semi, tok::kw_if, tok::kw_for,
                                tok::kw_while) ||
        (I + 1)->First->Type == TT_LineComment)
      return;
    // Only inline simple if's (no nested if or else).
    if (I + 2 != E && Line.First->is(tok::kw_if) &&
        (I + 2)->First->is(tok::kw_else))
      return;
    join(Line, *(++I));
  }

  void tryMergeSimpleBlock(std::vector<AnnotatedLine>::iterator &I,
                           std::vector<AnnotatedLine>::iterator E,
                           unsigned Limit) {
    // No merging if the brace already is on the next line.
    if (Style.BreakBeforeBraces != FormatStyle::BS_Attach)
      return;

    // First, check that the current line allows merging. This is the case if
    // we're not in a control flow statement and the last token is an opening
    // brace.
    AnnotatedLine &Line = *I;
    if (Line.First->isOneOf(tok::kw_if, tok::kw_while, tok::kw_do, tok::r_brace,
                            tok::kw_else, tok::kw_try, tok::kw_catch,
                            tok::kw_for,
                            // This gets rid of all ObjC @ keywords and methods.
                            tok::at, tok::minus, tok::plus))
      return;

    FormatToken *Tok = (I + 1)->First;
    if (Tok->is(tok::r_brace) && !Tok->MustBreakBefore &&
        (Tok->getNextNonComment() == NULL ||
         Tok->getNextNonComment()->is(tok::semi))) {
      // We merge empty blocks even if the line exceeds the column limit.
      Tok->SpacesRequiredBefore = 0;
      Tok->CanBreakBefore = true;
      join(Line, *(I + 1));
      I += 1;
    } else if (Limit != 0 && Line.First->isNot(tok::kw_namespace)) {
      // Check that we still have three lines and they fit into the limit.
      if (I + 2 == E || (I + 2)->Type == LT_Invalid ||
          !nextTwoLinesFitInto(I, Limit))
        return;

      // Second, check that the next line does not contain any braces - if it
      // does, readability declines when putting it into a single line.
      if ((I + 1)->Last->Type == TT_LineComment || Tok->MustBreakBefore)
        return;
      do {
        if (Tok->isOneOf(tok::l_brace, tok::r_brace))
          return;
        Tok = Tok->Next;
      } while (Tok != NULL);

      // Last, check that the third line contains a single closing brace.
      Tok = (I + 2)->First;
      if (Tok->getNextNonComment() != NULL || Tok->isNot(tok::r_brace) ||
          Tok->MustBreakBefore)
        return;

      join(Line, *(I + 1));
      join(Line, *(I + 2));
      I += 2;
    }
  }

  bool nextTwoLinesFitInto(std::vector<AnnotatedLine>::iterator I,
                           unsigned Limit) {
    return 1 + (I + 1)->Last->TotalLength + 1 + (I + 2)->Last->TotalLength <=
           Limit;
  }

  void join(AnnotatedLine &A, const AnnotatedLine &B) {
    assert(!A.Last->Next);
    assert(!B.First->Previous);
    A.Last->Next = B.First;
    B.First->Previous = A.Last;
    unsigned LengthA = A.Last->TotalLength + B.First->SpacesRequiredBefore;
    for (FormatToken *Tok = B.First; Tok; Tok = Tok->Next) {
      Tok->TotalLength += LengthA;
      A.Last = Tok;
    }
  }

  bool touchesRanges(const CharSourceRange &Range) {
    for (unsigned i = 0, e = Ranges.size(); i != e; ++i) {
      if (!SourceMgr.isBeforeInTranslationUnit(Range.getEnd(),
                                               Ranges[i].getBegin()) &&
          !SourceMgr.isBeforeInTranslationUnit(Ranges[i].getEnd(),
                                               Range.getBegin()))
        return true;
    }
    return false;
  }

  bool touchesLine(const AnnotatedLine &TheLine) {
    const FormatToken *First = TheLine.First;
    const FormatToken *Last = TheLine.Last;
    CharSourceRange LineRange = CharSourceRange::getCharRange(
        First->WhitespaceRange.getBegin().getLocWithOffset(
            First->LastNewlineOffset),
        Last->Tok.getLocation().getLocWithOffset(Last->TokenText.size() - 1));
    return touchesRanges(LineRange);
  }

  bool touchesPPDirective(std::vector<AnnotatedLine>::iterator I,
                          std::vector<AnnotatedLine>::iterator E) {
    for (; I != E; ++I) {
      if (I->First->HasUnescapedNewline)
        return false;
      if (touchesLine(*I))
        return true;
    }
    return false;
  }

  bool touchesEmptyLineBefore(const AnnotatedLine &TheLine) {
    const FormatToken *First = TheLine.First;
    CharSourceRange LineRange = CharSourceRange::getCharRange(
        First->WhitespaceRange.getBegin(),
        First->WhitespaceRange.getBegin().getLocWithOffset(
            First->LastNewlineOffset));
    return touchesRanges(LineRange);
  }

  virtual void consumeUnwrappedLine(const UnwrappedLine &TheLine) {
    AnnotatedLines.push_back(AnnotatedLine(TheLine));
  }

  /// \brief Add a new line and the required indent before the first Token
  /// of the \c UnwrappedLine if there was no structural parsing error.
  /// Returns the indent level of the \c UnwrappedLine.
  void formatFirstToken(const FormatToken &RootToken,
                        const FormatToken *PreviousToken, unsigned Indent,
                        bool InPPDirective) {
    unsigned Newlines =
        std::min(RootToken.NewlinesBefore, Style.MaxEmptyLinesToKeep + 1);
    // Remove empty lines before "}" where applicable.
    if (RootToken.is(tok::r_brace) &&
        (!RootToken.Next ||
         (RootToken.Next->is(tok::semi) && !RootToken.Next->Next)))
      Newlines = std::min(Newlines, 1u);
    if (Newlines == 0 && !RootToken.IsFirst)
      Newlines = 1;

    // Insert extra new line before access specifiers.
    if (PreviousToken && PreviousToken->isOneOf(tok::semi, tok::r_brace) &&
        RootToken.isAccessSpecifier() && RootToken.NewlinesBefore == 1)
      ++Newlines;

    Whitespaces.replaceWhitespace(
        RootToken, Newlines, Indent, Indent,
        InPPDirective && !RootToken.HasUnescapedNewline);
  }

  FormatStyle Style;
  Lexer &Lex;
  SourceManager &SourceMgr;
  WhitespaceManager Whitespaces;
  std::vector<CharSourceRange> Ranges;
  std::vector<AnnotatedLine> AnnotatedLines;

  encoding::Encoding Encoding;
  bool BinPackInconclusiveFunctions;
};

} // end anonymous namespace

tooling::Replacements reformat(const FormatStyle &Style, Lexer &Lex,
                               SourceManager &SourceMgr,
                               std::vector<CharSourceRange> Ranges) {
  Formatter formatter(Style, Lex, SourceMgr, Ranges);
  return formatter.format();
}

tooling::Replacements reformat(const FormatStyle &Style, StringRef Code,
                               std::vector<tooling::Range> Ranges,
                               StringRef FileName) {
  FileManager Files((FileSystemOptions()));
  DiagnosticsEngine Diagnostics(
      IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs),
      new DiagnosticOptions);
  SourceManager SourceMgr(Diagnostics, Files);
  llvm::MemoryBuffer *Buf = llvm::MemoryBuffer::getMemBuffer(Code, FileName);
  const clang::FileEntry *Entry =
      Files.getVirtualFile(FileName, Buf->getBufferSize(), 0);
  SourceMgr.overrideFileContents(Entry, Buf);
  FileID ID =
      SourceMgr.createFileID(Entry, SourceLocation(), clang::SrcMgr::C_User);
  Lexer Lex(ID, SourceMgr.getBuffer(ID), SourceMgr,
            getFormattingLangOpts(Style.Standard));
  SourceLocation StartOfFile = SourceMgr.getLocForStartOfFile(ID);
  std::vector<CharSourceRange> CharRanges;
  for (unsigned i = 0, e = Ranges.size(); i != e; ++i) {
    SourceLocation Start = StartOfFile.getLocWithOffset(Ranges[i].getOffset());
    SourceLocation End = Start.getLocWithOffset(Ranges[i].getLength());
    CharRanges.push_back(CharSourceRange::getCharRange(Start, End));
  }
  return reformat(Style, Lex, SourceMgr, CharRanges);
}

LangOptions getFormattingLangOpts(FormatStyle::LanguageStandard Standard) {
  LangOptions LangOpts;
  LangOpts.CPlusPlus = 1;
  LangOpts.CPlusPlus11 = Standard == FormatStyle::LS_Cpp03 ? 0 : 1;
  LangOpts.LineComment = 1;
  LangOpts.Bool = 1;
  LangOpts.ObjC1 = 1;
  LangOpts.ObjC2 = 1;
  return LangOpts;
}

} // namespace format
} // namespace clang
