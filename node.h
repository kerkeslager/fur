#ifndef NODE_H
#define NODE_H

#include <stdlib.h>

#include "tokenizer.h"

typedef enum {
  // Atom Nodes
  NODE_INTEGER_LITERAL,
  NODE_BOOLEAN_LITERAL,

  // Unary Nodes
  NODE_NEGATE,

  // Binary Nodes
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

  // Auxiliary nodes
  NODE_ERROR,
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

typedef enum {
  ERROR_MISSING_SEMICOLON,
  ERROR_PAREN_OPENED_BUT_NOT_CLOSED,
  ERROR_UNEXPECTED_TOKEN,
} ParseErrorType;

typedef struct {
  Node node;
  ParseErrorType type;
  Token token;
  Token auxToken;
  Node* previous;
} ErrorNode;

Node* ErrorNode_new(ParseErrorType type, Token token);
Node* ErrorNode_newWithAuxAndPrevious(ParseErrorType, Token token, Token auxToken, Node* previous);
void ErrorNode_print(Node*);

void Node_free(Node* self);

#ifdef TEST

void test_Node_new_basic();
void test_AtomNode_new_basic();
void test_UnaryNode_new_basic();
void test_BinaryNode_new_basic();

#endif

#endif
