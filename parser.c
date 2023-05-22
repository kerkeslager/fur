#include <stdbool.h>

#include <assert.h>
#include <stdio.h>

#include "parser.h"

void Parser_init(Parser* self, const char* source) {
  Tokenizer_init(&(self->tokenizer), source);
  self->isPanic = false;
  self->hasErrors = false;
}

void Parser_free(Parser* self) {
  assert(self != NULL);
}

typedef enum {
  PREC_NONE,
  PREC_ANY,
  PREC_STATEMENT,
  PREC_TERM_RIGHT,
  PREC_TERM_LEFT,
  PREC_FACTOR_RIGHT,
  PREC_FACTOR_LEFT,
  PREC_NEGATE,
} Precedence;

typedef struct {
  Precedence prefix;
  Precedence infixLeft;
  Precedence infixRight;
  bool opensOutfix;
  TokenType closesOutfix;
} PrecedenceRule;

const PrecedenceRule PRECEDENCE[] = {
  [TOKEN_INTEGER_LITERAL] = { PREC_NONE,    PREC_NONE,        PREC_NONE,          false,  NO_TOKEN },

  [TOKEN_PLUS] =            { PREC_NONE,    PREC_TERM_LEFT,   PREC_TERM_RIGHT,    false,  NO_TOKEN },
  [TOKEN_MINUS] =           { PREC_NEGATE,  PREC_TERM_LEFT,   PREC_TERM_RIGHT,    false,  NO_TOKEN },
  [TOKEN_ASTERISK] =        { PREC_NONE,    PREC_FACTOR_LEFT, PREC_FACTOR_RIGHT,  false,  NO_TOKEN },
  [TOKEN_SLASH_SLASH] =     { PREC_NONE,    PREC_FACTOR_LEFT, PREC_FACTOR_RIGHT,  false,  NO_TOKEN },

  [TOKEN_SEMICOLON] =       { PREC_NONE,    PREC_NONE,        PREC_STATEMENT,     false,  NO_TOKEN },

  [TOKEN_OPEN_PAREN] =      { PREC_NONE,    PREC_NONE,        PREC_NONE,          true,   NO_TOKEN },
  [TOKEN_CLOSE_PAREN] =     { PREC_NONE,    PREC_NONE,        PREC_NONE,          false,  TOKEN_OPEN_PAREN },

  [TOKEN_EOF] =             { PREC_NONE,    PREC_NONE,        PREC_NONE,          false,  NO_TOKEN },
  [TOKEN_ERROR] =           { PREC_NONE,    PREC_NONE,        PREC_NONE,          false,  NO_TOKEN },

  [NO_TOKEN] =              { PREC_NONE,    PREC_NONE,        PREC_NONE,          false,  NO_TOKEN },
};

inline static Precedence Token_prefixPrecedence(Token self) {
  assert(self.type != NO_TOKEN);
  return PRECEDENCE[self.type].prefix;
}

inline static Precedence Token_infixLeftPrecedence(Token self) {
  assert(self.type != NO_TOKEN);
  return PRECEDENCE[self.type].infixLeft;
}
inline static Precedence Token_infixRightPrecedence(Token self) {
  assert(self.type != NO_TOKEN);
  return PRECEDENCE[self.type].infixRight;
}

inline static bool Token_opensOutfix(Token self) {
  assert(self.type != NO_TOKEN);
  return PRECEDENCE[self.type].opensOutfix;
}

inline static TokenType Token_closesOutfix(Token self) {
  return PRECEDENCE[self.type].closesOutfix;
}

inline static NodeType mapInfix(Token token) {
  assert(token.type != NO_TOKEN);

  switch(token.type) {
    case TOKEN_PLUS: return NODE_ADD;
    case TOKEN_MINUS: return NODE_SUBTRACT;
    case TOKEN_ASTERISK: return NODE_MULTIPLY;
    case TOKEN_SLASH_SLASH: return NODE_INTEGER_DIVIDE;
    default: assert(false);
  }

  return NODE_ERROR;
}

inline static NodeType mapPrefix(Token token) {
  switch(token.type) {
    case TOKEN_MINUS: return NODE_NEGATE;
    default: assert(false);
  }
  return NODE_ERROR;
}

Node* Parser_parseAtom(Parser* self) {
  Token token = Tokenizer_scan(&(self->tokenizer));

  assert(token.type != TOKEN_ERROR);
  assert(token.type != TOKEN_EOF);
  assert(!Token_opensOutfix(token));
  assert(Token_prefixPrecedence(token) == PREC_NONE);
  assert(Token_infixLeftPrecedence(token) == PREC_NONE);
  assert(Token_infixRightPrecedence(token) == PREC_NONE);

  switch(token.type) {
    case TOKEN_INTEGER_LITERAL:
      return AtomNode_new(NODE_INTEGER_LITERAL, token.line, token.lexeme, token.length);

    default:
      // TODO Handle this
      assert(false);
  }
}

Node* Parser_parseExpressionWithPrecedence(Parser*, Precedence minPrecedence);

/*
 * TODO
 * The commented parameter will be needed if we add any postfix operators.
 */
Node* Parser_parseUnary(Parser* self/*, Precedence minPrecedence*/) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token token = Tokenizer_peek(tokenizer);
  Precedence prefixPrecedence = Token_prefixPrecedence(token);

  if(prefixPrecedence != PREC_NONE) {
    Tokenizer_scan(tokenizer);
    Node* inner = Parser_parseExpressionWithPrecedence(self, prefixPrecedence);

    // TODO Handle postfix
    return UnaryNode_new(mapPrefix(token), token.line, inner);
  } else if(Token_opensOutfix(token)) {
    Tokenizer_scan(tokenizer);

    // TODO Should we set a minPrecedence for opened "environments"?
    Node* result = Parser_parseExpressionWithPrecedence(self, PREC_ANY);

    Token closeToken = Tokenizer_scan(tokenizer);
    assert(Token_closesOutfix(closeToken) == token.type);

    return result;
  } else {
    // TODO Handle postfix
    return Parser_parseAtom(self);
  }

}

Node* Parser_parseExpressionWithPrecedence(Parser* self, Precedence minPrecedence) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Node* left = Parser_parseUnary(self/*, minPrecedence*/);

  for(;;) {
    Token operator = Tokenizer_peek(tokenizer);

    if(Token_infixRightPrecedence(operator) < minPrecedence) {
      return left;
    }

    /*
     * TODO
     * Semicolons are currently treated as an infix operator with precedence
     * of PREC_STATEMENT, which is just above PREC_ANY, meaning it's the
     * lowest precedence that actually gets parsed. Then we have the check
     * below which exits parsing.
     *
     * This feels like a hack, since semicolons aren't infix operators.
     * I considered using them as an infix operator that joins expressions
     * into an expression list, but that doesn't handle situations like
     * `if(true) { return 42; <no second operand> }`
     *
     * I considered implementing them as postfix operators, but that doesn't
     * work either, because then statements like `foo(); + 1` would parse
     * as one expression.
     *
     * This feels like the least hacky hack so far, but looking for a nicer
     * solution.
     */
    if(operator.type == TOKEN_SEMICOLON) {
      Tokenizer_scan(tokenizer);
      return left;
    }

    Tokenizer_scan(tokenizer);

    Node* right = Parser_parseExpressionWithPrecedence(
        self,
        Token_infixLeftPrecedence(operator));

    left = BinaryNode_new(mapInfix(operator), left->line, left, right);
  }
}

Node* Parser_parseExpression(Parser* self) {
  return Parser_parseExpressionWithPrecedence(self, PREC_ANY);
}

#ifdef TEST

void test_Parser_parseExpression_parseIntegerLiteral() {
  const char* source = "42";
  Parser parser;
  Parser_init(&parser, source);

  Node* expression = Parser_parseExpression(&parser);

  assert(expression->type == NODE_INTEGER_LITERAL);
  assert(expression->line == 1);

  AtomNode* ilExpression = (AtomNode*)expression;
  assert(ilExpression->text == source);
  assert(ilExpression->length == 2);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseExpression_addition() {
  const char* source = "1 + 2";
  Parser parser;
  Parser_init(&parser, source);

  Node* expression = Parser_parseExpression(&parser);
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
  Parser_free(&parser);
}

void test_Parser_parseExpression_subtraction() {
  const char* source = "3 - 2";
  Parser parser;
  Parser_init(&parser, source);

  Node* expression = Parser_parseExpression(&parser);
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
  Parser_free(&parser);
}

void test_Parser_parseExpression_multiplication() {
  const char* source = "2 * 3";
  Parser parser;
  Parser_init(&parser, source);

  Node* expression = Parser_parseExpression(&parser);
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
  Parser_free(&parser);
}

void test_Parser_parseExpression_integerDivision() {
  const char* source = "6 // 2";
  Parser parser;
  Parser_init(&parser, source);

  Node* expression = Parser_parseExpression(&parser);
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
  Parser_free(&parser);
}

void test_Parser_parseExpression_additionLeftAssociative() {
  const char* source = "1 + 2 + 3";
  Parser parser;
  Parser_init(&parser, source);

  BinaryNode* expression = (BinaryNode*)Parser_parseExpression(&parser);
  assert(expression->node.type == NODE_ADD);
  assert(expression->arg0->type == NODE_ADD);
  assert(expression->arg1->type == NODE_INTEGER_LITERAL);

  Node_free((Node*)expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_subtractionLeftAssociative() {
  const char* source = "3 - 2 - 1";
  Parser parser;
  Parser_init(&parser, source);

  BinaryNode* expression = (BinaryNode*)Parser_parseExpression(&parser);
  assert(expression->node.type == NODE_SUBTRACT);
  assert(expression->arg0->type == NODE_SUBTRACT);
  assert(expression->arg1->type == NODE_INTEGER_LITERAL);

  Node_free((Node*)expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_multiplicationLeftAssociative() {
  const char* source = "2 * 3 * 5";
  Parser parser;
  Parser_init(&parser, source);

  BinaryNode* expression = (BinaryNode*)Parser_parseExpression(&parser);
  assert(expression->node.type == NODE_MULTIPLY);
  assert(expression->arg0->type == NODE_MULTIPLY);
  assert(expression->arg1->type == NODE_INTEGER_LITERAL);

  Node_free((Node*)expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_integerDivisionLeftAssociative() {
  const char* source = "12 // 3 // 2";
  Parser parser;
  Parser_init(&parser, source);

  BinaryNode* expression = (BinaryNode*)Parser_parseExpression(&parser);
  assert(expression->node.type == NODE_INTEGER_DIVIDE);
  assert(expression->arg0->type == NODE_INTEGER_DIVIDE);
  assert(expression->arg1->type == NODE_INTEGER_LITERAL);

  Node_free((Node*)expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_multiplicationBeforeAddition() {
  const char* source = "1 + 3 * 2";
  Parser parser;
  Parser_init(&parser, source);

  BinaryNode* expression = (BinaryNode*)Parser_parseExpression(&parser);
  assert(expression->node.type == NODE_ADD);
  assert(expression->arg0->type == NODE_INTEGER_LITERAL);
  assert(expression->arg1->type == NODE_MULTIPLY);

  Node_free((Node*)expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_multiplicationBeforeSubtraction() {
  const char* source = "1 - 3 * 2";
  Parser parser;
  Parser_init(&parser, source);

  BinaryNode* expression = (BinaryNode*)Parser_parseExpression(&parser);
  assert(expression->node.type == NODE_SUBTRACT);
  assert(expression->arg0->type == NODE_INTEGER_LITERAL);
  assert(expression->arg1->type == NODE_MULTIPLY);

  Node_free((Node*)expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_integerDivisionBeforeAddition() {
  const char* source = "1 + 6 // 2";
  Parser parser;
  Parser_init(&parser, source);

  BinaryNode* expression = (BinaryNode*)Parser_parseExpression(&parser);
  assert(expression->node.type == NODE_ADD);
  assert(expression->arg0->type == NODE_INTEGER_LITERAL);
  assert(expression->arg1->type == NODE_INTEGER_DIVIDE);

  Node_free((Node*)expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_integerDivisionBeforeSubtraction() {
  const char* source = "1 - 6 // 2";
  Parser parser;
  Parser_init(&parser, source);

  BinaryNode* expression = (BinaryNode*)Parser_parseExpression(&parser);
  assert(expression->node.type == NODE_SUBTRACT);
  assert(expression->arg0->type == NODE_INTEGER_LITERAL);
  assert(expression->arg1->type == NODE_INTEGER_DIVIDE);

  Node_free((Node*)expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_negation() {
  const char* source = "-42";
  Parser parser;
  Parser_init(&parser, source);

  Node* expression = Parser_parseExpression(&parser);
  assert(expression->type == NODE_NEGATE);
  assert(expression->line == 1);

  UnaryNode* uNode = (UnaryNode*)expression;
  assert(uNode->arg0->type == NODE_INTEGER_LITERAL);
  assert(uNode->arg0->line == 1);

  AtomNode* arg0 = (AtomNode*)(uNode->arg0);
  assert(arg0->text == source + 1);
  assert(arg0->length == 2);

  Node_free(expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_nestedNegation() {
  const char* source = "--42";
  Parser parser;
  Parser_init(&parser, source);

  Node* expression = Parser_parseExpression(&parser);
  assert(expression->type == NODE_NEGATE);
  assert(expression->line == 1);

  UnaryNode* uNode = (UnaryNode*)expression;
  assert(uNode->arg0->type == NODE_NEGATE);
  assert(uNode->arg0->line == 1);

  uNode = (UnaryNode*)(uNode->arg0);
  assert(uNode->arg0->type == NODE_INTEGER_LITERAL);
  assert(uNode->arg0->line == 1);

  AtomNode* arg0 = (AtomNode*)(uNode->arg0);
  assert(arg0->text == source + 2);
  assert(arg0->length == 2);

  Node_free(expression);
  Parser_free(&parser);
}

void test_Parser_parseExpression_negationLeft() {
  const char* source;
  Parser parser;
  Node* node;

  source = "-42 + 1";
  Parser_init(&parser, source);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg0->type == NODE_NEGATE);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
  Node_free(node);
  Parser_free(&parser);

  source = "-42 - 1";
  Parser_init(&parser, source);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_SUBTRACT);
  assert(((BinaryNode*)node)->arg0->type == NODE_NEGATE);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
  Node_free(node);
  Parser_free(&parser);

  source = "-42 * 1";
  Parser_init(&parser, source);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_MULTIPLY);
  assert(((BinaryNode*)node)->arg0->type == NODE_NEGATE);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
  Node_free(node);
  Parser_free(&parser);

  source = "-42 // 1";
  Parser_init(&parser, source);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_INTEGER_DIVIDE);
  assert(((BinaryNode*)node)->arg0->type == NODE_NEGATE);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseExpression_negationRight() {
  const char* source;
  Parser parser;
  Node* node;

  source = "42 + -1";
  Parser_init(&parser, source);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_NEGATE);
  Node_free(node);
  Parser_free(&parser);

  source = "42 - -1";
  Parser_init(&parser, source);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_SUBTRACT);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_NEGATE);
  Node_free(node);
  Parser_free(&parser);

  source = "42 * -1";
  Parser_init(&parser, source);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_MULTIPLY);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_NEGATE);
  Node_free(node);
  Parser_free(&parser);

  source = "42 // -1";
  Parser_init(&parser, source);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_INTEGER_DIVIDE);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_NEGATE);
  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseExpression_simpleParens() {
  const char* source = "(42)";
  Parser parser;
  Parser_init(&parser, source);
  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_INTEGER_LITERAL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseExpression_parensOverAssociation() {
  const char* source = "1 + (42 + 1)";
  Parser parser;
  Parser_init(&parser, source);

  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_ADD);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseExpression_parensOverOrderOfOperations() {
  const char* source = "(1 + 1) * 3";
  Parser parser;
  Parser_init(&parser, source);

  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_MULTIPLY);
  assert(((BinaryNode*)node)->arg0->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseExpression_terminatesAtSemicolon() {
  const char* source = "1 + 1; // 42";

  Parser parser;
  Parser_init(&parser, source);

  Node* node = Parser_parseExpression(&parser);
  Token nextToken = Tokenizer_peek(&(parser.tokenizer));

  assert(node->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
  assert(nextToken.type == TOKEN_SLASH_SLASH);

  Node_free(node);
  Parser_free(&parser);
}

#endif
