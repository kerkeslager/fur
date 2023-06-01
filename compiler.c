#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "compiler.h"
#include "node.h"
#include "parser.h"

void Compiler_init(Compiler* self, bool repl) {
  self->repl = repl;
  self->hasErrors = false;
}

void Compiler_error(Compiler* self, Node* errorNode) {
  assert(errorNode != NULL);
  assert(errorNode->type == NODE_ERROR);

  self->hasErrors = true;

  ErrorNode_print(errorNode);
}

inline static void Compiler_emitOp(InstructionList* out, Instruction op, size_t line) {
  InstructionList_append(out, (uint8_t)op, line);
}

inline static void Compiler_emitInt32(InstructionList* out, int32_t i, size_t line) {
  InstructionList_appendInt32(out, i, line);
}

inline static void Compiler_emitInteger(InstructionList* out, AtomNode* node) {
  // TODO Handle overflows

  int32_t result = 0;

  for(size_t i = 0; i < node->length; i++) {
    result *= 10;
    result += node->text[i] - '0';
  }

  Compiler_emitOp(out, OP_INTEGER, node->node.line);
  Compiler_emitInt32(out, result, node->node.line);
}

inline static void Compiler_emitBoolean(InstructionList* out, AtomNode* node) {
  /*
   * Since there are only two possibilities which should have been matched
   * by the tokenizer, it should be sufficient to check only the first character.
   * However, we assert the full values for debug.
   */
  if(*(node->text) == 't') {
    assert(strncmp("true", node->text, node->length) == 0);
    return Compiler_emitOp(out, OP_TRUE, node->node.line);
  } else {
    assert(strncmp("false", ((AtomNode*)node)->text, ((AtomNode*)node)->length) == 0);
    return Compiler_emitOp(out, OP_FALSE, node->node.line);
  }
}

void Compiler_emitNode(InstructionList* out, Node* node);

inline static void Compiler_emitBinaryNode(InstructionList* out, Instruction op, Node* node) {
  Compiler_emitNode(out, ((BinaryNode*)node)->arg0);
  Compiler_emitNode(out, ((BinaryNode*)node)->arg1);
  InstructionList_append(out, op, node->line);
}

// TODO Switch arguments
void Compiler_emitNode(InstructionList* out, Node* node) {
  switch(node->type) {
    case NODE_INTEGER_LITERAL:
      return Compiler_emitInteger(out, (AtomNode*)node);

    case NODE_BOOLEAN_LITERAL:
      return Compiler_emitBoolean(out, (AtomNode*)node);

    case NODE_NEGATE:
      Compiler_emitNode(out, ((UnaryNode*)node)->arg0);
      return Compiler_emitOp(out, OP_NEGATE, node->line);

    case NODE_LOGICAL_NOT:
      Compiler_emitNode(out, ((UnaryNode*)node)->arg0);
      return Compiler_emitOp(out, OP_NOT, node->line);

    case NODE_ADD:            Compiler_emitBinaryNode(out, OP_ADD, node);       return;
    case NODE_SUBTRACT:       Compiler_emitBinaryNode(out, OP_SUBTRACT, node);  return;
    case NODE_MULTIPLY:       Compiler_emitBinaryNode(out, OP_MULTIPLY, node);  return;
    case NODE_INTEGER_DIVIDE: Compiler_emitBinaryNode(out, OP_IDIVIDE, node);   return;

    case NODE_LESS_THAN:          Compiler_emitBinaryNode(out, OP_LESS_THAN, node);   return;
    case NODE_LESS_THAN_EQUAL:    Compiler_emitBinaryNode(out, OP_LESS_THAN_EQUAL, node);   return;
    case NODE_GREATER_THAN:       Compiler_emitBinaryNode(out, OP_GREATER_THAN, node);   return;
    case NODE_GREATER_THAN_EQUAL: Compiler_emitBinaryNode(out, OP_GREATER_THAN_EQUAL, node);   return;
    case NODE_EQUAL:              Compiler_emitBinaryNode(out, OP_EQUAL, node);   return;
    case NODE_NOT_EQUAL:          Compiler_emitBinaryNode(out, OP_NOT_EQUAL, node);   return;

    case NODE_ERROR:
    case NODE_EOF:
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
    Compiler_emitOp(out, OP_NIL, statement->line);
    Compiler_emitOp(out, OP_RETURN, statement->line);
    return true;
  } else if(statement->type == NODE_ERROR) {
    Compiler_error(self, statement);
  } else {
    Compiler_emitNode(out, statement);
  }

  Node_free(statement);

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
      /*
       * TODO
       * The OP_DROP is really part of the previous statement, and should have
       * the line number of the previous statement.
       */
      Compiler_emitOp(out, OP_DROP, statement->line);
      Compiler_emitNode(out, statement);
    }

    Node_free(statement);
  }

  Compiler_emitOp(out, OP_RETURN, statement->line);

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

void test_Compiler_emitNode_emitsNot() {
  const char* text = "true";
  Node* node = UnaryNode_new(
      NODE_LOGICAL_NOT,
      1,
      AtomNode_new(NODE_BOOLEAN_LITERAL, 1, text, 4)
  );
  InstructionList out;
  InstructionList_init(&out);

  Compiler_emitNode(&out, node);

  assert(out.count == 2);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_NOT);

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

void test_Compiler_emitNode_emitsComparisons() {
  NodeType COMPARISON_NODE_TYPES[] = {
    NODE_LESS_THAN,
    NODE_LESS_THAN_EQUAL,
    NODE_GREATER_THAN,
    NODE_GREATER_THAN_EQUAL,
    NODE_EQUAL,
    NODE_NOT_EQUAL,
  };

  Instruction COMPARISON_INSTRUCTIONS[] = {
    OP_LESS_THAN,
    OP_LESS_THAN_EQUAL,
    OP_GREATER_THAN,
    OP_GREATER_THAN_EQUAL,
    OP_EQUAL,
    OP_NOT_EQUAL,
  };

  for(int i = 0; i < 6; i++) {
    const char* text = "1";
    Node* node = BinaryNode_new(
        COMPARISON_NODE_TYPES[i],
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
    assert(out.items[10] == COMPARISON_INSTRUCTIONS[i]);

    Node_free(node);
    InstructionList_free(&out);
  }
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
