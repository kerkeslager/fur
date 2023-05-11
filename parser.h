#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "tokenizer.h"

Node* parseExpression(Tokenizer* tokenizer);

#ifdef TEST

void test_parseExpression_parseIntegerLiteral();

#endif

#endif
