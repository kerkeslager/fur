#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "tokenizer.h"

Node* parseExpression(Tokenizer* tokenizer);

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

#endif

#endif
