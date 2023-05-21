#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "tokenizer.h"

typedef struct {
  Tokenizer tokenizer;
  TokenStack openOutfixes;
  bool isPanic;
  bool hasErrors;
} Parser;

void Parser_init(Parser*, const char* source);
void Parser_free(Parser*);

Node* Parser_parseExpression(Parser*);

#ifdef TEST

void test_Parser_parseExpression_parseIntegerLiteral();

void test_Parser_parseExpression_addition();
void test_Parser_parseExpression_subtraction();
void test_Parser_parseExpression_multiplication();
void test_Parser_parseExpression_integerDivision();

void test_Parser_parseExpression_additionLeftAssociative();
void test_Parser_parseExpression_subtractionLeftAssociative();
void test_Parser_parseExpression_multiplicationLeftAssociative();
void test_Parser_parseExpression_integerDivisionLeftAssociative();

void test_Parser_parseExpression_multiplicationBeforeAddition();
void test_Parser_parseExpression_multiplicationBeforeSubtraction();
void test_Parser_parseExpression_integerDivisionBeforeAddition();
void test_Parser_parseExpression_integerDivisionBeforeSubtraction();

void test_Parser_parseExpression_negation();
void test_Parser_parseExpression_nestedNegation();
void test_Parser_parseExpression_negationLeft();
void test_Parser_parseExpression_negationRight();

void test_Parser_parseExpression_simpleParens();
void test_Parser_parseExpression_parensOverOrderOfOperations();

#endif

#endif
