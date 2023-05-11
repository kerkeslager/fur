#include <stdbool.h>

#include "assert.h"
#include "parser.h"

Node* parseAtom(Tokenizer* tokenizer) {
  Token token = Tokenizer_scan(tokenizer);

  switch(token.type) {
    case TOKEN_INTEGER_LITERAL:
      return AtomNode_new(NODE_INTEGER_LITERAL, token.line, token.lexeme, token.length);

    default:
      // TODO Handle this
      assert(false);
  }
}

Node* parseExpression(Tokenizer* tokenizer) {
  return parseAtom(tokenizer);
}

#ifdef TEST

void test_parseExpression_parseIntegerLiteral() {
  const char* source = "42";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Node* expression = parseExpression(&tokenizer);

  assert(expression->type == NODE_INTEGER_LITERAL);
  assert(expression->line == 1);

  AtomNode* ilExpression = (AtomNode*)expression;
  assert(ilExpression->text == source);
  assert(ilExpression->length == 2);
}

#endif
