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
      || type == NODE_NIL_LITERAL
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
      || type == NODE_NIL_LITERAL
      || type == NODE_BOOLEAN_LITERAL
      || type == NODE_SYMBOL);
  AtomNode* node = malloc(sizeof(AtomNode));
  AtomNode_init(node, type, line, text, length);
  return (Node*)node;
}

inline static void AtomNode_del(AtomNode* self) {
  free(self);
}

inline static void UnaryNode_init(UnaryNode* self, NodeType type, size_t line, Node* arg0) {
  assert(type == NODE_NEGATE
      || type == NODE_LOGICAL_NOT
      || type == NODE_PARENS
      || type == NODE_MUT
      || type == NODE_LOOP
      || type == NODE_CONTINUE);
  Node_init(&(self->node), type, line);
  self->arg0 = arg0;
}

Node* UnaryNode_new(NodeType type, size_t line, Node* arg0) {
  UnaryNode* node = malloc(sizeof(UnaryNode));
  UnaryNode_init(node, type, line, arg0);
  return (Node*)node;
}

inline static void UnaryNode_del(UnaryNode* self) {
  Node_del(self->arg0);
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
      || type == NODE_NOT_EQUAL
      || type == NODE_AND
      || type == NODE_OR
      || type == NODE_BREAK
      || type == NODE_CALL);
  Node_init(&(self->node), type, line);
  self->arg0 = arg0;
  self->arg1 = arg1;
}

Node* BinaryNode_new(NodeType type, size_t line, Node* arg0, Node* arg1) {
  BinaryNode* node = malloc(sizeof(BinaryNode));
  BinaryNode_init(node, type, line, arg0, arg1);
  return (Node*)node;
}

inline static void BinaryNode_del(BinaryNode* self) {
  Node_del(self->arg0);
  Node_del(self->arg1);
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

inline static void TernaryNode_del(TernaryNode* self) {
  Node_del(self->arg0);
  Node_del(self->arg1);
  Node_del(self->arg2);
  free(self);
}

ListNode* ListNode_new(NodeType type, size_t line) {
  ListNode* self = malloc(sizeof(ListNode));
  // TODO Handle this
  assert(self != NULL);

  Node_init(&(self->node), type, line);

  self->count = 0;
  self->capacity = 0;
  self->items = NULL;

  return self;
}

inline static void ListNode_del(ListNode* self) {
  for(size_t i = 0; i < self->count; i++) {
    Node_del(self->items[i]);
  }

  if(self->capacity > 0) free(self->items);
  free(self);
}

void ListNode_append(ListNode* self, Node* item) {
  if(self->count == self->capacity) {
    if(self->capacity == 0) {
      self->capacity = 8;
    } else {
      self->capacity *= 2;
    }

    self->items = realloc(self->items, sizeof(Node*) * self->capacity);

    // TODO Handle this
    assert(self->items != NULL);
  }

  self->items[self->count++] = item;
}

Node* ListNode_finish(ListNode* self) {
  self->capacity = self->count;
  self->items = realloc(self->items, sizeof(Node*) * self->capacity);
  return (Node*)self;
}

void Node_del(Node* self) {
  if(self == NULL) return;

  switch(self->type) {
    case NODE_EOF:
      free(self);
      return;

    case NODE_NIL_LITERAL:
    case NODE_INTEGER_LITERAL:
    case NODE_BOOLEAN_LITERAL:
    case NODE_SYMBOL:
      AtomNode_del((AtomNode*)self);
      return;

    case NODE_CONTINUE:
    case NODE_NEGATE:
    case NODE_PARENS:
    case NODE_LOGICAL_NOT:
    case NODE_LOOP:
    case NODE_MUT:
      UnaryNode_del((UnaryNode*)self);
      return;

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
    case NODE_BREAK:
    case NODE_CALL:
      BinaryNode_del((BinaryNode*)self);
      return;

    case NODE_IF:
    case NODE_WHILE:
    case NODE_UNTIL:
      TernaryNode_del((TernaryNode*)self);
      return;

    case NODE_BLOCK:
      ListNode_del((ListNode*) self);
      return;
  }
}

#ifdef TEST

#include <stdbool.h>

void test_Node_new_basic() {
  Node* node = Node_new(NODE_EOF, 5);

  assert(node->type == NODE_EOF);
  assert(node->line == 5);

  Node_del(node);
}

void test_AtomNode_new_basic() {
  char* text = "42";

  Node* node = AtomNode_new(NODE_INTEGER_LITERAL, 42, text, 2);

  assert(node->type == NODE_INTEGER_LITERAL);
  assert(node->line == 42);

  AtomNode* subNode = (AtomNode*)node;

  assert(subNode->node.type == NODE_INTEGER_LITERAL);
  assert(subNode->node.line == 42);

  Node_del(node);
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

  Node_del(node);
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

  Node_del(node);
}

#endif
