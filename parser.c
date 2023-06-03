#include <assert.h>
#include <stdbool.h>

#include <stdio.h>

#include "parser.h"

void Parser_init(Parser* self, const char* source, bool repl) {
  Tokenizer_init(&(self->tokenizer), source);
  self->repl = repl;
}

void Parser_free(Parser* self) {
  assert(self != NULL);
}

typedef enum {
  PREC_NONE,
  PREC_ANY,
  PREC_ASSIGNMENT_LEFT,
  PREC_ASSIGNMENT_RIGHT,
  PREC_LOGICAL_NOT,
  PREC_COMPARISON_RIGHT,
  PREC_COMPARISON_LEFT,
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
  [TOKEN_INTEGER_LITERAL] =     { PREC_NONE,        PREC_NONE,            PREC_NONE,              false,  NO_TOKEN },

  [TOKEN_EQUALS] =              { PREC_NONE,        PREC_ASSIGNMENT_LEFT, PREC_ASSIGNMENT_RIGHT,  false,  NO_TOKEN },
  [TOKEN_SEMICOLON] =           { PREC_NONE,        PREC_NONE,            PREC_NONE,              false,  NO_TOKEN },

  [TOKEN_PLUS] =                { PREC_NONE,        PREC_TERM_LEFT,       PREC_TERM_RIGHT,        false,  NO_TOKEN },
  [TOKEN_MINUS] =               { PREC_NEGATE,      PREC_TERM_LEFT,       PREC_TERM_RIGHT,        false,  NO_TOKEN },
  [TOKEN_ASTERISK] =            { PREC_NONE,        PREC_FACTOR_LEFT,     PREC_FACTOR_RIGHT,      false,  NO_TOKEN },
  [TOKEN_SLASH_SLASH] =         { PREC_NONE,        PREC_FACTOR_LEFT,     PREC_FACTOR_RIGHT,      false,  NO_TOKEN },

  [TOKEN_LESS_THAN] =           { PREC_NONE,        PREC_COMPARISON_LEFT, PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_LESS_THAN_EQUALS] =    { PREC_NONE,        PREC_COMPARISON_LEFT, PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_GREATER_THAN] =        { PREC_NONE,        PREC_COMPARISON_LEFT, PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_GREATER_THAN_EQUALS] = { PREC_NONE,        PREC_COMPARISON_LEFT, PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_EQUALS_EQUALS] =       { PREC_NONE,        PREC_COMPARISON_LEFT, PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_BANG_EQUALS] =         { PREC_NONE,        PREC_COMPARISON_LEFT, PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },

  [TOKEN_OPEN_PAREN] =          { PREC_NONE,        PREC_NONE,            PREC_NONE,              true,   NO_TOKEN },
  [TOKEN_CLOSE_PAREN] =         { PREC_NONE,        PREC_NONE,            PREC_NONE,              false,  TOKEN_OPEN_PAREN },

  [TOKEN_TRUE] =                { PREC_NONE,        PREC_NONE,            PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_FALSE] =               { PREC_NONE,        PREC_NONE,            PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_NOT] =                 { PREC_LOGICAL_NOT, PREC_NONE,            PREC_NONE,              false,  NO_TOKEN },

  [TOKEN_SYMBOL] =              { PREC_NONE,        PREC_NONE,            PREC_NONE,              false,  NO_TOKEN },

  [TOKEN_EOF] =                 { PREC_NONE,        PREC_NONE,            PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_ERROR] =               { PREC_NONE,        PREC_NONE,            PREC_NONE,              false,  NO_TOKEN },

  [NO_TOKEN] =                  { PREC_NONE,        PREC_NONE,            PREC_NONE,              false,  NO_TOKEN },
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
    case TOKEN_EQUALS: return NODE_ASSIGN;
    case TOKEN_PLUS: return NODE_ADD;
    case TOKEN_MINUS: return NODE_SUBTRACT;
    case TOKEN_ASTERISK: return NODE_MULTIPLY;
    case TOKEN_SLASH_SLASH: return NODE_INTEGER_DIVIDE;
    case TOKEN_LESS_THAN: return NODE_LESS_THAN;
    case TOKEN_LESS_THAN_EQUALS: return NODE_LESS_THAN_EQUAL;
    case TOKEN_GREATER_THAN: return NODE_GREATER_THAN;
    case TOKEN_GREATER_THAN_EQUALS: return NODE_GREATER_THAN_EQUAL;
    case TOKEN_EQUALS_EQUALS: return NODE_EQUAL;
    case TOKEN_BANG_EQUALS: return NODE_NOT_EQUAL;
    default: assert(false);
  }

  return NODE_ERROR;
}

inline static NodeType mapPrefix(Token token) {
  switch(token.type) {
    case TOKEN_MINUS:
      return NODE_NEGATE;
    case TOKEN_NOT:
      return NODE_LOGICAL_NOT;

    default: assert(false);
  }
  return NODE_ERROR;
}

Node* Parser_parseAtom(Parser* self) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token token = Tokenizer_peek(tokenizer);

  switch(token.type) {
    case TOKEN_INTEGER_LITERAL:
      Tokenizer_scan(tokenizer);
      return AtomNode_new(NODE_INTEGER_LITERAL, token.line, token.lexeme, token.length);

    case TOKEN_TRUE:
    case TOKEN_FALSE:
      Tokenizer_scan(tokenizer);
      return AtomNode_new(NODE_BOOLEAN_LITERAL, token.line, token.lexeme, token.length);

    case TOKEN_SYMBOL:
      Tokenizer_scan(tokenizer);
      return AtomNode_new(NODE_SYMBOL, token.line, token.lexeme, token.length);

    default:
      // TODO More specific error
      return ErrorNode_new(ERROR_UNEXPECTED_TOKEN, token);
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

    if(inner->type == NODE_ERROR) {
      return inner;
    }

    // TODO Handle postfix
    return UnaryNode_new(mapPrefix(token), token.line, inner);
  } else if(Token_opensOutfix(token)) {
    Tokenizer_scan(tokenizer);

    // TODO Should we set a minPrecedence for opened "environments"?
    Node* result = Parser_parseExpressionWithPrecedence(self, PREC_ANY);

    Token closeToken = Tokenizer_peek(tokenizer);

    if(Token_closesOutfix(closeToken) == token.type) {
      Tokenizer_scan(tokenizer);
      return result;
    } else  {
      return ErrorNode_newWithAuxAndPrevious(
          ERROR_PAREN_OPENED_BUT_NOT_CLOSED,
          closeToken,
          token,
          result
      );
    }

  } else {
    // TODO Handle postfix
    return Parser_parseAtom(self);
  }

}

void Parser_panic(Parser* self) {
  Tokenizer* tokenizer = &(self->tokenizer);

  for(;;) {
    switch(Tokenizer_peek(tokenizer).type) {
      case TOKEN_SEMICOLON:
      case TOKEN_CLOSE_PAREN:
      case TOKEN_EOF:
        return;

      default:
        break;
    }

    Tokenizer_scan(tokenizer);
  }
}

Node* Parser_parseExpressionWithPrecedence(Parser* self, Precedence minPrecedence) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Node* left = Parser_parseUnary(self/*, minPrecedence*/);

  if(left->type == NODE_ERROR) {
    Parser_panic(self);
    return left;
  }

  for(;;) {
    Token operator = Tokenizer_peek(tokenizer);

    if(Token_infixRightPrecedence(operator) < minPrecedence) {
      return left;
    }

    Tokenizer_scan(tokenizer);

    Node* right = Parser_parseExpressionWithPrecedence(
        self,
        Token_infixLeftPrecedence(operator));

    if(right->type == NODE_ERROR) {
      Parser_panic(self);
      Node_free(left);
      return right;
    }

    left = BinaryNode_new(mapInfix(operator), left->line, left, right);
  }
}

Node* Parser_parseExpression(Parser* self) {
  return Parser_parseExpressionWithPrecedence(self, PREC_ANY);
}

Node* Parser_parseStatement(Parser* self) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token token;

  token = Tokenizer_peek(tokenizer);
  if(token.type == TOKEN_EOF) {
    return Node_new(NODE_EOF, token.line);
  }

  Node* expression = Parser_parseExpression(self);

  token = Tokenizer_peek(tokenizer);
  switch(token.type) {
    case TOKEN_SEMICOLON:
      Tokenizer_scan(tokenizer);
      return expression;

    case TOKEN_EOF:
      /*
       * We want to support eliding semicolons in the REPL, but not in files.
       * This is to avoid ambiguity in situations like `{ 42 }` (is that a
       * block that returns 42, or is it a set containing 42? Technically we
       * could elide the last semicolon in a file without ambiguity, because
       * the file is not a block, but that feels inconsistent.
       */
      if(self->repl) {
        return expression;
      } else {
        Node_free(expression);
        return ErrorNode_new(ERROR_MISSING_SEMICOLON, token);
      }

    default:
      // TODO Support eliding semicolons after blocks
      Node_free(expression);
      return ErrorNode_new(ERROR_MISSING_SEMICOLON, token);
  }
}

#ifdef TEST

void test_Parser_parseAtom_parseIntegerLiteral() {
  const char* source = "42";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseAtom(&parser);

  assert(expression->type == NODE_INTEGER_LITERAL);
  assert(expression->line == 1);

  AtomNode* ilExpression = (AtomNode*)expression;
  assert(ilExpression->text == source);
  assert(ilExpression->length == 2);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseAtom_errorOnUnexpectedToken() {
  const char* source = ")";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseAtom(&parser);

  assert(expression->type == NODE_ERROR);
  assert(expression->line == 1);

  ErrorNode* eNode = (ErrorNode*)expression;
  assert(eNode->type == ERROR_UNEXPECTED_TOKEN);
  assert(eNode->token.type == TOKEN_CLOSE_PAREN);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseAtom_errorOnUnexpectedEof() {
  const char* source = " ";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseAtom(&parser);

  assert(expression->type == NODE_ERROR);
  assert(expression->line == 1);

  ErrorNode* eNode = (ErrorNode*)expression;
  assert(eNode->type == ERROR_UNEXPECTED_TOKEN);
  assert(eNode->token.type == TOKEN_EOF);
  assert(eNode->auxToken.type = NO_TOKEN);
  assert(eNode->previous == NULL);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseAtom_errorOnUnexpectedTokenDoesNotConsume() {
  const char* source = ")";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseAtom(&parser);
  Tokenizer* tokenizer = &(parser.tokenizer);
  Token token = Tokenizer_peek(tokenizer);

  assert(token.type == TOKEN_CLOSE_PAREN);
  assert(token.line == 1);
  assert(token.lexeme == source);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseAtom_parsesTrue() {
  const char* source = "true";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseAtom(&parser);

  assert(expression->type == NODE_BOOLEAN_LITERAL);
  assert(expression->line == 1);

  AtomNode* ilExpression = (AtomNode*)expression;
  assert(ilExpression->text == source);
  assert(ilExpression->length == 4);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseAtom_parsesFalse() {
  const char* source = "false";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseAtom(&parser);

  assert(expression->type == NODE_BOOLEAN_LITERAL);
  assert(expression->line == 1);

  AtomNode* ilExpression = (AtomNode*)expression;
  assert(ilExpression->text == source);
  assert(ilExpression->length == 5);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseUnary_parenOpenedButNotClosed() {
  const char* source = "(1 + 2";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseUnary(&parser);

  assert(expression->type == NODE_ERROR);
  assert(expression->line == 1);

  ErrorNode* eNode = (ErrorNode*)expression;
  assert(eNode->type == ERROR_PAREN_OPENED_BUT_NOT_CLOSED);
  assert(eNode->token.type == TOKEN_EOF);
  assert(eNode->auxToken.type == TOKEN_OPEN_PAREN);
  assert(eNode->auxToken.lexeme == source);
  assert(eNode->previous->type == NODE_ADD);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseUnary_passesOnErrors() {
  const char* source = "- - ";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseUnary(&parser);

  assert(expression->type == NODE_ERROR);
  assert(expression->line == 1);

  ErrorNode* eNode = (ErrorNode*)expression;
  assert(eNode->type == ERROR_UNEXPECTED_TOKEN);
  assert(eNode->token.type == TOKEN_EOF);
  assert(eNode->token.lexeme == source + 4);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseUnary_notAfterComparison() {
  typedef struct {
    const char* source;
    NodeType nestedNodeType;
  } TestCase;

  const int TEST_COUNT = 6;
  /*
   * Don't infer the array length, because if we add cases, we want an error
   * if we don't update the test count, so that all test cases are executed
   * by the loop.
   */
  TestCase tests[TEST_COUNT] = {
    { "not 6 < 2", NODE_LESS_THAN },
    { "not 6 <= 2", NODE_LESS_THAN_EQUAL },
    { "not 6 > 2", NODE_GREATER_THAN },
    { "not 6 >= 2", NODE_GREATER_THAN_EQUAL },
    { "not 6 == 2", NODE_EQUAL },
    { "not 6 != 2", NODE_NOT_EQUAL },
  };

  for(int i = 0; i < TEST_COUNT; i++) {
    Parser parser;
    Parser_init(&parser, tests[i].source, false);

    Node* expression = Parser_parseUnary(&parser);

    assert(expression->type == NODE_LOGICAL_NOT);
    assert(expression->line == 1);
    assert(((UnaryNode*)expression)->arg0->type == tests[i].nestedNodeType);

    Parser_free(&parser);
    Node_free(expression);
  }
}

void test_Parser_parseExpression_parseIntegerLiteral() {
  const char* source = "42";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseExpression(&parser);

  assert(expression->type == NODE_INTEGER_LITERAL);
  assert(expression->line == 1);

  AtomNode* ilExpression = (AtomNode*)expression;
  assert(ilExpression->text == source);
  assert(ilExpression->length == 2);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parseExpression_infixOperatorsBasic() {
  Parser parser;
  Node* expression;

  typedef struct {
    const char* source;
    NodeType nodeType;
  } Test;

  const int TEST_COUNT = 10;

  /*
   * Don't infer the array length--we want an error if we add tests and don't
   * update the test count, otherwise the loop won't execute new tests.
   */
  Test tests[TEST_COUNT] = {
    { "6 + 2", NODE_ADD },
    { "6 - 2", NODE_SUBTRACT },
    { "6 * 2", NODE_MULTIPLY },
    { "6 // 2", NODE_INTEGER_DIVIDE },
    { "6 < 2", NODE_LESS_THAN },
    { "6 <= 2", NODE_LESS_THAN_EQUAL },
    { "6 > 2", NODE_GREATER_THAN },
    { "6 >= 2", NODE_GREATER_THAN_EQUAL },
    { "6 == 2", NODE_EQUAL },
    { "6 != 2", NODE_NOT_EQUAL },
  };

  for(int i = 0; i < TEST_COUNT; i++) {
    Parser_init(&parser, tests[i].source, false);
    expression = Parser_parseExpression(&parser);

    assert(expression->type == tests[i].nodeType);
    assert(expression->line == 1);

    BinaryNode* bNode = (BinaryNode*)expression;
    assert(bNode->arg0->type == NODE_INTEGER_LITERAL);
    assert(bNode->arg0->line == 1);
    assert(bNode->arg1->type == NODE_INTEGER_LITERAL);
    assert(bNode->arg1->line == 1);

    AtomNode* arg0 = (AtomNode*)(bNode->arg0);
    assert(arg0->text[0] == '6');
    assert(arg0->length == 1);

    AtomNode* arg1 = (AtomNode*)(bNode->arg1);
    assert(arg1->text[0] == '2');
    assert(arg1->length == 1);

    Node_free(expression);
    Parser_free(&parser);
  }
}

void test_Parser_parseExpression_infixOperatorsLeftAssociative() {
  Parser parser;
  Node* expression;

  typedef struct {
    const char* source;
    NodeType nodeType0;
    NodeType nodeType1;
  } Test;

  const int TEST_COUNT = 44;

  /*
   * Don't infer the array length--we want an error if we add tests and don't
   * update the test count, otherwise the loop won't execute new tests.
   */
  Test tests[TEST_COUNT] = {
    { "6 + 2 + 3", NODE_ADD, NODE_ADD },
    { "6 + 2 - 3", NODE_ADD, NODE_SUBTRACT },
    { "6 - 2 + 3", NODE_SUBTRACT, NODE_ADD },
    { "6 - 2 - 3", NODE_SUBTRACT, NODE_SUBTRACT},
    { "6 * 2 * 3", NODE_MULTIPLY, NODE_MULTIPLY },
    { "6 * 2 // 3", NODE_MULTIPLY, NODE_INTEGER_DIVIDE },
    { "6 // 2 * 3", NODE_INTEGER_DIVIDE, NODE_MULTIPLY },
    { "6 // 2 // 3", NODE_INTEGER_DIVIDE, NODE_INTEGER_DIVIDE },
    { "6 < 2 < 3", NODE_LESS_THAN, NODE_LESS_THAN },
    { "6 < 2 <= 3", NODE_LESS_THAN, NODE_LESS_THAN_EQUAL },
    { "6 < 2 > 3", NODE_LESS_THAN, NODE_GREATER_THAN },
    { "6 < 2 >= 3", NODE_LESS_THAN, NODE_GREATER_THAN_EQUAL },
    { "6 < 2 == 3", NODE_LESS_THAN, NODE_EQUAL },
    { "6 < 2 != 3", NODE_LESS_THAN, NODE_NOT_EQUAL },
    { "6 <= 2 < 3", NODE_LESS_THAN_EQUAL, NODE_LESS_THAN },
    { "6 <= 2 <= 3", NODE_LESS_THAN_EQUAL, NODE_LESS_THAN_EQUAL },
    { "6 <= 2 > 3", NODE_LESS_THAN_EQUAL, NODE_GREATER_THAN },
    { "6 <= 2 >= 3", NODE_LESS_THAN_EQUAL, NODE_GREATER_THAN_EQUAL },
    { "6 <= 2 == 3", NODE_LESS_THAN_EQUAL, NODE_EQUAL },
    { "6 <= 2 != 3", NODE_LESS_THAN_EQUAL, NODE_NOT_EQUAL },
    { "6 > 2 < 3", NODE_GREATER_THAN, NODE_LESS_THAN },
    { "6 > 2 <= 3", NODE_GREATER_THAN, NODE_LESS_THAN_EQUAL },
    { "6 > 2 > 3", NODE_GREATER_THAN, NODE_GREATER_THAN },
    { "6 > 2 >= 3", NODE_GREATER_THAN, NODE_GREATER_THAN_EQUAL },
    { "6 > 2 == 3", NODE_GREATER_THAN, NODE_EQUAL },
    { "6 > 2 != 3", NODE_GREATER_THAN, NODE_NOT_EQUAL },
    { "6 >= 2 < 3", NODE_GREATER_THAN_EQUAL, NODE_LESS_THAN },
    { "6 >= 2 <= 3", NODE_GREATER_THAN_EQUAL, NODE_LESS_THAN_EQUAL },
    { "6 >= 2 > 3", NODE_GREATER_THAN_EQUAL, NODE_GREATER_THAN },
    { "6 >= 2 >= 3", NODE_GREATER_THAN_EQUAL, NODE_GREATER_THAN_EQUAL },
    { "6 >= 2 == 3", NODE_GREATER_THAN_EQUAL, NODE_EQUAL },
    { "6 >= 2 != 3", NODE_GREATER_THAN_EQUAL, NODE_NOT_EQUAL },
    { "6 == 2 < 3", NODE_EQUAL, NODE_LESS_THAN },
    { "6 == 2 <= 3", NODE_EQUAL, NODE_LESS_THAN_EQUAL },
    { "6 == 2 > 3", NODE_EQUAL, NODE_GREATER_THAN },
    { "6 == 2 >= 3", NODE_EQUAL, NODE_GREATER_THAN_EQUAL },
    { "6 == 2 == 3", NODE_EQUAL, NODE_EQUAL },
    { "6 == 2 != 3", NODE_EQUAL, NODE_NOT_EQUAL },
    { "6 != 2 < 3", NODE_NOT_EQUAL, NODE_LESS_THAN },
    { "6 != 2 <= 3", NODE_NOT_EQUAL, NODE_LESS_THAN_EQUAL },
    { "6 != 2 > 3", NODE_NOT_EQUAL, NODE_GREATER_THAN },
    { "6 != 2 >= 3", NODE_NOT_EQUAL, NODE_GREATER_THAN_EQUAL },
    { "6 != 2 == 3", NODE_NOT_EQUAL, NODE_EQUAL },
    { "6 != 2 != 3", NODE_NOT_EQUAL, NODE_NOT_EQUAL },
  };

  for(int i = 0; i < TEST_COUNT; i++) {
    Parser_init(&parser, tests[i].source, false);
    expression = Parser_parseExpression(&parser);

    assert(expression->type == tests[i].nodeType1);
    assert(expression->line == 1);

    BinaryNode* bNode = (BinaryNode*)expression;
    assert(bNode->arg0->type == tests[i].nodeType0);
    assert(bNode->arg0->line == 1);
    assert(bNode->arg1->type == NODE_INTEGER_LITERAL);
    assert(bNode->arg1->line == 1);

    Node_free(expression);
    Parser_free(&parser);
  }
}

void test_Parser_parseExpression_infixOrderOfOperations() {
  Parser parser;
  Node* expression;

  typedef struct {
    const char* source;
    NodeType nodeType0;
    NodeType nodeType1;
  } Test;

  const int TEST_COUNT = 28;

  /*
   * Don't infer the array length--we want an error if we add tests and don't
   * update the test count, otherwise the loop won't execute new tests.
   */
  Test tests[TEST_COUNT] = {
    { "6 + 2 * 3", NODE_ADD, NODE_MULTIPLY },
    { "6 + 2 // 3", NODE_ADD, NODE_INTEGER_DIVIDE },
    { "6 - 2 * 3", NODE_SUBTRACT, NODE_MULTIPLY },
    { "6 - 2 // 3", NODE_SUBTRACT, NODE_INTEGER_DIVIDE },
    { "6 < 2 + 3", NODE_LESS_THAN, NODE_ADD },
    { "6 < 2 - 3", NODE_LESS_THAN, NODE_SUBTRACT },
    { "6 < 2 * 3", NODE_LESS_THAN, NODE_MULTIPLY },
    { "6 < 2 // 3", NODE_LESS_THAN, NODE_INTEGER_DIVIDE },
    { "6 <= 2 + 3", NODE_LESS_THAN_EQUAL, NODE_ADD },
    { "6 <=  2 - 3", NODE_LESS_THAN_EQUAL, NODE_SUBTRACT },
    { "6 <=  2 * 3", NODE_LESS_THAN_EQUAL, NODE_MULTIPLY },
    { "6 <=  2 // 3", NODE_LESS_THAN_EQUAL, NODE_INTEGER_DIVIDE },
    { "6 > 2 + 3", NODE_GREATER_THAN, NODE_ADD },
    { "6 > 2 - 3", NODE_GREATER_THAN, NODE_SUBTRACT },
    { "6 > 2 * 3", NODE_GREATER_THAN, NODE_MULTIPLY },
    { "6 > 2 // 3", NODE_GREATER_THAN, NODE_INTEGER_DIVIDE },
    { "6 >= 2 + 3", NODE_GREATER_THAN_EQUAL, NODE_ADD },
    { "6 >= 2 - 3", NODE_GREATER_THAN_EQUAL, NODE_SUBTRACT },
    { "6 >= 2 * 3", NODE_GREATER_THAN_EQUAL, NODE_MULTIPLY },
    { "6 >= 2 // 3", NODE_GREATER_THAN_EQUAL, NODE_INTEGER_DIVIDE },
    { "6 == 2 + 3", NODE_EQUAL, NODE_ADD },
    { "6 == 2 - 3", NODE_EQUAL, NODE_SUBTRACT },
    { "6 == 2 * 3", NODE_EQUAL, NODE_MULTIPLY },
    { "6 == 2 // 3", NODE_EQUAL, NODE_INTEGER_DIVIDE },
    { "6 != 2 + 3", NODE_NOT_EQUAL, NODE_ADD },
    { "6 != 2 - 3", NODE_NOT_EQUAL, NODE_SUBTRACT },
    { "6 != 2 * 3", NODE_NOT_EQUAL, NODE_MULTIPLY },
    { "6 != 2 // 3", NODE_NOT_EQUAL, NODE_INTEGER_DIVIDE },
  };

  for(int i = 0; i < TEST_COUNT; i++) {
    Parser_init(&parser, tests[i].source, false);
    expression = Parser_parseExpression(&parser);

    assert(expression->type == tests[i].nodeType0);
    assert(expression->line == 1);

    BinaryNode* bNode = (BinaryNode*)expression;
    assert(bNode->arg0->type == NODE_INTEGER_LITERAL);
    assert(bNode->arg0->line == 1);
    assert(bNode->arg1->type == tests[i].nodeType1);
    assert(bNode->arg1->line == 1);

    Node_free(expression);
    Parser_free(&parser);
  }
}

void test_Parser_parseExpression_negation() {
  const char* source = "-42";
  Parser parser;
  Parser_init(&parser, source, false);

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
  Parser_init(&parser, source, false);

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
  Parser_init(&parser, source, false);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg0->type == NODE_NEGATE);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
  Node_free(node);
  Parser_free(&parser);

  source = "-42 - 1";
  Parser_init(&parser, source, false);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_SUBTRACT);
  assert(((BinaryNode*)node)->arg0->type == NODE_NEGATE);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
  Node_free(node);
  Parser_free(&parser);

  source = "-42 * 1";
  Parser_init(&parser, source, false);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_MULTIPLY);
  assert(((BinaryNode*)node)->arg0->type == NODE_NEGATE);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
  Node_free(node);
  Parser_free(&parser);

  source = "-42 // 1";
  Parser_init(&parser, source, false);
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
  Parser_init(&parser, source, false);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_NEGATE);
  Node_free(node);
  Parser_free(&parser);

  source = "42 - -1";
  Parser_init(&parser, source, false);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_SUBTRACT);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_NEGATE);
  Node_free(node);
  Parser_free(&parser);

  source = "42 * -1";
  Parser_init(&parser, source, false);
  node = Parser_parseExpression(&parser);
  assert(node->type == NODE_MULTIPLY);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_NEGATE);
  Node_free(node);
  Parser_free(&parser);

  source = "42 // -1";
  Parser_init(&parser, source, false);
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
  Parser_init(&parser, source, false);
  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_INTEGER_LITERAL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseExpression_parensOverAssociation() {
  const char* source = "1 + (42 + 1)";
  Parser parser;
  Parser_init(&parser, source, false);

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
  Parser_init(&parser, source, false);

  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_MULTIPLY);
  assert(((BinaryNode*)node)->arg0->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseExpression_infixLeftError() {
  const char* source = "(1 + ";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_ERROR);
  assert(node->line == 1);

  ErrorNode* eNode = (ErrorNode*)node;
  assert(eNode->type == ERROR_PAREN_OPENED_BUT_NOT_CLOSED);
  assert(eNode->token.type == TOKEN_EOF);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseExpression_infixRightError() {
  const char* source = "1 + ";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_ERROR);
  assert(node->line == 1);

  ErrorNode* eNode = (ErrorNode*)node;
  assert(eNode->type == ERROR_UNEXPECTED_TOKEN);
  assert(eNode->token.type == TOKEN_EOF);
  assert(eNode->auxToken.type = NO_TOKEN);
  assert(eNode->previous == NULL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseExpression_assignment() {
  const char* source = "foo = 42";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_ASSIGN);
  assert(((BinaryNode*)node)->arg0->type == NODE_SYMBOL);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_terminatesAtSemicolon() {
  const char* source = "1 + 1; // 42";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);
  Token nextToken = Tokenizer_peek(&(parser.tokenizer));

  assert(node->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
  assert(nextToken.type == TOKEN_SLASH_SLASH);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_elideSemicolonAtEndInReplMode() {
  const char* source = "1 + 1";

  Parser parser;
  Parser_init(&parser, source, true);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_ADD);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_noElideSemicolonAtEndInModuleMode() {
  const char* source = "1 + 1";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_ERROR);

  ErrorNode* eNode = (ErrorNode*)node;
  assert(eNode->type == ERROR_MISSING_SEMICOLON);
  assert(eNode->token.type == TOKEN_EOF);

  // TODO Assert more things

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_noMissingSemicolon() {
  /*
   * TODO
   * At some point we probably want 1 + 1() to be syntactically correct, but a
   * type error because 1 is not callable. But at the time of this writing this
   * is the only way to induce a statement end with a token that is not a
   * semicolon. Change this source to something else at some point.
   */
  const char* source = "1 + 1 (";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_ERROR);

  ErrorNode* eNode = (ErrorNode*)node;
  assert(eNode->type == ERROR_MISSING_SEMICOLON);
  assert(eNode->token.type == TOKEN_OPEN_PAREN);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_eof() {
  const char* source = " ";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_EOF);

  Node_free(node);
  Parser_free(&parser);
}

#endif
