#ifndef NODE_H
#define NODE_H

#include <stdlib.h>

typedef enum {
  // Atom Nodes
  NODE_INTEGER_LITERAL,

  // Unary Nodes
  NODE_NEGATE,

  // Binary Nodes
  NODE_ADD,
  NODE_SUBTRACT,
  NODE_MULTIPLY,
  NODE_INTEGER_DIVIDE,

  NODE_ERROR,
} NodeType;

typedef struct {
  NodeType type;
  size_t line;
} Node;

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

void Node_free(Node* self);

#ifdef TEST

void test_AtomNode_new_basic();
void test_UnaryNode_new_basic();
void test_BinaryNode_new_basic();

#endif

#endif