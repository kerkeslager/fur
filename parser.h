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

Node* parseExpression(Parser*);

#ifdef TEST

void test_parseExpression_parseIntegerLiteral();

void test_parseExpression_addition();
void test_parseExpression_subtraction();
void test_parseExpression_multiplication();
void test_parseExpression_integerDivision();

void test_parseExpression_additionLeftAssociative();
void test_parseExpression_subtractionLeftAssociative();
void test_parseExpression_multiplicationLeftAssociative();
void test_parseExpression_integerDivisionLeftAssociative();

void test_parseExpression_multiplicationBeforeAddition();
void test_parseExpression_multiplicationBeforeSubtraction();
void test_parseExpression_integerDivisionBeforeAddition();
void test_parseExpression_integerDivisionBeforeSubtraction();

void test_parseExpression_negation();
void test_parseExpression_nestedNegation();
void test_parseExpression_negationLeft();
void test_parseExpression_negationRight();

#endif

#endif
