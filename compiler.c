#include <assert.h>
#include <stdbool.h>

#include "compiler.h"
#include "node.h"
#include "parser.h"
#include "tokenizer.h"

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

void Compiler_emitNode(Node* node, InstructionList* out) {
  switch(node->type) {
    case NODE_INTEGER_LITERAL:
      return Compiler_emitInteger(node, out);

    case NODE_NEGATE:
      assert(false);

    case NODE_ADD:
    case NODE_SUBTRACT:
    case NODE_MULTIPLY:
    case NODE_INTEGER_DIVIDE:
      assert(false);

    case NODE_ERROR:
      assert(false);
  }
}

void Compiler_compile(const char* source, InstructionList* out) {
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Node* expression = parseExpression(&tokenizer);

  Compiler_emitNode(expression, out);
}

#ifdef TEST

void test_Compiler_emitNode_emitsIntegerLiteral() {
  const char* text = "42";
  Node* node = AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 2);
  InstructionList out;
  InstructionList_init(&out);

  Compiler_emitNode(node, &out);

  assert(out.count == 5);
  assert(out.items[0] == OP_INTEGER);
  assert(*(int32_t*)(out.items + 1) == 42);

  Node_free(node);
  InstructionList_free(&out);
}

#endif
