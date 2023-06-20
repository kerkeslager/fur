#include <assert.h>
#include <stdbool.h>

#include <stdio.h>

#include "parser.h"

void Parser_init(Parser* self, const char* source, bool repl) {
  Tokenizer_init(&(self->tokenizer), source, repl ? 0 : 1);
  self->repl = repl;
}

void Parser_free(Parser* self) {
  assert(self != NULL);
}

void Parser_appendLine(Parser* self, const char* line) {
  // Make sure that all the existing source is consumed before overwriting it
  assert(Tokenizer_peek(&(self->tokenizer)).type == TOKEN_EOF);
  Tokenizer_appendLine(&(self->tokenizer), line);
}

typedef enum {
  PREC_NONE,
  PREC_ANY,
  PREC_MUTABILITY,
  PREC_ASSIGNMENT_LEFT,
  PREC_ASSIGNMENT_RIGHT,
  PREC_LOGICAL_OR_RIGHT,
  PREC_LOGICAL_OR_LEFT,
  PREC_LOGICAL_AND_RIGHT,
  PREC_LOGICAL_AND_LEFT,
  PREC_LOGICAL_NOT,
  PREC_COMPARISON_RIGHT,
  PREC_COMPARISON_LEFT,
  PREC_TERM_RIGHT,
  PREC_TERM_LEFT,
  PREC_FACTOR_RIGHT,
  PREC_FACTOR_LEFT,
  PREC_NEGATE,
  PREC_CALL,
} Precedence;

typedef struct {
  Precedence prefix;
  Precedence postfix;
  Precedence infixLeft;
  Precedence infixRight;
  bool opensOutfix;
  TokenType closesOutfix;
} PrecedenceRule;

const PrecedenceRule PRECEDENCE[] = {
  [TOKEN_INTEGER_LITERAL] =     { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },

  [TOKEN_MUT] =                 { PREC_MUTABILITY,  PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_EQUALS] =              { PREC_NONE,        PREC_NONE,  PREC_ASSIGNMENT_LEFT,   PREC_ASSIGNMENT_RIGHT,  false,  NO_TOKEN },
  [TOKEN_SEMICOLON] =           { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },

  [TOKEN_PLUS] =                { PREC_NONE,        PREC_NONE,  PREC_TERM_LEFT,         PREC_TERM_RIGHT,        false,  NO_TOKEN },
  [TOKEN_MINUS] =               { PREC_NEGATE,      PREC_NONE,  PREC_TERM_LEFT,         PREC_TERM_RIGHT,        false,  NO_TOKEN },
  [TOKEN_ASTERISK] =            { PREC_NONE,        PREC_NONE,  PREC_FACTOR_LEFT,       PREC_FACTOR_RIGHT,      false,  NO_TOKEN },
  [TOKEN_SLASH_SLASH] =         { PREC_NONE,        PREC_NONE,  PREC_FACTOR_LEFT,       PREC_FACTOR_RIGHT,      false,  NO_TOKEN },

  [TOKEN_LESS_THAN] =           { PREC_NONE,        PREC_NONE,  PREC_COMPARISON_LEFT,   PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_LESS_THAN_EQUALS] =    { PREC_NONE,        PREC_NONE,  PREC_COMPARISON_LEFT,   PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_GREATER_THAN] =        { PREC_NONE,        PREC_NONE,  PREC_COMPARISON_LEFT,   PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_GREATER_THAN_EQUALS] = { PREC_NONE,        PREC_NONE,  PREC_COMPARISON_LEFT,   PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_EQUALS_EQUALS] =       { PREC_NONE,        PREC_NONE,  PREC_COMPARISON_LEFT,   PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },
  [TOKEN_BANG_EQUALS] =         { PREC_NONE,        PREC_NONE,  PREC_COMPARISON_LEFT,   PREC_COMPARISON_RIGHT,  false,  NO_TOKEN },

  [TOKEN_AND] =                 { PREC_NONE,        PREC_NONE,  PREC_LOGICAL_AND_LEFT,  PREC_LOGICAL_AND_RIGHT, false,  NO_TOKEN },
  [TOKEN_OR] =                  { PREC_NONE,        PREC_NONE,  PREC_LOGICAL_OR_LEFT,   PREC_LOGICAL_OR_RIGHT,  false,  NO_TOKEN },

  [TOKEN_OPEN_PAREN] =          { PREC_NONE,        PREC_CALL,  PREC_NONE,              PREC_NONE,              true,   NO_TOKEN },
  [TOKEN_CLOSE_PAREN] =         { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  TOKEN_OPEN_PAREN },

  [TOKEN_NIL] =                 { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_TRUE] =                { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_FALSE] =               { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_NOT] =                 { PREC_LOGICAL_NOT, PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },

  [TOKEN_SYMBOL] =              { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },

  [TOKEN_LOOP] =                { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_IF] =                  { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_ELSE] =                { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_WHILE] =               { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_UNTIL] =               { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },

  [TOKEN_EOF] =                 { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
  [TOKEN_ERROR] =               { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },

  [NO_TOKEN] =                  { PREC_NONE,        PREC_NONE,  PREC_NONE,              PREC_NONE,              false,  NO_TOKEN },
};

inline static Precedence Token_prefixPrecedence(Token self) {
  assert(self.type != NO_TOKEN);
  return PRECEDENCE[self.type].prefix;
}

inline static Precedence Token_postfixPrecedence(Token self) {
  assert(self.type != NO_TOKEN);
  return PRECEDENCE[self.type].postfix;
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

inline static TokenType Token_closesOutfix(Token self, Token openToken) {
  return PRECEDENCE[self.type].closesOutfix == openToken.type;
}

inline static NodeType mapInfix(Token token) {
  assert(token.type != NO_TOKEN);

  switch(token.type) {
    case TOKEN_EQUALS:
      return NODE_ASSIGN;
    case TOKEN_PLUS:
      return NODE_ADD;
    case TOKEN_MINUS:
      return NODE_SUBTRACT;
    case TOKEN_ASTERISK:
      return NODE_MULTIPLY;
    case TOKEN_SLASH_SLASH:
      return NODE_INTEGER_DIVIDE;
    case TOKEN_LESS_THAN:
      return NODE_LESS_THAN;
    case TOKEN_LESS_THAN_EQUALS:
      return NODE_LESS_THAN_EQUAL;
    case TOKEN_GREATER_THAN:
      return NODE_GREATER_THAN;
    case TOKEN_GREATER_THAN_EQUALS:
      return NODE_GREATER_THAN_EQUAL;
    case TOKEN_EQUALS_EQUALS:
      return NODE_EQUAL;
    case TOKEN_BANG_EQUALS:
      return NODE_NOT_EQUAL;
    case TOKEN_AND:
      return NODE_AND;
    case TOKEN_OR:
      return NODE_OR;

    default:
      break;
  }

  // Should never happen
  assert(false);
  return NODE_ERROR;
}

inline static NodeType mapOutfix(Token token) {
  switch(token.type) {
    case TOKEN_OPEN_PAREN:
      return NODE_PARENS;

    default:
      break;
  }

  // Should never happen
  assert(false);
  return NODE_ERROR;
}

inline static NodeType mapPrefix(Token token) {
  switch(token.type) {
    case TOKEN_MINUS:
      return NODE_NEGATE;
    case TOKEN_NOT:
      return NODE_LOGICAL_NOT;
    case TOKEN_MUT:
      return NODE_MUT;

    default:
      assert(false);
  }
  return NODE_ERROR;
}

inline static NodeType mapPostfix(Token token) {
  switch(token.type) {
    case TOKEN_OPEN_PAREN:
      return NODE_CALL;

    default:
      break;
  }

  // Should never happen
  assert(false);
  return NODE_ERROR;
}

Node* Parser_parseCondJumpExpr(Parser* self, NodeType nodeType) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token token = Tokenizer_scan(tokenizer);

  // TODO Handle this better
  assert(Tokenizer_scan(tokenizer).type == TOKEN_OPEN_PAREN);

  Node* condition = Parser_parseExpression(self);

  // TODO Handle this better
  assert(Tokenizer_scan(tokenizer).type == TOKEN_CLOSE_PAREN);

  if(condition->type == NODE_ERROR) {
    return condition;
  }

  Node* ifBranch = Parser_parseStatement(self);

  if(ifBranch->type == NODE_ERROR) {
    free(condition);
    return ifBranch;
  }

  Token elseToken = Tokenizer_peek(tokenizer);

  if(elseToken.type != TOKEN_ELSE) {
    return TernaryNode_new(
      nodeType,
      token.line,
      condition,
      ifBranch,
      NULL
    );
  }

  Tokenizer_scan(tokenizer);

  Node* elseBranch = Parser_parseStatement(self);

  if(elseBranch->type == NODE_ERROR) {
    Node_free(condition);
    Node_free(ifBranch);
    return elseBranch;
  }

  return TernaryNode_new(
    nodeType,
    token.line,
    condition,
    ifBranch,
    elseBranch
  );
}

/*
 * TODO
 * This is clearly not parsing atoms any more. Come up with a better name.
 */
Node* Parser_parseAtom(Parser* self) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token token = Tokenizer_peek(tokenizer);

  switch(token.type) {
    case TOKEN_INTEGER_LITERAL:
      Tokenizer_scan(tokenizer);
      return AtomNode_new(NODE_INTEGER_LITERAL, token.line, token.lexeme, token.length);

    case TOKEN_NIL:
      Tokenizer_scan(tokenizer);
      return AtomNode_new(NODE_NIL_LITERAL, token.line, token.lexeme, token.length);

    case TOKEN_TRUE:
    case TOKEN_FALSE:
      Tokenizer_scan(tokenizer);
      return AtomNode_new(NODE_BOOLEAN_LITERAL, token.line, token.lexeme, token.length);

    case TOKEN_SYMBOL:
      Tokenizer_scan(tokenizer);
      return AtomNode_new(NODE_SYMBOL, token.line, token.lexeme, token.length);

    case TOKEN_CLOSE_PAREN:
      return ErrorNode_new(ERROR_UNEXPECTED_TOKEN, token);

    case TOKEN_OPEN_BRACE:
      Tokenizer_scan(tokenizer);
      {
        ListNode* listNode = ListNode_new(NODE_BLOCK, token.line);

        for(;;) {
          token = Tokenizer_peek(tokenizer);

          switch(token.type) {
            case TOKEN_CLOSE_BRACE:
              Tokenizer_scan(tokenizer);
              return ListNode_finish(listNode);

            case TOKEN_EOF:
              // TODO Handle this
              assert(false);
              return NULL;

            default:
              {
                Node* next = Parser_parseStatement(self);

                // TODO Handle this
                assert(next->type != NODE_ERROR);

                ListNode_append(listNode, next);
              }
          }
        }
      }

    case TOKEN_LOOP:
      Tokenizer_scan(tokenizer);
      {
        Node* body = Parser_parseStatement(self);

        if(body->type == NODE_ERROR) {
          return body;
        } else {
          return UnaryNode_new(NODE_LOOP, token.line, body);
        }
      }

    case TOKEN_IF:
      return Parser_parseCondJumpExpr(self, NODE_IF);
    case TOKEN_WHILE:
      return Parser_parseCondJumpExpr(self, NODE_WHILE);
    case TOKEN_UNTIL:
      return Parser_parseCondJumpExpr(self, NODE_UNTIL);

    case TOKEN_CONTINUE:
      {
        Tokenizer_scan(tokenizer);
        Node* continueTo = NULL;

        token = Tokenizer_peek(tokenizer);

        switch(token.type) {
          case TOKEN_INTEGER_LITERAL:
            Tokenizer_scan(tokenizer);
            continueTo = AtomNode_new(NODE_INTEGER_LITERAL, token.line, token.lexeme, token.length);
            break;

          default:
            break;
        }

        return UnaryNode_new(NODE_CONTINUE, token.line, continueTo);
      }

    case TOKEN_BREAK:
      {
        Tokenizer_scan(tokenizer);
        Node* breakTo = NULL;
        Node* breakWith = NULL;

        token = Tokenizer_peek(tokenizer);

        switch(token.type) {
          case TOKEN_INTEGER_LITERAL:
            Tokenizer_scan(tokenizer);
            breakTo = AtomNode_new(NODE_INTEGER_LITERAL, token.line, token.lexeme, token.length);

            token = Tokenizer_peek(tokenizer);
            if(token.type == TOKEN_WITH) {
              Tokenizer_scan(tokenizer);
              breakWith = Parser_parseExpression(self);
            }
            break;

          case TOKEN_WITH:
            Tokenizer_scan(tokenizer);
            breakWith = Parser_parseExpression(self);
            break;

          default:
            break;
        }

        return BinaryNode_new(NODE_BREAK, token.line, breakTo, breakWith);
      }

    default:
      // TODO More specific error
      Tokenizer_scan(tokenizer);
      return ErrorNode_new(ERROR_UNEXPECTED_TOKEN, token);
  }
}

Node* Parser_parseStatement(Parser*);
Node* Parser_parseExpression(Parser*);
Node* Parser_parseExprWithPrec(Parser*, Precedence minPrecedence);

Node* Parser_parseOutfix(Parser* self) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token openToken = Tokenizer_peek(tokenizer);

  if(!Token_opensOutfix(openToken)) return Parser_parseAtom(self);

  Tokenizer_scan(tokenizer);

  // TODO Should we set a minPrecedence for opened "environments"?
  Node* result = Parser_parseExpression(self);

  Token closeToken = Tokenizer_peek(tokenizer);

  if(!Token_closesOutfix(closeToken, openToken)) {
    return ErrorNode_newWithAuxAndPrevious(
      ERROR_PAREN_OPENED_BUT_NOT_CLOSED,
      closeToken,
      openToken,
      result
    );
  }

  Tokenizer_scan(tokenizer);

  return UnaryNode_new(
    mapOutfix(openToken),
    openToken.line,
    result
  );
}

Precedence Precedence_max(Precedence arg0, Precedence arg1) {
  return arg0 > arg1 ? arg0 : arg1;
}

Node* Parser_parsePrefix(Parser* self, Precedence minPrecedence) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token token = Tokenizer_peek(tokenizer);
  Precedence prefixPrecedence = Token_prefixPrecedence(token);

  if(prefixPrecedence == PREC_NONE) return Parser_parseOutfix(self);

  Tokenizer_scan(tokenizer);
  Node* inner = Parser_parseExprWithPrec(
    self,
    Precedence_max(
      prefixPrecedence,
      minPrecedence
    )
  );

  if(inner->type == NODE_ERROR) {
    return inner;
  }

  return UnaryNode_new(mapPrefix(token), token.line, inner);
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

Node* Parser_parseExprWithPrec(Parser* self, Precedence minPrecedence) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Node* result = Parser_parsePrefix(self, minPrecedence);

  if(result->type == NODE_ERROR) {
    Parser_panic(self);
    return result;
  }

  for(;;) {
    Token operator = Tokenizer_peek(tokenizer);

    if(Token_infixRightPrecedence(operator) >= minPrecedence) {
      Tokenizer_scan(tokenizer);

      Node* right = Parser_parseExprWithPrec(
        self,
        Token_infixLeftPrecedence(operator)
      );

      if(right->type == NODE_ERROR) {
        Parser_panic(self);
        Node_free(result);
        return right;
      }

      result = BinaryNode_new(mapInfix(operator), result->line, result, right);

      continue;
    }

    if(Token_postfixPrecedence(operator) >= minPrecedence) {
      if(Token_opensOutfix(operator)) {
        Node* argExpr = Parser_parseExpression(self);

        // TODO Handle this better
        assert(Token_closesOutfix(Tokenizer_scan(tokenizer), operator));

        result = BinaryNode_new(mapPostfix(operator), result->line, result, argExpr);
      } else {
        // Currently all postfix operators are outfix as well
        assert(false);
      }

      continue;
    }

    return result;
  }
}

Node* Parser_parseExpression(Parser* self) {
  return Parser_parseExprWithPrec(self, PREC_ANY);
}

#include "debug.h"

static inline bool Node_requiresSemicolon(Node* self) {
  switch(self->type) {
    case NODE_BLOCK:
      return false;

    /*
     * These don't require a semicolon because they already consume
     * their semicolons by calling parseStatement.
     */
    case NODE_LOOP:
    case NODE_IF:
    case NODE_UNTIL:
    case NODE_WHILE:
      return false;

    case NODE_INTEGER_LITERAL:
    case NODE_NIL_LITERAL:
    case NODE_BOOLEAN_LITERAL:
    case NODE_SYMBOL:
    case NODE_PARENS:
    case NODE_CALL:
      return true;

    case NODE_NEGATE:
    case NODE_LOGICAL_NOT:
    case NODE_MUT:
      return Node_requiresSemicolon(((UnaryNode*)self)->arg0);

    case NODE_ASSIGN:
    case NODE_ADD:
    case NODE_SUBTRACT:
    case NODE_MULTIPLY:
    case NODE_INTEGER_DIVIDE:
    case NODE_LESS_THAN:
    case NODE_LESS_THAN_EQUAL:
    case NODE_GREATER_THAN:
    case NODE_GREATER_THAN_EQUAL:
    case NODE_EQUAL:
    case NODE_NOT_EQUAL:
    case NODE_AND:
    case NODE_OR:
      return Node_requiresSemicolon(((BinaryNode*)self)->arg1);

    /*
     * Requires semicolon should never be called for these.
     */
    case NODE_EOF:
    case NODE_ERROR:
    case NODE_BREAK:
    case NODE_CONTINUE:
      NodeType_println(self->type);
      assert(false);
      break;
  }

  // Should never happen
  assert(false);
  return false;
}

Node* Parser_parseContinueStmt(Parser* self) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token token = Tokenizer_scan(tokenizer);
  Node* continueTo = NULL;

  token = Tokenizer_peek(tokenizer);

  if(token.type == TOKEN_INTEGER_LITERAL) {
    Tokenizer_scan(tokenizer);
    continueTo = AtomNode_new(
      NODE_INTEGER_LITERAL,
      token.line,
      token.lexeme,
      token.length
    );
    token = Tokenizer_peek(tokenizer);
  }

  if(token.type != TOKEN_SEMICOLON) {
    return ErrorNode_new(ERROR_MISSING_SEMICOLON, token);
  }

  Tokenizer_scan(tokenizer);
  return UnaryNode_new(NODE_CONTINUE, token.line, continueTo);
}

Node* Parser_parseBreakStmt(Parser* self) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token token = Tokenizer_scan(tokenizer);
  Node* breakTo = NULL;
  Node* breakWith = NULL;

  token = Tokenizer_peek(tokenizer);

  switch(token.type) {
    case TOKEN_INTEGER_LITERAL:
      Tokenizer_scan(tokenizer);
      breakTo = AtomNode_new(NODE_INTEGER_LITERAL, token.line, token.lexeme, token.length);

      token = Tokenizer_peek(tokenizer);
      if(token.type == TOKEN_WITH) {
        Tokenizer_scan(tokenizer);
        breakWith = Parser_parseExpression(self);
      }
      break;

    case TOKEN_WITH:
      Tokenizer_scan(tokenizer);
      breakWith = Parser_parseExpression(self);
      break;

    default:
      break;
  }

  token = Tokenizer_peek(tokenizer);

  if(token.type != TOKEN_SEMICOLON) {
    return ErrorNode_new(ERROR_MISSING_SEMICOLON, token);
  }

  Tokenizer_scan(tokenizer);
  return BinaryNode_new(NODE_BREAK, token.line, breakTo, breakWith);
}

Node* Parser_parseStatement(Parser* self) {
  Tokenizer* tokenizer = &(self->tokenizer);
  Token token = Tokenizer_peek(tokenizer);

  switch(token.type) {
    case TOKEN_EOF:
      return Node_new(NODE_EOF, token.line);

    case TOKEN_CONTINUE:
      return Parser_parseContinueStmt(self);

    case TOKEN_BREAK:
      return Parser_parseBreakStmt(self);

    default:
      break;
  }

  Node* expression = Parser_parseExpression(self);

  if(expression->type == NODE_ERROR) {
    return expression;
  }

  if(!Node_requiresSemicolon(expression)) {
    return expression;
  }

  token = Tokenizer_peek(tokenizer);
  switch(token.type) {
    case TOKEN_SEMICOLON:
      Tokenizer_scan(tokenizer);
      return expression;

    case TOKEN_CLOSE_PAREN:
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

void test_Parser_parseAtom_parsesNil() {
  const char* source = "nil";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parseAtom(&parser);

  assert(expression->type == NODE_NIL_LITERAL);
  assert(expression->line == 1);

  AtomNode* ilExpression = (AtomNode*)expression;
  assert(ilExpression->text == source);
  assert(ilExpression->length == 3);

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

void test_Parser_parsePrefix_parenOpenedButNotClosed() {
  const char* source = "(1 + 2";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parsePrefix(&parser, PREC_ANY);

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

void test_Parser_parsePrefix_passesOnErrors() {
  const char* source = "- - ";
  Parser parser;
  Parser_init(&parser, source, false);

  Node* expression = Parser_parsePrefix(&parser, PREC_ANY);

  assert(expression->type == NODE_ERROR);
  assert(expression->line == 1);

  ErrorNode* eNode = (ErrorNode*)expression;
  assert(eNode->type == ERROR_UNEXPECTED_TOKEN);
  assert(eNode->token.type == TOKEN_EOF);
  assert(eNode->token.lexeme == source + 4);

  Parser_free(&parser);
  Node_free(expression);
}

void test_Parser_parsePrefix_notAfterComparison() {
  #define TEST_COUNT 6

  typedef struct {
    const char* source;
    NodeType nestedNodeType;
  } TestCase;

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

    Node* expression = Parser_parsePrefix(&parser, PREC_ANY);

    assert(expression->type == NODE_LOGICAL_NOT);
    assert(expression->line == 1);
    assert(((UnaryNode*)expression)->arg0->type == tests[i].nestedNodeType);

    Parser_free(&parser);
    Node_free(expression);
  }
  #undef TEST_COUNT
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
  #define TEST_COUNT 12
  Parser parser;
  Node* expression;

  typedef struct {
    const char* source;
    NodeType nodeType;
  } Test;

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
    { "6 and 2", NODE_AND },
    { "6 or 2", NODE_OR },
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

  #undef TEST_COUNT
}

void test_Parser_parseExpression_infixOperatorsLeftAssociative() {
  #define TEST_COUNT 46

  Parser parser;
  Node* expression;

  typedef struct {
    const char* source;
    NodeType nodeType0;
    NodeType nodeType1;
  } Test;

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
    { "6 and 2 and 3", NODE_AND, NODE_AND },
    { "6 or 2 or 3", NODE_OR, NODE_OR },
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

  #undef TEST_COUNT
}

void test_Parser_parseExpression_infixOrderOfOperations() {
  #define TEST_COUNT 49
  Parser parser;
  Node* expression;

  typedef struct {
    const char* source;
    NodeType nodeType0;
    NodeType nodeType1;
  } Test;

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

    { "6 and 2 + 3", NODE_AND, NODE_ADD },
    { "6 and 2 - 3", NODE_AND, NODE_SUBTRACT },
    { "6 and 2 * 3", NODE_AND, NODE_MULTIPLY },
    { "6 and 2 // 3", NODE_AND, NODE_INTEGER_DIVIDE },
    { "6 and 2 < 3", NODE_AND, NODE_LESS_THAN },
    { "6 and 2 <= 3", NODE_AND, NODE_LESS_THAN_EQUAL },
    { "6 and 2 > 3", NODE_AND, NODE_GREATER_THAN },
    { "6 and 2 >= 3", NODE_AND, NODE_GREATER_THAN_EQUAL },
    { "6 and 2 == 3", NODE_AND, NODE_EQUAL },
    { "6 and 2 != 3", NODE_AND, NODE_NOT_EQUAL },

    { "6 or 2 + 3", NODE_OR, NODE_ADD },
    { "6 or 2 - 3", NODE_OR, NODE_SUBTRACT },
    { "6 or 2 * 3", NODE_OR, NODE_MULTIPLY },
    { "6 or 2 // 3", NODE_OR, NODE_INTEGER_DIVIDE },
    { "6 or 2 < 3", NODE_OR, NODE_LESS_THAN },
    { "6 or 2 <= 3", NODE_OR, NODE_LESS_THAN_EQUAL },
    { "6 or 2 > 3", NODE_OR, NODE_GREATER_THAN },
    { "6 or 2 >= 3", NODE_OR, NODE_GREATER_THAN_EQUAL },
    { "6 or 2 == 3", NODE_OR, NODE_EQUAL },
    { "6 or 2 != 3", NODE_OR, NODE_NOT_EQUAL },
    { "6 or 2 and 3", NODE_OR, NODE_AND },
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

void test_Parser_parseExpression_parens() {
  const char* source = "(42)";
  Parser parser;
  Parser_init(&parser, source, false);
  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_PARENS);
  assert(((UnaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);

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

void test_Parser_parseExpression_mutableAssignment() {
  const char* source = "mut foo = 42";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseExpression(&parser);

  assert(node->type == NODE_MUT);

  Node* assignNode = ((UnaryNode*)node)->arg0;

  assert(((BinaryNode*)assignNode)->arg0->type == NODE_SYMBOL);
  assert(((BinaryNode*)assignNode)->arg1->type == NODE_INTEGER_LITERAL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_parsesJumpStatementsWithoutElse() {
  const char* sources[3] = {
    "if(true) 42;",
    "while(true) 42;",
    "until(true) 42;",
  };

  NodeType nodeTypes[3] = {
    NODE_IF,
    NODE_WHILE,
    NODE_UNTIL,
  };

  for(int i = 0; i < 3; i++) {
    Parser parser;
    Parser_init(&parser, sources[i], false);

    Node* node = Parser_parseStatement(&parser);

    assert(node->type == nodeTypes[i]);
    assert(((TernaryNode*)node)->arg0 != NULL);
    assert(((TernaryNode*)node)->arg0->type == NODE_BOOLEAN_LITERAL);
    assert(((TernaryNode*)node)->arg1 != NULL);
    assert(((TernaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
    assert(((TernaryNode*)node)->arg2 == NULL);

    Node_free(node);
    Parser_free(&parser);
  }
}

void test_Parser_parseStatement_requiresSemicolonsForJumpStatementsOutsideREPL() {
  const char* sources[3] = {
    "if(true) 42",
    "while(true) 42",
    "until(true) 42",
  };

  for(int i = 0; i < 3; i++) {
    Parser parser;
    Parser_init(&parser, sources[i], false);

    Node* node = Parser_parseStatement(&parser);

    assert(node->type == NODE_ERROR);
    assert(((ErrorNode*)node)->type == ERROR_MISSING_SEMICOLON);

    Node_free(node);
    Parser_free(&parser);
  }
}


void test_Parser_parseStatement_parsesJumpStatementsWithoutElseOrSemicolonInREPL() {
  const char* sources[3] = {
    "if(true) 42",
    "while(true) 42",
    "until(true) 42",
  };

  NodeType nodeTypes[3] = {
    NODE_IF,
    NODE_WHILE,
    NODE_UNTIL,
  };

  for(int i = 0; i < 3; i++) {
    Parser parser;
    Parser_init(&parser, sources[i], true);

    Node* node = Parser_parseStatement(&parser);

    assert(node->type == nodeTypes[i]);
    assert(((TernaryNode*)node)->arg0 != NULL);
    assert(((TernaryNode*)node)->arg0->type == NODE_BOOLEAN_LITERAL);
    assert(((TernaryNode*)node)->arg1 != NULL);
    assert(((TernaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
    assert(((TernaryNode*)node)->arg2 == NULL);

    Node_free(node);
    Parser_free(&parser);
  }
}

void test_Parser_parseStatement_parsesJumpElse() {
  const char* sources[3] = {
    "if(true) 42; else 37;",
    "while(true) 42; else 37;",
    "until(true) 42; else 37;",
  };

  NodeType nodeTypes[3] = {
    NODE_IF,
    NODE_WHILE,
    NODE_UNTIL,
  };

  for(int i = 0; i < 3; i++) {
    Parser parser;
    Parser_init(&parser, sources[i], false);

    Node* node = Parser_parseStatement(&parser);

    assert(node->type == nodeTypes[i]);
    assert(((TernaryNode*)node)->arg0 != NULL);
    assert(((TernaryNode*)node)->arg0->type == NODE_BOOLEAN_LITERAL);
    assert(((TernaryNode*)node)->arg1 != NULL);
    assert(((TernaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
    assert(((TernaryNode*)node)->arg2 != NULL);
    assert(((TernaryNode*)node)->arg2->type == NODE_INTEGER_LITERAL);

    Node_free(node);
    Parser_free(&parser);
  }
}

void test_Parser_parseStatement_parsesJumpInAssignment() {
  const char* sources[3] = {
    "a = if(true) 42;",
    "a = while(true) 42;",
    "a = until(true) 42;",
  };

  NodeType nodeTypes[3] = {
    NODE_IF,
    NODE_WHILE,
    NODE_UNTIL,
  };

  for(int i = 0; i < 3; i++) {
    Parser parser;
    Parser_init(&parser, sources[i], false);

    Node* assignmentNode = Parser_parseStatement(&parser);

    assert(assignmentNode->type == NODE_ASSIGN);

    Node* node = ((BinaryNode*)assignmentNode)->arg1;

    assert(node->type == nodeTypes[i]);
    assert(((TernaryNode*)node)->arg0 != NULL);
    assert(((TernaryNode*)node)->arg0->type == NODE_BOOLEAN_LITERAL);
    assert(((TernaryNode*)node)->arg1 != NULL);
    assert(((TernaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
    assert(((TernaryNode*)node)->arg2 == NULL);

    Node_free(node);
    Parser_free(&parser);
  }
}

void test_Parser_parseStatement_parsesJumpElseInAssignment() {
  const char* sources[3] = {
    "a = if(true) 42; else 37;",
    "a = while(true) 42; else 37;",
    "a = until(true) 42; else 37;",
  };

  NodeType nodeTypes[3] = {
    NODE_IF,
    NODE_WHILE,
    NODE_UNTIL,
  };

  for(int i = 0; i < 3; i++) {
    Parser parser;
    Parser_init(&parser, sources[i], false);

    Node* assignmentNode = Parser_parseStatement(&parser);

    assert(assignmentNode->type == NODE_ASSIGN);

    Node* node = ((BinaryNode*)assignmentNode)->arg1;

    assert(node->type == nodeTypes[i]);
    assert(((TernaryNode*)node)->arg0 != NULL);
    assert(((TernaryNode*)node)->arg0->type == NODE_BOOLEAN_LITERAL);
    assert(((TernaryNode*)node)->arg1 != NULL);
    assert(((TernaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
    assert(((TernaryNode*)node)->arg2 != NULL);
    assert(((TernaryNode*)node)->arg2->type == NODE_INTEGER_LITERAL);

    Node_free(node);
    Parser_free(&parser);
  }
}

void test_Parser_parseStatement_parsesJumpInParens() {
  const char* sources[3] = {
    "a = (if(true) 42);",
    "a = (while(true) 42);",
    "a = (until(true) 42);",
  };

  NodeType nodeTypes[3] = {
    NODE_IF,
    NODE_WHILE,
    NODE_UNTIL,
  };

  for(int i = 0; i < 3; i++) {
    Parser parser;
    Parser_init(&parser, sources[i], false);

    Node* assignmentNode = Parser_parseStatement(&parser);

    assert(assignmentNode->type == NODE_ASSIGN);

    Node* parenNode = ((BinaryNode*)assignmentNode)->arg1;

    assert(parenNode->type == NODE_PARENS);

    Node* node = ((UnaryNode*)parenNode)->arg0;

    assert(node->type == nodeTypes[i]);
    assert(((TernaryNode*)node)->arg0 != NULL);
    assert(((TernaryNode*)node)->arg0->type == NODE_BOOLEAN_LITERAL);
    assert(((TernaryNode*)node)->arg1 != NULL);
    assert(((TernaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
    assert(((TernaryNode*)node)->arg2 == NULL);

    Node_free(assignmentNode);
    Parser_free(&parser);
  }
}

void test_Parser_parseStatement_parsesJumpElseInParens() {
  const char* sources[3] = {
    "a = (if(true) 42; else 37);",
    "a = (while(true) 42; else 37);",
    "a = (until(true) 42; else 37);",
  };

  NodeType nodeTypes[3] = {
    NODE_IF,
    NODE_WHILE,
    NODE_UNTIL,
  };

  for(int i = 0; i < 3; i++) {
    Parser parser;
    Parser_init(&parser, sources[i], false);

    Node* assignmentNode = Parser_parseStatement(&parser);

    assert(assignmentNode->type == NODE_ASSIGN);

    Node* parenNode = ((BinaryNode*)assignmentNode)->arg1;

    assert(parenNode->type == NODE_PARENS);

    Node* node = ((UnaryNode*)parenNode)->arg0;

    assert(node != NULL);
    assert(node->type == nodeTypes[i]);
    assert(((TernaryNode*)node)->arg0 != NULL);
    assert(((TernaryNode*)node)->arg0->type == NODE_BOOLEAN_LITERAL);
    assert(((TernaryNode*)node)->arg1 != NULL);
    assert(((TernaryNode*)node)->arg1->type == NODE_INTEGER_LITERAL);
    assert(((TernaryNode*)node)->arg2 != NULL);
    assert(((TernaryNode*)node)->arg2->type == NODE_INTEGER_LITERAL);

    Node_free(assignmentNode);
    Parser_free(&parser);
  }
}

void test_Parser_parseStatement_continue() {
  const char* source = "continue;";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_CONTINUE);
  assert(((UnaryNode*)node)->arg0 == NULL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_continueTo() {
  const char* source = "continue 3;";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_CONTINUE);
  assert(((UnaryNode*)node)->arg0 != NULL);
  assert(((UnaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_break() {
  const char* source = "break;";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_BREAK);
  assert(((BinaryNode*)node)->arg0 == NULL);
  assert(((BinaryNode*)node)->arg1 == NULL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_breakTo() {
  const char* source = "break 3;";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_BREAK);
  assert(((BinaryNode*)node)->arg0 != NULL);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1 == NULL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_breakWith() {
  const char* source = "break with true;";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_BREAK);
  assert(((BinaryNode*)node)->arg0 == NULL);
  assert(((BinaryNode*)node)->arg1 != NULL);
  assert(((BinaryNode*)node)->arg1->type == NODE_BOOLEAN_LITERAL);

  Node_free(node);
  Parser_free(&parser);
}

void test_Parser_parseStatement_breakToWith() {
  const char* source = "break 3 with true;";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_BREAK);
  assert(((BinaryNode*)node)->arg0 != NULL);
  assert(((BinaryNode*)node)->arg0->type == NODE_INTEGER_LITERAL);
  assert(((BinaryNode*)node)->arg1 != NULL);
  assert(((BinaryNode*)node)->arg1->type == NODE_BOOLEAN_LITERAL);

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
  const char* source = "1 + 1 if";

  Parser parser;
  Parser_init(&parser, source, false);

  Node* node = Parser_parseStatement(&parser);

  assert(node->type == NODE_ERROR);

  ErrorNode* eNode = (ErrorNode*)node;
  assert(eNode->type == ERROR_MISSING_SEMICOLON);
  assert(eNode->token.type == TOKEN_IF);

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
