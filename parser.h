#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "tokenizer.h"

typedef struct {
  Tokenizer tokenizer;
  bool repl;
} Parser;

void Parser_init(Parser*, const char* source, bool repl);
void Parser_free(Parser*);

Node* Parser_parseExpression(Parser*);
Node* Parser_parseStatement(Parser*);

#ifdef TEST

void test_Parser_parseAtom_parseIntegerLiteral();
void test_Parser_parseAtom_errorOnUnexpectedToken();
void test_Parser_parseAtom_errorOnUnexpectedEof();
void test_Parser_parseAtom_errorOnUnexpectedTokenDoesNotConsume();
void test_Parser_parseAtom_parsesTrue();
void test_Parser_parseAtom_parsesFalse();

void test_Parser_parseUnary_parenOpenedButNotClosed();
void test_Parser_parseUnary_passesOnErrors();
void test_Parser_parseUnary_notAfterComparison();

void test_Parser_parseExpression_parseIntegerLiteral();

void test_Parser_parseExpression_infixOperatorsBasic();
void test_Parser_parseExpression_infixOperatorsLeftAssociative();
void test_Parser_parseExpression_infixOrderOfOperations();

void test_Parser_parseExpression_negation();
void test_Parser_parseExpression_nestedNegation();
void test_Parser_parseExpression_negationLeft();
void test_Parser_parseExpression_negationRight();

void test_Parser_parseExpression_simpleParens();
void test_Parser_parseExpression_parensOverAssociation();
void test_Parser_parseExpression_parensOverOrderOfOperations();

void test_Parser_parseExpression_infixLeftError();
void test_Parser_parseExpression_infixRightError();

void test_Parser_parseStatement_terminatesAtSemicolon();
void test_Parser_parseStatement_elideSemicolonAtEndInReplMode();
void test_Parser_parseStatement_noElideSemicolonAtEndInModuleMode();
void test_Parser_parseStatement_noMissingSemicolon();
void test_Parser_parseStatement_eof();

#endif

#endif
