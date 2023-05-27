#include <assert.h>
#include <stdbool.h>

#include "compiler.h"
#include "node.h"
#include "parser.h"

void Compiler_init(Compiler* self, bool repl) {
  self->repl = repl;
  self->hasErrors = false;
}

static void Compiler_error(Compiler* self, Node* errorNode) {
  assert(errorNode != NULL);
  assert(errorNode->type == NODE_ERROR);

  self->hasErrors = true;

  ErrorNode_print(errorNode);
}

inline static void Compiler_emitOp(InstructionList* out, Instruction op) {
  InstructionList_append(out, (uint8_t)op);
}

inline static void Compiler_emitInt32(InstructionList* out, int32_t i) {
  InstructionList_appendInt32(out, i);
}

inline static void Compiler_emitInteger(Node* node, InstructionList* out) {
  // TODO Handle overflows

  AtomNode* aNode = (AtomNode*)node;
  int32_t result = 0;

  for(size_t i = 0; i < aNode->length; i++) {
    result *= 10;
    result += aNode->text[i] - '0';
  }

  Compiler_emitOp(out, OP_INTEGER);
  Compiler_emitInt32(out, result);
}

void Compiler_emitNode(InstructionList* out, Node* node);

inline static void Compiler_emitBinaryNode(InstructionList* out, Instruction op, Node* node) {
  Compiler_emitNode(out, ((BinaryNode*)node)->arg0);
  Compiler_emitNode(out, ((BinaryNode*)node)->arg1);
  InstructionList_append(out, op);
}

// TODO Switch arguments
void Compiler_emitNode(InstructionList* out, Node* node) {
  switch(node->type) {
    case NODE_INTEGER_LITERAL:
      return Compiler_emitInteger(node, out);

    case NODE_NEGATE:
      Compiler_emitNode(out, ((UnaryNode*)node)->arg0);
      return Compiler_emitOp(out, OP_NEGATE);

    case NODE_ADD:            Compiler_emitBinaryNode(out, OP_ADD, node);       return;
    case NODE_SUBTRACT:       Compiler_emitBinaryNode(out, OP_SUBTRACT, node);  return;
    case NODE_MULTIPLY:       Compiler_emitBinaryNode(out, OP_MULTIPLY, node);  return;
    case NODE_INTEGER_DIVIDE: Compiler_emitBinaryNode(out, OP_IDIVIDE, node);   return;

    default:
      assert(false);
  }
}

bool Compiler_compile(Compiler* self, InstructionList* out, const char* source) {
  Parser parser;
  Parser_init(&parser, source, self->repl);

  Node* statement = Parser_parseStatement(&parser);

  if(statement->type == NODE_EOF) {
    /*
     * This is so that empty files or repl lines emit a return value, since
     * callers expect this.
     */
    Compiler_emitOp(out, OP_NIL);
    Compiler_emitOp(out, OP_RETURN);
    return true;
  } else if(statement->type == NODE_ERROR) {
    Compiler_error(self, statement);
  } else {
    Compiler_emitNode(out, statement);
  }

  while((statement = Parser_parseStatement(&parser))->type != NODE_EOF) {
    if(statement->type == NODE_ERROR) {
      Compiler_error(self, statement);
    } else {
      // Drop the result of previous statement
      /*
       * TODO
       * We can pass a flag into emitNode() to tell it if the result of the
       * expression will be used, and not emit unused results that we'll
       * just have to drop.
       */
      Compiler_emitOp(out, OP_DROP);
      Compiler_emitNode(out, statement);
    }
  }

  Compiler_emitOp(out, OP_RETURN);

  Parser_free(&parser);

  return !(self->hasErrors);
}

#ifdef TEST

void test_Compiler_emitNode_emitsIntegerLiteral() {
  const char* text = "42";
  Node* node = AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 2);
  InstructionList out;
  InstructionList_init(&out);

  Compiler_emitNode(&out, node);

  assert(out.count == 5);
  assert(out.items[0] == OP_INTEGER);
  assert(*(int32_t*)(out.items + 1) == 42);

  Node_free(node);
  InstructionList_free(&out);
}

void test_Compiler_emitNode_emitsNegate() {
  const char* text = "42";
  Node* node = UnaryNode_new(
      NODE_NEGATE,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 2)
  );
  InstructionList out;
  InstructionList_init(&out);

  Compiler_emitNode(&out, node);

  assert(out.count == 6);
  assert(out.items[0] == OP_INTEGER);
  assert(*(int32_t*)(out.items + 1) == 42);
  assert(out.items[5] == OP_NEGATE);

  Node_free(node);
  InstructionList_free(&out);
}

#include<stdio.h>

void test_Compiler_emitNode_emitsAdd() {
  const char* text = "1";
  Node* node = BinaryNode_new(
      NODE_ADD,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1)
  );
  InstructionList out;
  InstructionList_init(&out);

  Compiler_emitNode(&out, node);

  assert(out.count == 11);
  assert(out.items[0] == OP_INTEGER);
  assert(out.items[5] == OP_INTEGER);
  assert(out.items[10] == OP_ADD);

  Node_free(node);
  InstructionList_free(&out);
}

void test_Compiler_emitNode_emitsSubtract() {
  const char* text = "1";
  Node* node = BinaryNode_new(
      NODE_SUBTRACT,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1)
  );
  InstructionList out;
  InstructionList_init(&out);

  Compiler_emitNode(&out, node);

  assert(out.count == 11);
  assert(out.items[0] == OP_INTEGER);
  assert(out.items[5] == OP_INTEGER);
  assert(out.items[10] == OP_SUBTRACT);

  Node_free(node);
  InstructionList_free(&out);
}

void test_Compiler_emitNode_emitsMultiply() {
  const char* text = "1";
  Node* node = BinaryNode_new(
      NODE_MULTIPLY,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1)
  );
  InstructionList out;
  InstructionList_init(&out);

  Compiler_emitNode(&out, node);

  assert(out.count == 11);
  assert(out.items[0] == OP_INTEGER);
  assert(out.items[5] == OP_INTEGER);
  assert(out.items[10] == OP_MULTIPLY);

  Node_free(node);
  InstructionList_free(&out);
}

void test_Compiler_emitNode_emitsIntegerDivide() {
  const char* text = "1";
  Node* node = BinaryNode_new(
      NODE_INTEGER_DIVIDE,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1)
  );
  InstructionList out;
  InstructionList_init(&out);

  Compiler_emitNode(&out, node);

  assert(out.count == 11);
  assert(out.items[0] == OP_INTEGER);
  assert(out.items[5] == OP_INTEGER);
  assert(out.items[10] == OP_IDIVIDE);

  Node_free(node);
  InstructionList_free(&out);
}

void test_Compiler_compile_emitsNilOnEmptyInput() {
  const char* text = "";
  InstructionList out;
  InstructionList_init(&out);
  Compiler compiler;
  Compiler_init(&compiler, false);

  bool success = Compiler_compile(&compiler, &out, text);

  assert(success);
  assert(out.count == 2);
  assert(out.items[0] == OP_NIL);
  assert(out.items[1] == OP_RETURN);

  InstructionList_free(&out);
}

void test_Compiler_compile_emitsNilOnBlankInput() {
  const char* text = " \t \n \r";
  InstructionList out;
  InstructionList_init(&out);
  Compiler compiler;
  Compiler_init(&compiler, false);

  bool success = Compiler_compile(&compiler, &out, text);

  assert(success);
  assert(out.count == 2);
  assert(out.items[0] == OP_NIL);
  assert(out.items[1] == OP_RETURN);

  InstructionList_free(&out);
}

#endif
