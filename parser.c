#include <stdbool.h>

#include <assert.h>
#include "parser.h"
#include <stdio.h>

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

typedef enum {
  PREC_NONE,
  PREC_ANY,
  PREC_TERM_RIGHT,
  PREC_TERM_LEFT,
  PREC_FACTOR_RIGHT,
  PREC_FACTOR_LEFT,
} Precedence;

typedef struct {
  Precedence infixLeft;
  Precedence infixRight;
} PrecedenceRule;

const PrecedenceRule PRECEDENCE[] = {
  [TOKEN_INTEGER_LITERAL] = { PREC_NONE,        PREC_NONE },
  [TOKEN_PLUS] =            { PREC_TERM_LEFT,   PREC_TERM_RIGHT },
  [TOKEN_MINUS] =           { PREC_TERM_LEFT,   PREC_TERM_RIGHT },
  [TOKEN_ASTERISK] =        { PREC_FACTOR_LEFT, PREC_FACTOR_RIGHT },
  [TOKEN_SLASH_SLASH] =     { PREC_FACTOR_LEFT, PREC_FACTOR_RIGHT },

  [TOKEN_EOF] =             { PREC_NONE,        PREC_NONE },
  [TOKEN_ERROR] =           { PREC_NONE,        PREC_NONE },
};

inline static Precedence Token_infixLeftPrecedence(Token self) {
  return PRECEDENCE[self.type].infixLeft;
}
inline static Precedence Token_infixRightPrecedence(Token self) {
  return PRECEDENCE[self.type].infixRight;
}

inline static NodeType mapInfix(Token token) {
  switch(token.type) {
    case TOKEN_PLUS: return NODE_ADD;
    case TOKEN_MINUS: return NODE_SUBTRACT;
    case TOKEN_ASTERISK: return NODE_MULTIPLY;
    case TOKEN_SLASH_SLASH: return NODE_INTEGER_DIVIDE;
    default: assert(false);
  }
  return NODE_ERROR;
}

Node* parseExpressionWithPrecedence(Tokenizer* tokenizer, Precedence minPrecedence) {
  Node* left = parseAtom(tokenizer);

  for(;;) {
    Token operator = Tokenizer_peek(tokenizer);

    if(Token_infixRightPrecedence(operator) < minPrecedence) {
      return left;
    }

    Tokenizer_scan(tokenizer);

    Node* right = parseExpressionWithPrecedence(
        tokenizer,
        Token_infixLeftPrecedence(operator));

    left = BinaryNode_new(mapInfix(operator), left->line, left, right);
  }
}

Node* parseExpression(Tokenizer* tokenizer) {
  return parseExpressionWithPrecedence(tokenizer, PREC_ANY);
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

  Node_free(expression);
}

void test_parseExpression_addition() {
  const char* source = "1 + 2";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Node* expression = parseExpression(&tokenizer);
  assert(expression->type == NODE_ADD);
  assert(expression->line == 1);

  BinaryNode* bNode = (BinaryNode*)expression;
  assert(bNode->arg0->type == NODE_INTEGER_LITERAL);
  assert(bNode->arg0->line == 1);
  assert(bNode->arg1->type == NODE_INTEGER_LITERAL);
  assert(bNode->arg1->line == 1);

  AtomNode* arg0 = (AtomNode*)(bNode->arg0);
  assert(arg0->text == source);
  assert(arg0->length == 1);
  AtomNode* arg1 = (AtomNode*)(bNode->arg1);
  assert(arg1->text == source + 4);
  assert(arg1->length == 1);

  Node_free(expression);
}

void test_parseExpression_subtraction() {
  const char* source = "3 - 2";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Node* expression = parseExpression(&tokenizer);
  assert(expression->type == NODE_SUBTRACT);
  assert(expression->line == 1);

  BinaryNode* bNode = (BinaryNode*)expression;
  assert(bNode->arg0->type == NODE_INTEGER_LITERAL);
  assert(bNode->arg0->line == 1);
  assert(bNode->arg1->type == NODE_INTEGER_LITERAL);
  assert(bNode->arg1->line == 1);

  AtomNode* arg0 = (AtomNode*)(bNode->arg0);
  assert(arg0->text == source);
  assert(arg0->length == 1);
  AtomNode* arg1 = (AtomNode*)(bNode->arg1);
  assert(arg1->text == source + 4);
  assert(arg1->length == 1);

  Node_free(expression);
}

void test_parseExpression_multiplication() {
  const char* source = "2 * 3";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Node* expression = parseExpression(&tokenizer);
  assert(expression->type == NODE_MULTIPLY);
  assert(expression->line == 1);

  BinaryNode* bNode = (BinaryNode*)expression;
  assert(bNode->arg0->type == NODE_INTEGER_LITERAL);
  assert(bNode->arg0->line == 1);
  assert(bNode->arg1->type == NODE_INTEGER_LITERAL);
  assert(bNode->arg1->line == 1);

  AtomNode* arg0 = (AtomNode*)(bNode->arg0);
  assert(arg0->text == source);
  assert(arg0->length == 1);
  AtomNode* arg1 = (AtomNode*)(bNode->arg1);
  assert(arg1->text == source + 4);
  assert(arg1->length == 1);

  Node_free(expression);
}

void test_parseExpression_integerDivision() {
  const char* source = "6 // 2";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Node* expression = parseExpression(&tokenizer);
  assert(expression->type == NODE_INTEGER_DIVIDE);
  assert(expression->line == 1);

  BinaryNode* bNode = (BinaryNode*)expression;
  assert(bNode->arg0->type == NODE_INTEGER_LITERAL);
  assert(bNode->arg0->line == 1);
  assert(bNode->arg1->type == NODE_INTEGER_LITERAL);
  assert(bNode->arg1->line == 1);

  AtomNode* arg0 = (AtomNode*)(bNode->arg0);
  assert(arg0->text == source);
  assert(arg0->length == 1);
  AtomNode* arg1 = (AtomNode*)(bNode->arg1);
  assert(arg1->text == source + 5);
  assert(arg1->length == 1);

  Node_free(expression);
}

void test_parseExpression_additionLeftAssociative() {
  const char* source = "1 + 2 + 3";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  BinaryNode* expression = (BinaryNode*)parseExpression(&tokenizer);
  assert(expression->node.type == NODE_ADD);
  assert(expression->arg0->type == NODE_ADD);
  assert(expression->arg1->type == NODE_INTEGER_LITERAL);

  Node_free((Node*)expression);
}

void test_parseExpression_subtractionLeftAssociative() {
  const char* source = "3 - 2 - 1";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  BinaryNode* expression = (BinaryNode*)parseExpression(&tokenizer);
  assert(expression->node.type == NODE_SUBTRACT);
  assert(expression->arg0->type == NODE_SUBTRACT);
  assert(expression->arg1->type == NODE_INTEGER_LITERAL);

  Node_free((Node*)expression);
}

void test_parseExpression_multiplicationLeftAssociative() {
  const char* source = "2 * 3 * 5";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  BinaryNode* expression = (BinaryNode*)parseExpression(&tokenizer);
  assert(expression->node.type == NODE_MULTIPLY);
  assert(expression->arg0->type == NODE_MULTIPLY);
  assert(expression->arg1->type == NODE_INTEGER_LITERAL);

  Node_free((Node*)expression);
}

void test_parseExpression_integerDivisionLeftAssociative() {
  const char* source = "12 // 3 // 2";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  BinaryNode* expression = (BinaryNode*)parseExpression(&tokenizer);
  assert(expression->node.type == NODE_INTEGER_DIVIDE);
  assert(expression->arg0->type == NODE_INTEGER_DIVIDE);
  assert(expression->arg1->type == NODE_INTEGER_LITERAL);

  Node_free((Node*)expression);
}

void test_parseExpression_multiplicationBeforeAddition() {
  const char* source = "1 + 3 * 2";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  BinaryNode* expression = (BinaryNode*)parseExpression(&tokenizer);
  assert(expression->node.type == NODE_ADD);
  assert(expression->arg0->type == NODE_INTEGER_LITERAL);
  assert(expression->arg1->type == NODE_MULTIPLY);

  Node_free((Node*)expression);
}

void test_parseExpression_multiplicationBeforeSubtraction() {
  const char* source = "1 - 3 * 2";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  BinaryNode* expression = (BinaryNode*)parseExpression(&tokenizer);
  assert(expression->node.type == NODE_SUBTRACT);
  assert(expression->arg0->type == NODE_INTEGER_LITERAL);
  assert(expression->arg1->type == NODE_MULTIPLY);

  Node_free((Node*)expression);
}

void test_parseExpression_integerDivisionBeforeAddition() {
  const char* source = "1 + 6 // 2";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  BinaryNode* expression = (BinaryNode*)parseExpression(&tokenizer);
  assert(expression->node.type == NODE_ADD);
  assert(expression->arg0->type == NODE_INTEGER_LITERAL);
  assert(expression->arg1->type == NODE_INTEGER_DIVIDE);

  Node_free((Node*)expression);
}

void test_parseExpression_integerDivisionBeforeSubtraction() {
  const char* source = "1 - 6 // 2";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  BinaryNode* expression = (BinaryNode*)parseExpression(&tokenizer);
  assert(expression->node.type == NODE_SUBTRACT);
  assert(expression->arg0->type == NODE_INTEGER_LITERAL);
  assert(expression->arg1->type == NODE_INTEGER_DIVIDE);

  Node_free((Node*)expression);
}

#endif
