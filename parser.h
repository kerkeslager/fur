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

#endif

#endif
