#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "node.h"

inline static void Node_init(Node* self, NodeType type, size_t line) {
  self->type = type;
  self->line = line;
}

inline static void AtomNode_init(AtomNode* self, NodeType type, size_t line, const char* text, size_t length) {
  assert(type == NODE_INTEGER_LITERAL
      || type == NODE_BOOLEAN_LITERAL
      || type == NODE_SYMBOL);
  Node_init(&(self->node), type, line);
  self->text = text;
  self->length = length;
}

Node* Node_new(NodeType type, size_t line) {
  assert(type == NODE_EOF);
  Node* node = malloc(sizeof(Node));
  Node_init(node, type, line);
  return node;
}

Node* AtomNode_new(NodeType type, size_t line, const char* text, size_t length) {
  assert(type == NODE_INTEGER_LITERAL
      || type == NODE_BOOLEAN_LITERAL
      || type == NODE_SYMBOL);
  AtomNode* node = malloc(sizeof(AtomNode));
  AtomNode_init(node, type, line, text, length);
  return (Node*)node;
}

inline static void AtomNode_free(AtomNode* self) {
  free(self);
}

inline static void UnaryNode_init(UnaryNode* self, NodeType type, size_t line, Node* arg0) {
  assert(type == NODE_NEGATE
      || type == NODE_LOGICAL_NOT
      || type == NODE_LOOP);
  Node_init(&(self->node), type, line);
  self->arg0 = arg0;
}

Node* UnaryNode_new(NodeType type, size_t line, Node* arg0) {
  UnaryNode* node = malloc(sizeof(UnaryNode));
  UnaryNode_init(node, type, line, arg0);
  return (Node*)node;
}

inline static void UnaryNode_free(UnaryNode* self) {
  Node_free(self->arg0);
  free(self);
}

inline static void BinaryNode_init(BinaryNode* self, NodeType type, size_t line, Node* arg0, Node* arg1) {
  assert(type == NODE_ASSIGN
      || type == NODE_ADD
      || type == NODE_SUBTRACT
      || type == NODE_MULTIPLY
      || type == NODE_INTEGER_DIVIDE
      || type == NODE_LESS_THAN
      || type == NODE_LESS_THAN_EQUAL
      || type == NODE_GREATER_THAN
      || type == NODE_GREATER_THAN_EQUAL
      || type == NODE_EQUAL
      || type == NODE_NOT_EQUAL);
  Node_init(&(self->node), type, line);
  self->arg0 = arg0;
  self->arg1 = arg1;
}

Node* BinaryNode_new(NodeType type, size_t line, Node* arg0, Node* arg1) {
  BinaryNode* node = malloc(sizeof(BinaryNode));
  BinaryNode_init(node, type, line, arg0, arg1);
  return (Node*)node;
}

inline static void BinaryNode_free(BinaryNode* self) {
  Node_free(self->arg0);
  Node_free(self->arg1);
  free(self);
}

inline static void TernaryNode_init(TernaryNode* self, NodeType type, size_t line, Node* arg0, Node* arg1, Node* arg2) {
  assert(type == NODE_IF
      || type == NODE_WHILE
      || type == NODE_UNTIL);
  Node_init(&(self->node), type, line);
  self->arg0 = arg0;
  self->arg1 = arg1;
  self->arg2 = arg2;
}

Node* TernaryNode_new(NodeType type, size_t line, Node* arg0, Node* arg1, Node* arg2) {
  TernaryNode* node = malloc(sizeof(TernaryNode));
  TernaryNode_init(node, type, line, arg0, arg1, arg2);
  return (Node*)node;
}

inline static void TernaryNode_free(TernaryNode* self) {
  Node_free(self->arg0);
  Node_free(self->arg1);
  Node_free(self->arg2);
  free(self);
}

inline static void ErrorNode_init(ErrorNode* self, ParseErrorType type, Token token, Token auxToken, Node* previous) {
  Node_init(&(self->node), NODE_ERROR, token.line);
  self->type = type;
  self->token = token;
  self->auxToken = auxToken;
  self->previous = previous;
}

Node* ErrorNode_newWithAuxAndPrevious(ParseErrorType type, Token token, Token auxToken, Node* previous) {
  ErrorNode* node = malloc(sizeof(ErrorNode));
  ErrorNode_init(node, type, token, auxToken, previous);
  return (Node*)node;
}

Node* ErrorNode_new(ParseErrorType type, Token token) {
  Token auxToken;
  auxToken.type = NO_TOKEN;
  return ErrorNode_newWithAuxAndPrevious(type, token, auxToken, NULL);
}

inline static void ErrorNode_free(ErrorNode* self) {
  if(self->previous != NULL) Node_free(self->previous);
  free(self);
}

void Node_free(Node* self) {
  if(self == NULL) return;

  switch(self->type) {
    case NODE_EOF:
      free(self);
      return;

    case NODE_INTEGER_LITERAL:
    case NODE_BOOLEAN_LITERAL:
    case NODE_SYMBOL:
      AtomNode_free((AtomNode*)self);
      return;

    case NODE_NEGATE:
    case NODE_LOGICAL_NOT:
    case NODE_LOOP:
      UnaryNode_free((UnaryNode*)self);
      return;

    case NODE_ASSIGN:
    case NODE_MUT_ASSIGN:
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
      BinaryNode_free((BinaryNode*)self);
      return;

    case NODE_IF:
    case NODE_WHILE:
    case NODE_UNTIL:
      TernaryNode_free((TernaryNode*)self);
      return;

    case NODE_ERROR:
      ErrorNode_free((ErrorNode*)self);
  }
}

void ErrorNode_print(Node* node) {
  assert(node->type == NODE_ERROR);

  ErrorNode* self = (ErrorNode*)node;

  if(self->previous != NULL && self->previous->type == NODE_ERROR) {
    ErrorNode_print(self->previous);
  }

  switch(self->type) {
    case ERROR_MISSING_SEMICOLON:
      printError(self->token.line, "Missing ';'.");
      break;

    case ERROR_PAREN_OPENED_BUT_NOT_CLOSED:
      printError(
        self->token.line,
        "Parentheses opened but not closed.\n"
        "\tParentheses opened on line %zu.",
        self->auxToken.line
      );
      break;

    case ERROR_UNEXPECTED_TOKEN:
      if(self->token.type == TOKEN_EOF) {
        printError(self->token.line, "Unexpected end of file.");
      } else if(self->token.length > 64) {
        /*
         * This check also protects against overflowing the integer cast in
         * the else block, so it's a security concern as well.
         */
        printError(
          self->token.line,
          "Unexpected token '%.*s...'.",
          64,
          self->token.lexeme
        );
      } else {
        printError(
          self->token.line,
          "Unexpected token '%.*s'.",
          (int)(self->token.length),
          self->token.lexeme
        );
      }
      break;
  }
}

#ifdef TEST

#include <stdbool.h>

void test_Node_new_basic() {
  Node* node = Node_new(NODE_EOF, 5);

  assert(node->type == NODE_EOF);
  assert(node->line == 5);

  Node_free(node);
}

void test_AtomNode_new_basic() {
  char* text = "42";

  Node* node = AtomNode_new(NODE_INTEGER_LITERAL, 42, text, 2);

  assert(node->type == NODE_INTEGER_LITERAL);
  assert(node->line == 42);

  AtomNode* subNode = (AtomNode*)node;

  assert(subNode->node.type == NODE_INTEGER_LITERAL);
  assert(subNode->node.line == 42);

  Node_free(node);
}

void test_UnaryNode_new_basic() {
  Node* arg0 = AtomNode_new(NODE_INTEGER_LITERAL, 42, "42", 2);
  Node* node = UnaryNode_new(NODE_NEGATE, 5, arg0);

  assert(node->type == NODE_NEGATE);
  assert(node->line == 5);

  UnaryNode* subNode = (UnaryNode*)node;

  assert(subNode->node.type == NODE_NEGATE);
  assert(subNode->node.line == 5);
  assert(subNode->arg0 == arg0);

  Node_free(node);
}

void test_BinaryNode_new_basic() {
  Node* arg0 = AtomNode_new(NODE_INTEGER_LITERAL, 42, "42", 2);
  Node* arg1 = AtomNode_new(NODE_INTEGER_LITERAL, 44, "44", 2);
  Node* node = BinaryNode_new(NODE_ADD, 5, arg0, arg1);

  assert(node->type == NODE_ADD);
  assert(node->line == 5);

  BinaryNode* subNode = (BinaryNode*)node;

  assert(subNode->node.type == NODE_ADD);
  assert(subNode->node.line == 5);
  assert(subNode->arg0 == arg0);
  assert(subNode->arg1 == arg1);

  Node_free(node);
}

#endif
