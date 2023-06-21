#ifndef NODE_H
#define NODE_H

#include <stdlib.h>

#include "tokenizer.h"

typedef enum {
  // Atom Nodes
  NODE_INTEGER_LITERAL,
  NODE_NIL_LITERAL,
  NODE_BOOLEAN_LITERAL,
  NODE_SYMBOL,

  // Unary Nodes
  NODE_NEGATE,
  NODE_LOGICAL_NOT,
  NODE_PARENS,
  NODE_MUT,
  NODE_LOOP,
  NODE_CONTINUE,

  // Binary Nodes
  NODE_ASSIGN,
  NODE_ADD,
  NODE_SUBTRACT,
  NODE_MULTIPLY,
  NODE_INTEGER_DIVIDE,
  NODE_LESS_THAN,
  NODE_LESS_THAN_EQUAL,
  NODE_GREATER_THAN,
  NODE_GREATER_THAN_EQUAL,
  NODE_EQUAL,
  NODE_NOT_EQUAL,
  NODE_AND,
  NODE_OR,
  NODE_BREAK,
  NODE_CALL,

  // Ternary Nodes
  NODE_IF,
  NODE_WHILE,
  NODE_UNTIL,

  // List Nodes
  NODE_BLOCK,

  // Auxiliary nodes
  NODE_EOF,
} NodeType;

typedef struct {
  NodeType type;
  size_t line;
} Node;

Node* Node_new(NodeType type, size_t line);

typedef struct {
  Node node;
  const char* text;
  size_t length;
} AtomNode;

Node* AtomNode_new(NodeType type, size_t line, const char* text, size_t length);

typedef struct {
  Node node;
  Node* arg0;
} UnaryNode;

Node* UnaryNode_new(NodeType type, size_t line, Node* arg0);

typedef struct {
  Node node;
  Node* arg0;
  Node* arg1;
} BinaryNode;

Node* BinaryNode_new(NodeType type, size_t line, Node* arg0, Node* arg1);

typedef struct {
  Node node;
  Node* arg0;
  Node* arg1;
  Node* arg2;
} TernaryNode;

Node* TernaryNode_new(NodeType type, size_t line, Node* arg0, Node* arg1, Node* arg2);

typedef struct {
  Node node;
  Node** items;
  size_t count;
  size_t capacity;
} ListNode;

ListNode* ListNode_new(NodeType type, size_t line);
void ListNode_append(ListNode*, Node*);
Node* ListNode_finish(ListNode*);

void Node_del(Node* self);

#ifdef TEST

void test_Node_new_basic();
void test_AtomNode_new_basic();
void test_UnaryNode_new_basic();
void test_BinaryNode_new_basic();

#endif

#endif
