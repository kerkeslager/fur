#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "node.h"

inline static void Node_init(Node* self, NodeType type, size_t line) {
  self->type = type;
  self->line = line;
}

inline static void AtomNode_init(AtomNode* self, NodeType type, size_t line, const char* text, size_t length) {
  assert(type == NODE_INTEGER_LITERAL);
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
  assert(type == NODE_INTEGER_LITERAL);
  AtomNode* node = malloc(sizeof(AtomNode));
  AtomNode_init(node, type, line, text, length);
  return (Node*)node;
}

inline static void AtomNode_free(AtomNode* self) {
  free(self);
}

inline static void UnaryNode_init(UnaryNode* self, NodeType type, size_t line, Node* arg0) {
  assert(type == NODE_NEGATE);
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
  assert(type == NODE_ADD
      || type == NODE_SUBTRACT
      || type == NODE_MULTIPLY
      || type == NODE_INTEGER_DIVIDE);
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

inline static void ErrorNode_init(ErrorNode* self, ErrorType type, Token token) {
  Node_init(&(self->node), NODE_ERROR, token.line);
  self->type = type;
  self->token = token;
}

Node* ErrorNode_new(ErrorType type, Token token) {
  ErrorNode* node = malloc(sizeof(ErrorNode));
  ErrorNode_init(node, type, token);
  return (Node*)node;
}

inline static void ErrorNode_free(ErrorNode* self) {
  free(self);
}

void Node_free(Node* self) {
  switch(self->type) {
    case NODE_EOF:
      free(self);
      return;

    case NODE_INTEGER_LITERAL:
      AtomNode_free((AtomNode*)self);
      return;

    case NODE_NEGATE:
      UnaryNode_free((UnaryNode*)self);
      return;

    case NODE_ADD:
    case NODE_SUBTRACT:
    case NODE_MULTIPLY:
    case NODE_INTEGER_DIVIDE:
      BinaryNode_free((BinaryNode*)self);
      return;

    case NODE_ERROR:
      ErrorNode_free((ErrorNode*)self);
  }
}

void ErrorNode_print(Node* node) {
  assert(node->type == NODE_ERROR);

  ErrorNode* self = (ErrorNode*)node;

  switch(self->type) {
    case ERROR_MISSING_SEMICOLON:
      assert(false);

    case ERROR_UNEXPECTED_TOKEN:
      assert(false);
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
