//===--- ContinuationIndenter.cpp - Format C++ code -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements the continuation indenter.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "format-formatter"

#include "BreakableToken.h"
#include "ContinuationIndenter.h"
#include "WhitespaceManager.h"
#include "clang/Basic/OperatorPrecedence.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Format/Format.h"
#include "llvm/Support/Debug.h"
#include <string>

namespace clang {
namespace format {

// Returns the length of everything up to the first possible line break after
// the ), ], } or > matching \c Tok.
static unsigned getLengthToMatchingParen(const FormatToken &Tok) {
  if (Tok.MatchingParen == NULL)
    return 0;
  FormatToken *End = Tok.MatchingParen;
  while (End->Next && !End->Next->CanBreakBefore) {
    End = End->Next;
  }
  return End->TotalLength - Tok.TotalLength + 1;
}

ContinuationIndenter::ContinuationIndenter(const FormatStyle &Style,
                                           SourceManager &SourceMgr,
                                           const AnnotatedLine &Line,
                                           unsigned FirstIndent,
                                           WhitespaceManager &Whitespaces,
                                           encoding::Encoding Encoding,
                                           bool BinPackInconclusiveFunctions)
    : Style(Style), SourceMgr(SourceMgr), Line(Line), FirstIndent(FirstIndent),
      Whitespaces(Whitespaces), Encoding(Encoding),
      BinPackInconclusiveFunctions(BinPackInconclusiveFunctions) {}

LineState ContinuationIndenter::getInitialState() {
  // Initialize state dependent on indent.
  LineState State;
  State.Column = FirstIndent;
  State.NextToken = Line.First;
  State.Stack.push_back(ParenState(FirstIndent, FirstIndent,
                                   /*AvoidBinPacking=*/false,
                                   /*NoLineBreak=*/false));
  State.LineContainsContinuedForLoopSection = false;
  State.ParenLevel = 0;
  State.StartOfStringLiteral = 0;
  State.StartOfLineLevel = State.ParenLevel;
  State.LowestLevelOnLine = State.ParenLevel;
  State.IgnoreStackForComparison = false;

  // The first token has already been indented and thus consumed.
  moveStateToNextToken(State, /*DryRun=*/false,
                       /*Newline=*/false);
  return State;
}

bool ContinuationIndenter::canBreak(const LineState &State) {
  const FormatToken &Current = *State.NextToken;
  const FormatToken &Previous = *Current.Previous;
  assert(&Previous == Current.Previous);
  if (!Current.CanBreakBefore &&
      !(Current.is(tok::r_brace) && State.Stack.back().BreakBeforeClosingBrace))
    return false;
  // The opening "{" of a braced list has to be on the same line as the first
  // element if it is nested in another braced init list or function call.
  if (!Current.MustBreakBefore && Previous.is(tok::l_brace) &&
      Previous.Previous &&
      Previous.Previous->isOneOf(tok::l_brace, tok::l_paren, tok::comma))
    return false;
  // This prevents breaks like:
  //   ...
  //   SomeParameter, OtherParameter).DoSomething(
  //   ...
  // As they hide "DoSomething" and are generally bad for readability.
  if (Previous.opensScope() && State.LowestLevelOnLine < State.StartOfLineLevel)
    return false;
  return !State.Stack.back().NoLineBreak;
}

bool ContinuationIndenter::mustBreak(const LineState &State) {
  const FormatToken &Current = *State.NextToken;
  const FormatToken &Previous = *Current.Previous;
  if (Current.MustBreakBefore || Current.Type == TT_InlineASMColon)
    return true;
  if (!Style.Cpp11BracedListStyle && Current.is(tok::r_brace) &&
      State.Stack.back().BreakBeforeClosingBrace)
    return true;
  if (Previous.is(tok::semi) && State.LineContainsContinuedForLoopSection)
    return true;
  if (Style.BreakConstructorInitializersBeforeComma) {
    if (Previous.Type == TT_CtorInitializerComma)
      return false;
    if (Current.Type == TT_CtorInitializerComma)
      return true;
  }
  if ((Previous.isOneOf(tok::comma, tok::semi) || Current.is(tok::question) ||
       (Current.Type == TT_ConditionalExpr &&
        !(Current.is(tok::colon) && Previous.is(tok::question)))) &&
      State.Stack.back().BreakBeforeParameter && !Current.isTrailingComment() &&
      !Current.isOneOf(tok::r_paren, tok::r_brace))
    return true;
  if (Style.AlwaysBreakBeforeMultilineStrings &&
      State.Column > State.Stack.back().Indent &&
      Current.is(tok::string_literal) && Previous.isNot(tok::lessless) &&
      Previous.Type != TT_InlineASMColon &&
      ((Current.getNextNonComment() &&
        Current.getNextNonComment()->is(tok::string_literal)) ||
       (Current.TokenText.find("\\\n") != StringRef::npos)))
    return true;

  if (!Style.BreakBeforeBinaryOperators) {
    // If we need to break somewhere inside the LHS of a binary expression, we
    // should also break after the operator. Otherwise, the formatting would
    // hide the operator precedence, e.g. in:
    //   if (aaaaaaaaaaaaaa ==
    //           bbbbbbbbbbbbbb && c) {..
    // For comparisons, we only apply this rule, if the LHS is a binary
    // expression itself as otherwise, the line breaks seem superfluous.
    // We need special cases for ">>" which we have split into two ">" while
    // lexing in order to make template parsing easier.
    //
    // FIXME: We'll need something similar for styles that break before binary
    // operators.
    bool IsComparison = (Previous.getPrecedence() == prec::Relational ||
                         Previous.getPrecedence() == prec::Equality) &&
                        Previous.Previous &&
                        Previous.Previous->Type != TT_BinaryOperator; // For >>.
    bool LHSIsBinaryExpr =
        Previous.Previous && Previous.Previous->FakeRParens > 0;
    if (Previous.Type == TT_BinaryOperator &&
        (!IsComparison || LHSIsBinaryExpr) &&
        Current.Type != TT_BinaryOperator && // For >>.
        !Current.isTrailingComment() &&
        !Previous.isOneOf(tok::lessless, tok::question) &&
        Previous.getPrecedence() != prec::Assignment &&
        State.Stack.back().BreakBeforeParameter)
      return true;
  }

  // Same as above, but for the first "<<" operator.
  if (Current.is(tok::lessless) && State.Stack.back().BreakBeforeParameter &&
      State.Stack.back().FirstLessLess == 0)
    return true;

  // FIXME: Comparing LongestObjCSelectorName to 0 is a hacky way of finding
  // out whether it is the first parameter. Clean this up.
  if (Current.Type == TT_ObjCSelectorName &&
      Current.LongestObjCSelectorName == 0 &&
      State.Stack.back().BreakBeforeParameter)
    return true;
  if ((Current.Type == TT_CtorInitializerColon ||
       (Previous.ClosesTemplateDeclaration && State.ParenLevel == 0)))
    return true;

  if ((Current.Type == TT_StartOfName || Current.is(tok::kw_operator)) &&
      Line.MightBeFunctionDecl && State.Stack.back().BreakBeforeParameter &&
      State.ParenLevel == 0)
    return true;
  return false;
}

unsigned ContinuationIndenter::addTokenToState(LineState &State, bool Newline,
                                               bool DryRun) {
  const FormatToken &Current = *State.NextToken;
  const FormatToken &Previous = *State.NextToken->Previous;

  // Extra penalty that needs to be added because of the way certain line
  // breaks are chosen.
  unsigned ExtraPenalty = 0;

  if (State.Stack.size() == 0 || Current.Type == TT_ImplicitStringLiteral) {
    // FIXME: Is this correct?
    int WhitespaceLength = SourceMgr.getSpellingColumnNumber(
                               State.NextToken->WhitespaceRange.getEnd()) -
                           SourceMgr.getSpellingColumnNumber(
                               State.NextToken->WhitespaceRange.getBegin());
    State.Column += WhitespaceLength + State.NextToken->CodePointCount;
    State.NextToken = State.NextToken->Next;
    return 0;
  }

  // If we are continuing an expression, we want to indent an extra 4 spaces.
  unsigned ContinuationIndent =
      std::max(State.Stack.back().LastSpace, State.Stack.back().Indent) + 4;
  if (Newline) {
    // Breaking before the first "<<" is generally not desirable if the LHS is
    // short.
    if (Current.is(tok::lessless) && State.Stack.back().FirstLessLess == 0 &&
        State.Column <= Style.ColumnLimit / 2)
      ExtraPenalty += Style.PenaltyBreakFirstLessLess;

    State.Stack.back().ContainsLineBreak = true;
    if (Current.is(tok::r_brace)) {
      if (Current.BlockKind == BK_BracedInit)
        State.Column = State.Stack[State.Stack.size() - 2].LastSpace;
      else
        State.Column = FirstIndent;
    } else if (Current.is(tok::string_literal) &&
               State.StartOfStringLiteral != 0) {
      State.Column = State.StartOfStringLiteral;
      State.Stack.back().BreakBeforeParameter = true;
    } else if (Current.is(tok::lessless) &&
               State.Stack.back().FirstLessLess != 0) {
      State.Column = State.Stack.back().FirstLessLess;
    } else if (Current.isOneOf(tok::period, tok::arrow) &&
               Current.Type != TT_DesignatedInitializerPeriod) {
      if (State.Stack.back().CallContinuation == 0) {
        State.Column = ContinuationIndent;
        State.Stack.back().CallContinuation = State.Column;
      } else {
        State.Column = State.Stack.back().CallContinuation;
      }
    } else if (Current.Type == TT_ConditionalExpr) {
      State.Column = State.Stack.back().QuestionColumn;
    } else if (Previous.is(tok::comma) && State.Stack.back().VariablePos != 0) {
      State.Column = State.Stack.back().VariablePos;
    } else if (Previous.ClosesTemplateDeclaration ||
               ((Current.Type == TT_StartOfName ||
                 Current.is(tok::kw_operator)) &&
                State.ParenLevel == 0 &&
                (!Style.IndentFunctionDeclarationAfterType ||
                 Line.StartsDefinition))) {
      State.Column = State.Stack.back().Indent;
    } else if (Current.Type == TT_ObjCSelectorName) {
      if (State.Stack.back().ColonPos > Current.CodePointCount) {
        State.Column = State.Stack.back().ColonPos - Current.CodePointCount;
      } else {
        State.Column = State.Stack.back().Indent;
        State.Stack.back().ColonPos = State.Column + Current.CodePointCount;
      }
    } else if (Current.is(tok::l_square) && Current.Type != TT_ObjCMethodExpr) {
      if (State.Stack.back().StartOfArraySubscripts != 0)
        State.Column = State.Stack.back().StartOfArraySubscripts;
      else
        State.Column = ContinuationIndent;
    } else if (Current.Type == TT_StartOfName ||
               Previous.isOneOf(tok::coloncolon, tok::equal) ||
               Previous.Type == TT_ObjCMethodExpr) {
      State.Column = ContinuationIndent;
    } else if (Current.Type == TT_CtorInitializerColon) {
      State.Column = FirstIndent + Style.ConstructorInitializerIndentWidth;
    } else if (Current.Type == TT_CtorInitializerComma) {
      State.Column = State.Stack.back().Indent;
    } else {
      State.Column = State.Stack.back().Indent;
      // Ensure that we fall back to indenting 4 spaces instead of just
      // flushing continuations left.
      if (State.Column == FirstIndent)
        State.Column += 4;
    }

    if (Current.is(tok::question))
      State.Stack.back().BreakBeforeParameter = true;
    if ((Previous.isOneOf(tok::comma, tok::semi) &&
         !State.Stack.back().AvoidBinPacking) ||
        Previous.Type == TT_BinaryOperator)
      State.Stack.back().BreakBeforeParameter = false;
    if (Previous.Type == TT_TemplateCloser && State.ParenLevel == 0)
      State.Stack.back().BreakBeforeParameter = false;

    if (!DryRun) {
      unsigned NewLines = 1;
      if (Current.is(tok::comment))
        NewLines = std::max(NewLines, std::min(Current.NewlinesBefore,
                                               Style.MaxEmptyLinesToKeep + 1));
      Whitespaces.replaceWhitespace(Current, NewLines, State.Column,
                                    State.Column, Line.InPPDirective);
    }

    if (!Current.isTrailingComment())
      State.Stack.back().LastSpace = State.Column;
    if (Current.isOneOf(tok::arrow, tok::period) &&
        Current.Type != TT_DesignatedInitializerPeriod)
      State.Stack.back().LastSpace += Current.CodePointCount;
    State.StartOfLineLevel = State.ParenLevel;
    State.LowestLevelOnLine = State.ParenLevel;

    // Any break on this level means that the parent level has been broken
    // and we need to avoid bin packing there.
    for (unsigned i = 0, e = State.Stack.size() - 1; i != e; ++i) {
      State.Stack[i].BreakBeforeParameter = true;
    }
    const FormatToken *TokenBefore = Current.getPreviousNonComment();
    if (TokenBefore && !TokenBefore->isOneOf(tok::comma, tok::semi) &&
        TokenBefore->Type != TT_TemplateCloser &&
        TokenBefore->Type != TT_BinaryOperator && !TokenBefore->opensScope())
      State.Stack.back().BreakBeforeParameter = true;

    // If we break after {, we should also break before the corresponding }.
    if (Previous.is(tok::l_brace))
      State.Stack.back().BreakBeforeClosingBrace = true;

    if (State.Stack.back().AvoidBinPacking) {
      // If we are breaking after '(', '{', '<', this is not bin packing
      // unless AllowAllParametersOfDeclarationOnNextLine is false.
      if (!(Previous.isOneOf(tok::l_paren, tok::l_brace) ||
            Previous.Type == TT_BinaryOperator) ||
          (!Style.AllowAllParametersOfDeclarationOnNextLine &&
           Line.MustBeDeclaration))
        State.Stack.back().BreakBeforeParameter = true;
    }

  } else {
    if (Current.is(tok::equal) &&
        (Line.First->is(tok::kw_for) || State.ParenLevel == 0) &&
        State.Stack.back().VariablePos == 0) {
      State.Stack.back().VariablePos = State.Column;
      // Move over * and & if they are bound to the variable name.
      const FormatToken *Tok = &Previous;
      while (Tok && State.Stack.back().VariablePos >= Tok->CodePointCount) {
        State.Stack.back().VariablePos -= Tok->CodePointCount;
        if (Tok->SpacesRequiredBefore != 0)
          break;
        Tok = Tok->Previous;
      }
      if (Previous.PartOfMultiVariableDeclStmt)
        State.Stack.back().LastSpace = State.Stack.back().VariablePos;
    }

    unsigned Spaces = State.NextToken->SpacesRequiredBefore;

    if (!DryRun)
      Whitespaces.replaceWhitespace(Current, 0, Spaces, State.Column + Spaces);

    if (Current.Type == TT_ObjCSelectorName &&
        State.Stack.back().ColonPos == 0) {
      if (State.Stack.back().Indent + Current.LongestObjCSelectorName >
          State.Column + Spaces + Current.CodePointCount)
        State.Stack.back().ColonPos =
            State.Stack.back().Indent + Current.LongestObjCSelectorName;
      else
        State.Stack.back().ColonPos =
            State.Column + Spaces + Current.CodePointCount;
    }

    if (Previous.opensScope() && Previous.Type != TT_ObjCMethodExpr &&
        Current.Type != TT_LineComment)
      State.Stack.back().Indent = State.Column + Spaces;
    if (Previous.is(tok::comma) && !Current.isTrailingComment() &&
        State.Stack.back().AvoidBinPacking)
      State.Stack.back().NoLineBreak = true;

    State.Column += Spaces;
    if (Current.is(tok::l_paren) && Previous.isOneOf(tok::kw_if, tok::kw_for))
      // Treat the condition inside an if as if it was a second function
      // parameter, i.e. let nested calls have an indent of 4.
      State.Stack.back().LastSpace = State.Column + 1; // 1 is length of "(".
    else if (Previous.is(tok::comma))
      State.Stack.back().LastSpace = State.Column;
    else if ((Previous.Type == TT_BinaryOperator ||
              Previous.Type == TT_ConditionalExpr ||
              Previous.Type == TT_CtorInitializerColon) &&
             !(Previous.getPrecedence() == prec::Assignment &&
               Current.FakeLParens.empty()))
      // Always indent relative to the RHS of the expression unless this is a
      // simple assignment without binary expression on the RHS.
      State.Stack.back().LastSpace = State.Column;
    else if (Previous.Type == TT_InheritanceColon)
      State.Stack.back().Indent = State.Column;
    else if (Previous.opensScope()) {
      // If a function has multiple parameters (including a single parameter
      // that is a binary expression) or a trailing call, indent all
      // parameters from the opening parenthesis. This avoids confusing
      // indents like:
      //   OuterFunction(InnerFunctionCall(
      //       ParameterToInnerFunction),
      //                 SecondParameterToOuterFunction);
      bool HasMultipleParameters = !Current.FakeLParens.empty();
      bool HasTrailingCall = false;
      if (Previous.MatchingParen) {
        const FormatToken *Next = Previous.MatchingParen->getNextNonComment();
        if (Next && Next->isOneOf(tok::period, tok::arrow))
          HasTrailingCall = true;
      }
      if (HasMultipleParameters || HasTrailingCall)
        State.Stack.back().LastSpace = State.Column;
    }
  }

  return moveStateToNextToken(State, DryRun, Newline) + ExtraPenalty;
}

unsigned ContinuationIndenter::moveStateToNextToken(LineState &State,
                                                    bool DryRun, bool Newline) {
  const FormatToken &Current = *State.NextToken;
  assert(State.Stack.size());

  if (Current.Type == TT_InheritanceColon)
    State.Stack.back().AvoidBinPacking = true;
  if (Current.is(tok::lessless) && State.Stack.back().FirstLessLess == 0)
    State.Stack.back().FirstLessLess = State.Column;
  if (Current.is(tok::l_square) &&
      State.Stack.back().StartOfArraySubscripts == 0)
    State.Stack.back().StartOfArraySubscripts = State.Column;
  if (Current.is(tok::question))
    State.Stack.back().QuestionColumn = State.Column;
  if (!Current.opensScope() && !Current.closesScope())
    State.LowestLevelOnLine =
        std::min(State.LowestLevelOnLine, State.ParenLevel);
  if (Current.isOneOf(tok::period, tok::arrow) &&
      Line.Type == LT_BuilderTypeCall && State.ParenLevel == 0)
    State.Stack.back().StartOfFunctionCall =
        Current.LastInChainOfCalls ? 0 : State.Column + Current.CodePointCount;
  if (Current.Type == TT_CtorInitializerColon) {
    // Indent 2 from the column, so:
    // SomeClass::SomeClass()
    //     : First(...), ...
    //       Next(...)
    //       ^ line up here.
    State.Stack.back().Indent =
        State.Column + (Style.BreakConstructorInitializersBeforeComma ? 0 : 2);
    if (Style.ConstructorInitializerAllOnOneLineOrOnePerLine)
      State.Stack.back().AvoidBinPacking = true;
    State.Stack.back().BreakBeforeParameter = false;
  }

  // If return returns a binary expression, align after it.
  if (Current.is(tok::kw_return) && !Current.FakeLParens.empty())
    State.Stack.back().LastSpace = State.Column + 7;

  // In ObjC method declaration we align on the ":" of parameters, but we need
  // to ensure that we indent parameters on subsequent lines by at least 4.
  if (Current.Type == TT_ObjCMethodSpecifier)
    State.Stack.back().Indent += 4;

  // Insert scopes created by fake parenthesis.
  const FormatToken *Previous = Current.getPreviousNonComment();
  // Don't add extra indentation for the first fake parenthesis after
  // 'return', assignements or opening <({[. The indentation for these cases
  // is special cased.
  bool SkipFirstExtraIndent =
      Current.is(tok::kw_return) ||
      (Previous && (Previous->opensScope() ||
                    Previous->getPrecedence() == prec::Assignment));
  for (SmallVectorImpl<prec::Level>::const_reverse_iterator
           I = Current.FakeLParens.rbegin(),
           E = Current.FakeLParens.rend();
       I != E; ++I) {
    ParenState NewParenState = State.Stack.back();
    NewParenState.ContainsLineBreak = false;
    NewParenState.Indent =
        std::max(std::max(State.Column, NewParenState.Indent),
                 State.Stack.back().LastSpace);

    // Always indent conditional expressions. Never indent expression where
    // the 'operator' is ',', ';' or an assignment (i.e. *I <=
    // prec::Assignment) as those have different indentation rules. Indent
    // other expression, unless the indentation needs to be skipped.
    if (*I == prec::Conditional ||
        (!SkipFirstExtraIndent && *I > prec::Assignment &&
         !Style.BreakBeforeBinaryOperators))
      NewParenState.Indent += 4;
    if (Previous && !Previous->opensScope())
      NewParenState.BreakBeforeParameter = false;
    State.Stack.push_back(NewParenState);
    SkipFirstExtraIndent = false;
  }

  // If we encounter an opening (, [, { or <, we add a level to our stacks to
  // prepare for the following tokens.
  if (Current.opensScope()) {
    unsigned NewIndent;
    unsigned LastSpace = State.Stack.back().LastSpace;
    bool AvoidBinPacking;
    if (Current.is(tok::l_brace)) {
      NewIndent =
          LastSpace + (Style.Cpp11BracedListStyle ? 4 : Style.IndentWidth);
      const FormatToken *NextNoComment = Current.getNextNonComment();
      AvoidBinPacking = NextNoComment &&
                        NextNoComment->Type == TT_DesignatedInitializerPeriod;
    } else {
      NewIndent =
          4 + std::max(LastSpace, State.Stack.back().StartOfFunctionCall);
      AvoidBinPacking = !Style.BinPackParameters ||
                        (Style.ExperimentalAutoDetectBinPacking &&
                         (Current.PackingKind == PPK_OnePerLine ||
                          (!BinPackInconclusiveFunctions &&
                           Current.PackingKind == PPK_Inconclusive)));
    }

    State.Stack.push_back(ParenState(NewIndent, LastSpace, AvoidBinPacking,
                                     State.Stack.back().NoLineBreak));
    ++State.ParenLevel;
  }

  // If this '[' opens an ObjC call, determine whether all parameters fit into
  // one line and put one per line if they don't.
  if (Current.is(tok::l_square) && Current.Type == TT_ObjCMethodExpr &&
      Current.MatchingParen != NULL) {
    if (getLengthToMatchingParen(Current) + State.Column > getColumnLimit())
      State.Stack.back().BreakBeforeParameter = true;
  }

  // If we encounter a closing ), ], } or >, we can remove a level from our
  // stacks.
  if (Current.isOneOf(tok::r_paren, tok::r_square) ||
      (Current.is(tok::r_brace) && State.NextToken != Line.First) ||
      State.NextToken->Type == TT_TemplateCloser) {
    State.Stack.pop_back();
    --State.ParenLevel;
  }
  if (Current.is(tok::r_square)) {
    // If this ends the array subscript expr, reset the corresponding value.
    const FormatToken *NextNonComment = Current.getNextNonComment();
    if (NextNonComment && NextNonComment->isNot(tok::l_square))
      State.Stack.back().StartOfArraySubscripts = 0;
  }

  // Remove scopes created by fake parenthesis.
  for (unsigned i = 0, e = Current.FakeRParens; i != e; ++i) {
    unsigned VariablePos = State.Stack.back().VariablePos;
    State.Stack.pop_back();
    State.Stack.back().VariablePos = VariablePos;
  }

  if (Current.is(tok::string_literal) && State.StartOfStringLiteral == 0) {
    State.StartOfStringLiteral = State.Column;
  } else if (!Current.isOneOf(tok::comment, tok::identifier, tok::hash,
                              tok::string_literal)) {
    State.StartOfStringLiteral = 0;
  }

  State.Column += Current.CodePointCount;

  State.NextToken = State.NextToken->Next;

  if (!Newline && Style.AlwaysBreakBeforeMultilineStrings &&
      Current.is(tok::string_literal) && Current.CanBreakBefore)
    return 0;

  return breakProtrudingToken(Current, State, DryRun);
}

unsigned ContinuationIndenter::breakProtrudingToken(const FormatToken &Current,
                                                    LineState &State,
                                                    bool DryRun) {
  llvm::OwningPtr<BreakableToken> Token;
  unsigned StartColumn = State.Column - Current.CodePointCount;
  unsigned OriginalStartColumn =
      SourceMgr.getSpellingColumnNumber(Current.getStartOfNonWhitespace()) - 1;

  if (Current.is(tok::string_literal) &&
      Current.Type != TT_ImplicitStringLiteral) {
    // Only break up default narrow strings.
    if (!Current.TokenText.startswith("\""))
      return 0;
    // Don't break string literals with escaped newlines. As clang-format must
    // not change the string's content, it is unlikely that we'll end up with
    // a better format.
    if (Current.TokenText.find("\\\n") != StringRef::npos)
      return 0;
    // Exempts unterminated string literals from line breaking. The user will
    // likely want to terminate the string before any line breaking is done.
    if (Current.IsUnterminatedLiteral)
      return 0;

    Token.reset(new BreakableStringLiteral(Current, StartColumn,
                                           Line.InPPDirective, Encoding));
  } else if (Current.Type == TT_BlockComment && Current.isTrailingComment()) {
    Token.reset(new BreakableBlockComment(
        Style, Current, StartColumn, OriginalStartColumn, !Current.Previous,
        Line.InPPDirective, Encoding));
  } else if (Current.Type == TT_LineComment &&
             (Current.Previous == NULL ||
              Current.Previous->Type != TT_ImplicitStringLiteral)) {
    // Don't break line comments with escaped newlines. These look like
    // separate line comments, but in fact contain a single line comment with
    // multiple lines including leading whitespace and the '//' markers.
    //
    // FIXME: If we want to handle them correctly, we'll need to adjust
    // leading whitespace in consecutive lines when changing indentation of
    // the first line similar to what we do with block comments.
    StringRef::size_type EscapedNewlinePos = Current.TokenText.find("\\\n");
    if (EscapedNewlinePos != StringRef::npos) {
      State.Column =
          StartColumn +
          encoding::getCodePointCount(
              Current.TokenText.substr(0, EscapedNewlinePos), Encoding) +
          1;
      return 0;
    }

    Token.reset(new BreakableLineComment(Current, StartColumn,
                                         Line.InPPDirective, Encoding));
  } else {
    return 0;
  }
  if (Current.UnbreakableTailLength >= getColumnLimit())
    return 0;

  unsigned RemainingSpace = getColumnLimit() - Current.UnbreakableTailLength;
  bool BreakInserted = false;
  unsigned Penalty = 0;
  unsigned RemainingTokenColumns = 0;
  for (unsigned LineIndex = 0, EndIndex = Token->getLineCount();
       LineIndex != EndIndex; ++LineIndex) {
    if (!DryRun)
      Token->replaceWhitespaceBefore(LineIndex, Whitespaces);
    unsigned TailOffset = 0;
    RemainingTokenColumns =
        Token->getLineLengthAfterSplit(LineIndex, TailOffset, StringRef::npos);
    while (RemainingTokenColumns > RemainingSpace) {
      BreakableToken::Split Split =
          Token->getSplit(LineIndex, TailOffset, getColumnLimit());
      if (Split.first == StringRef::npos) {
        // The last line's penalty is handled in addNextStateToQueue().
        if (LineIndex < EndIndex - 1)
          Penalty += Style.PenaltyExcessCharacter *
                     (RemainingTokenColumns - RemainingSpace);
        break;
      }
      assert(Split.first != 0);
      unsigned NewRemainingTokenColumns = Token->getLineLengthAfterSplit(
          LineIndex, TailOffset + Split.first + Split.second, StringRef::npos);
      assert(NewRemainingTokenColumns < RemainingTokenColumns);
      if (!DryRun)
        Token->insertBreak(LineIndex, TailOffset, Split, Whitespaces);
      Penalty += Current.is(tok::string_literal) ? Style.PenaltyBreakString
                                                 : Style.PenaltyBreakComment;
      unsigned ColumnsUsed =
          Token->getLineLengthAfterSplit(LineIndex, TailOffset, Split.first);
      if (ColumnsUsed > getColumnLimit()) {
        Penalty +=
            Style.PenaltyExcessCharacter * (ColumnsUsed - getColumnLimit());
      }
      TailOffset += Split.first + Split.second;
      RemainingTokenColumns = NewRemainingTokenColumns;
      BreakInserted = true;
    }
  }

  State.Column = RemainingTokenColumns;

  if (BreakInserted) {
    // If we break the token inside a parameter list, we need to break before
    // the next parameter on all levels, so that the next parameter is clearly
    // visible. Line comments already introduce a break.
    if (Current.Type != TT_LineComment) {
      for (unsigned i = 0, e = State.Stack.size(); i != e; ++i)
        State.Stack[i].BreakBeforeParameter = true;
    }

    State.Stack.back().LastSpace = StartColumn;
  }
  return Penalty;
}

unsigned ContinuationIndenter::getColumnLimit() const {
  // In preprocessor directives reserve two chars for trailing " \"
  return Style.ColumnLimit - (Line.InPPDirective ? 2 : 0);
}

} // namespace format
} // namespace clang
