#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "tokenizer.h"

typedef struct {
  Tokenizer tokenizer;
  bool repl;
  bool panic;
} Parser;

void Parser_init(Parser*, const char* source, bool repl);
void Parser_free(Parser*);

void Parser_appendLine(Parser*, const char*);

void Parser_clearPanic(Parser*);

Node* Parser_parseExpression(Parser*);
Node* Parser_parseStatement(Parser*);

#ifdef TEST

void test_Parser_parseAtom_parseIntegerLiteral();
/*
 * TODO
 * These tests commented out until we can mock out printError.
 */
//void test_Parser_parseAtom_errorOnUnexpectedToken();
//void test_Parser_parseAtom_errorOnUnexpectedEof();
//void test_Parser_parseAtom_errorOnUnexpectedTokenDoesNotConsume();
void test_Parser_parseAtom_parsesNil();
void test_Parser_parseAtom_parsesTrue();
void test_Parser_parseAtom_parsesFalse();

/*
 * TODO
 * These tests commented out until we can mock out printError.
 */
//void test_Parser_parsePrefix_parenOpenedButNotClosed();
void test_Parser_parsePrefix_notAfterComparison();

void test_Parser_parseExpression_parseIntegerLiteral();

void test_Parser_parseExpression_infixOperatorsBasic();
void test_Parser_parseExpression_infixOperatorsLeftAssociative();
void test_Parser_parseExpression_infixOrderOfOperations();

void test_Parser_parseExpression_negation();
void test_Parser_parseExpression_nestedNegation();
void test_Parser_parseExpression_negationLeft();
void test_Parser_parseExpression_negationRight();

void test_Parser_parseExpression_parens();

/*
 * TODO
 * These tests commented out until we can mock out printError.
 */
//void test_Parser_parseExpression_infixLeftError();
//void test_Parser_parseExpression_infixRightError();

void test_Parser_parseExpression_assignment();
void test_Parser_parseExpression_mutableAssignment();

void test_Parser_parseStatement_parsesJumpStatementsWithoutElse();

/*
 * TODO
 * These tests commented out until we can mock out printError.
 */
//void test_Parser_parseStatement_requiresSemicolonsForJumpStatementsOutsideREPL();

void test_Parser_parseStatement_parsesJumpStatementsWithoutElseOrSemicolonInREPL();
void test_Parser_parseStatement_parsesJumpElse();
void test_Parser_parseStatement_parsesJumpInAssignment();
void test_Parser_parseStatement_parsesJumpElseInAssignment();
void test_Parser_parseStatement_parsesJumpInParens();
void test_Parser_parseStatement_parsesJumpElseInParens();

void test_Parser_parseStatement_continue();
void test_Parser_parseStatement_continueTo();

void test_Parser_parseStatement_break();
void test_Parser_parseStatement_breakTo();
void test_Parser_parseStatement_breakWith();
void test_Parser_parseStatement_breakToWith();

void test_Parser_parseStatement_terminatesAtSemicolon();
void test_Parser_parseStatement_elideSemicolonAtEndInReplMode();
/*
 * TODO
 * These tests commented out until we can mock out printError.
 */
//void test_Parser_parseStatement_noElideSemicolonAtEndInModuleMode();
//void test_Parser_parseStatement_noMissingSemicolon();
void test_Parser_parseStatement_eof();

#endif

#endif
