#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "compiler.h"
#include "error.h"
#include "node.h"
#include "parser.h"

void Compiler_init(Compiler* self) {
  SymbolTable_init(&(self->symbolTable));
  SymbolList_init(&(self->symbolList));
  /*
   * We don't need to initialize hasErrors, because it's initialialized
   * in the only place that it should be used, in Compiler_compile().
   */
}

void Compiler_free(Compiler* self) {
  SymbolTable_free(&(self->symbolTable));
  SymbolList_free(&(self->symbolList));
}

inline static void Compiler_emitOp(ByteCode* out, Instruction op, size_t line) {
  ByteCode_append(out, (uint8_t)op, line);
}

inline static void Compiler_emitInt16(ByteCode* out, int16_t i, size_t line) {
  ByteCode_appendInt16(out, i, line);
}

inline static void Compiler_emitUInt16(ByteCode* out, uint16_t i, size_t line) {
  ByteCode_appendUInt16(out, i, line);
}
inline static void Compiler_emitInt32(ByteCode* out, int32_t i, size_t line) {
  ByteCode_appendInt32(out, i, line);
}

inline static void Compiler_emitInteger(ByteCode* out, AtomNode* node) {
  // TODO Handle overflows

  int32_t result = 0;

  for(size_t i = 0; i < node->length; i++) {
    result *= 10;
    result += node->text[i] - '0';
  }

  Compiler_emitOp(out, OP_INTEGER, node->node.line);
  Compiler_emitInt32(out, result, node->node.line);
}

inline static void Compiler_emitBoolean(ByteCode* out, AtomNode* node) {
  /*
   * Since there are only two possibilities which should have been matched
   * by the tokenizer, it should be sufficient to check only the first
   * character. However, we assert the full values for debug.
   */
  if(*(node->text) == 't') {
    assert(strncmp("true", node->text, node->length) == 0);
    return Compiler_emitOp(out, OP_TRUE, node->node.line);
  } else {
    assert(strncmp("false", ((AtomNode*)node)->text, ((AtomNode*)node)->length) == 0);
    return Compiler_emitOp(out, OP_FALSE, node->node.line);
  }
}

void Compiler_emitNode(Compiler* self, ByteCode* out, Node* node);

inline static void Compiler_emitBinaryNode(Compiler* self, ByteCode* out, Instruction op, Node* node) {
  Compiler_emitNode(self, out, ((BinaryNode*)node)->arg0);
  Compiler_emitNode(self, out, ((BinaryNode*)node)->arg1);
  ByteCode_append(out, op, node->line);
}

void Compiler_emitNode(Compiler* self, ByteCode* out, Node* node) {
  switch(node->type) {
    case NODE_INTEGER_LITERAL:
      return Compiler_emitInteger(out, (AtomNode*)node);

    case NODE_BOOLEAN_LITERAL:
      return Compiler_emitBoolean(out, (AtomNode*)node);

    case NODE_SYMBOL:
      {
        AtomNode* aNode = (AtomNode*)node;
        Symbol* symbol = SymbolTable_getOrCreate(&(self->symbolTable), aNode->text, aNode->length);
        int32_t index = SymbolList_find(&(self->symbolList), symbol);

        /*
         * This means the symbol wasn't found.
         */
        if(index == -1) {
          self->hasErrors = true;

          printError(
            node->line,
            "Symbol `%.*s` referenced before assignment.",
            symbol->length,
            symbol->text
          );

          return;
        }

        assert(0 <= index && index <= UINT16_MAX);
        Compiler_emitOp(out, OP_GET, node->line);
        Compiler_emitUInt16(out, index, node->line);
        return;
      }

    case NODE_ASSIGN:
      {
        BinaryNode* assignNode = (BinaryNode*)node;
        Compiler_emitNode(self, out, assignNode->arg1);

        if(assignNode->arg0->type == NODE_SYMBOL) {
          AtomNode* symbolNode = ((AtomNode*)(assignNode->arg0));
          Symbol* symbol = SymbolTable_getOrCreate(
            &(self->symbolTable),
            symbolNode->text,
            symbolNode->length
          );
          int32_t index = SymbolList_find(&(self->symbolList), symbol);

          if(index == -1) {
            SymbolList_append(
              &(self->symbolList),
              symbol,
              node->line,
              false
            );
          } else {
            if(SymbolList_isMutable(&(self->symbolList), index)) {
              Compiler_emitOp(out, OP_SET, node->line);
              Compiler_emitUInt16(out, index, node->line);
            } else {
              self->hasErrors = true;
              printError(
                node->line,
                "Reassigning immutable variable `%.*s` after definition on line %zu.",
                symbol->length,
                symbol->text,
                SymbolList_definedOnLine(&(self->symbolList), index)
              );
              return;
            }
          }
        } else {
          // TODO Handle assigning to other kinds of nodes
          assert(false);
        }

        // An assignment statement returns NIL
        Compiler_emitOp(out, OP_NIL, node->line);
      }
      return;

    case NODE_MUT_ASSIGN:
      {
        BinaryNode* assignNode = (BinaryNode*)node;
        Compiler_emitNode(self, out, assignNode->arg1);

        if(assignNode->arg0->type == NODE_SYMBOL) {
          AtomNode* symbolNode = ((AtomNode*)(assignNode->arg0));
          Symbol* symbol = SymbolTable_getOrCreate(
            &(self->symbolTable),
            symbolNode->text,
            symbolNode->length
          );
          int32_t index = SymbolList_find(&(self->symbolList), symbol);

          if(index == -1) {
            SymbolList_append(
              &(self->symbolList),
              symbol,
              node->line,
              true
            );
          } else {
            self->hasErrors = true;
            printError(
              node->line,
              "Re-declaring symbol `%.*s` already declared on line %zu.",
              symbol->length,
              symbol->text,
              SymbolList_definedOnLine(&(self->symbolList), index)
            );
          }
        } else {
          // TODO Handle assigning to other kinds of nodes
          assert(false);
        }

        // An assignment statement returns NIL
        Compiler_emitOp(out, OP_NIL, node->line);
      }
      return;

    case NODE_NEGATE:
      Compiler_emitNode(self, out, ((UnaryNode*)node)->arg0);
      return Compiler_emitOp(out, OP_NEGATE, node->line);

    case NODE_LOGICAL_NOT:
      Compiler_emitNode(self, out, ((UnaryNode*)node)->arg0);
      return Compiler_emitOp(out, OP_NOT, node->line);

    case NODE_ADD:            Compiler_emitBinaryNode(self, out, OP_ADD, node);       return;
    case NODE_SUBTRACT:       Compiler_emitBinaryNode(self, out, OP_SUBTRACT, node);  return;
    case NODE_MULTIPLY:       Compiler_emitBinaryNode(self, out, OP_MULTIPLY, node);  return;
    case NODE_INTEGER_DIVIDE: Compiler_emitBinaryNode(self, out, OP_IDIVIDE, node);   return;

    case NODE_LESS_THAN:          Compiler_emitBinaryNode(self, out, OP_LESS_THAN, node);   return;
    case NODE_LESS_THAN_EQUAL:    Compiler_emitBinaryNode(self, out, OP_LESS_THAN_EQUAL, node);   return;
    case NODE_GREATER_THAN:       Compiler_emitBinaryNode(self, out, OP_GREATER_THAN, node);   return;
    case NODE_GREATER_THAN_EQUAL: Compiler_emitBinaryNode(self, out, OP_GREATER_THAN_EQUAL, node);   return;
    case NODE_EQUAL:              Compiler_emitBinaryNode(self, out, OP_EQUAL, node);   return;
    case NODE_NOT_EQUAL:          Compiler_emitBinaryNode(self, out, OP_NOT_EQUAL, node);   return;

    case NODE_AND:
      {
        BinaryNode* bNode = (BinaryNode*)node;
        Compiler_emitNode(self, out, bNode->arg0);
        Compiler_emitOp(out, OP_JUMP_TRUE, node->line);
        Compiler_emitInt16(out, 6, node->line);
        Compiler_emitOp(out, OP_FALSE, node->line);
        Compiler_emitOp(out, OP_JUMP, node->line);
        size_t shortCircuitStart = ByteCode_count(out);
        int16_t* shortCircuitBackpatch = (int16_t*)ByteCode_pc(out, shortCircuitStart);
        Compiler_emitInt16(out, 0, node->line);
        Compiler_emitNode(self, out, bNode->arg1);
        *shortCircuitBackpatch = ByteCode_count(out) - shortCircuitStart;
        return;
      }

    case NODE_OR:
      {
        BinaryNode* bNode = (BinaryNode*)node;
        Compiler_emitNode(self, out, bNode->arg0);
        Compiler_emitOp(out, OP_JUMP_FALSE, node->line);
        Compiler_emitInt16(out, 6, node->line);
        Compiler_emitOp(out, OP_TRUE, node->line);
        Compiler_emitOp(out, OP_JUMP, node->line);
        size_t shortCircuitStart = ByteCode_count(out);
        int16_t* shortCircuitBackpatch = (int16_t*)ByteCode_pc(out, shortCircuitStart);
        Compiler_emitInt16(out, 0, node->line);
        Compiler_emitNode(self, out, bNode->arg1);
        *shortCircuitBackpatch = ByteCode_count(out) - shortCircuitStart;
        return;
      }

    case NODE_LOOP:
      {
        size_t start = ByteCode_count(out);
        Compiler_emitNode(self, out, ((UnaryNode*)node)->arg0);
        Compiler_emitOp(out, OP_DROP, node->line);
        Compiler_emitOp(out, OP_JUMP, node->line);
        // TODO Bounds-check that this fits in an int16_t
        return Compiler_emitInt16(out, start - ByteCode_count(out), node->line);
      }

    case NODE_IF:
      {
        TernaryNode* tNode = (TernaryNode*)node;
        Compiler_emitNode(self, out, tNode->arg0);
        Compiler_emitOp(out, OP_JUMP_FALSE, node->line);

        size_t ifJumpStart = ByteCode_count(out);
        int16_t* ifJumpBackpatch = (int16_t*)ByteCode_pc(out, ifJumpStart);
        Compiler_emitInt16(out, 0, node->line);

        Compiler_emitNode(self, out, tNode->arg1);

        Compiler_emitOp(out, OP_JUMP, node->line);

        size_t elseJumpStart = ByteCode_count(out);
        int16_t* elseJumpBackpatch = (int16_t*)ByteCode_pc(out, elseJumpStart);
        Compiler_emitInt16(out, 0, node->line);

        // TODO Bounds-check fits in an int16_t
        *ifJumpBackpatch = ByteCode_count(out) - ifJumpStart;

        if(tNode->arg2 == NULL) {
          Compiler_emitOp(out, OP_NIL, node->line);
        } else {
          Compiler_emitNode(self, out, tNode->arg2);
        }

        // TODO Bounds-check fits in an int16_t
        *elseJumpBackpatch = ByteCode_count(out) - elseJumpStart;

        return;
      }

    case NODE_WHILE:
    case NODE_UNTIL:
      {
        TernaryNode* tNode = (TernaryNode*)node;
        size_t beforeLoop = ByteCode_count(out);

        Compiler_emitNode(self, out, tNode->arg0);

        if(node->type == NODE_WHILE) {
          Compiler_emitOp(out, OP_JUMP_FALSE, node->line);
        } else if(node->type == NODE_UNTIL) {
          Compiler_emitOp(out, OP_JUMP_TRUE, node->line);
        } else {
          assert(false);
        }

        size_t loopJumpStart = ByteCode_count(out);
        int16_t* loopJumpBackpatch = (int16_t*)ByteCode_pc(out, loopJumpStart);
        Compiler_emitInt16(out, 0, node->line);

        Compiler_emitNode(self, out, tNode->arg1);
        Compiler_emitOp(out, OP_DROP, node->line);

        // TODO Bounds-check fits in an int16_t
        Compiler_emitOp(out, OP_JUMP, node->line);
        Compiler_emitInt16(out, beforeLoop - ByteCode_count(out), node->line);

        // TODO Bounds-check fits in an int16_t
        *loopJumpBackpatch = ByteCode_count(out) - loopJumpStart;

        if(tNode->arg2 == NULL) {
          Compiler_emitOp(out, OP_NIL, node->line);
        } else {
          Compiler_emitNode(self, out, tNode->arg2);
        }

        return;
      }

    case NODE_ERROR:
    case NODE_EOF:
      assert(false);
  }
}

bool Compiler_compile(Compiler* self, ByteCode* out, Parser* parser) {
  Node* statement = Parser_parseStatement(parser);
  self->hasErrors = false;

  /*
   * Take some checkpoints so we can back out what we've emitted if there are
   * errors point if there are errors.
   */
  size_t byteCodeCheckpoint = ByteCode_count(out);
  size_t symbolListCheckpoint = SymbolList_count(&(self->symbolList));

  if(statement->type == NODE_EOF) {
    /*
     * This is so that empty files or repl lines emit a return value, since
     * callers expect this.
     */
    Compiler_emitOp(out, OP_NIL, statement->line);
    Compiler_emitOp(out, OP_RETURN, statement->line);
    return true;
  } else if(statement->type == NODE_ERROR) {
    ErrorNode_print(statement);
    self->hasErrors = true;
  } else {
    Compiler_emitNode(self, out, statement);
  }

  Node_free(statement);

  while((statement = Parser_parseStatement(parser))->type != NODE_EOF) {
    if(statement->type == NODE_ERROR) {
      ErrorNode_print(statement);
      self->hasErrors = true;
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
      Compiler_emitNode(self, out, statement);
    }

    Node_free(statement);
  }

  /*
   * Rewind the emitted code if there were errors.
   */
  if(self->hasErrors) {
    ByteCode_rewind(out, byteCodeCheckpoint);
    SymbolList_rewind(&(self->symbolList), symbolListCheckpoint);
  } else {
    Compiler_emitOp(out, OP_RETURN, statement->line);
  }

  return !(self->hasErrors);
}

#ifdef TEST

void test_Compiler_emitNode_emitsIntegerLiteral() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "42";
  Node* node = AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 2);
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 5);
  assert(out.items[0] == OP_INTEGER);
  assert(*(int32_t*)(out.items + 1) == 42);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_emitsNegate() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "42";
  Node* node = UnaryNode_new(
      NODE_NEGATE,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 2)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 6);
  assert(out.items[0] == OP_INTEGER);
  assert(*(int32_t*)(out.items + 1) == 42);
  assert(out.items[5] == OP_NEGATE);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_emitsNot() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "true";
  Node* node = UnaryNode_new(
      NODE_LOGICAL_NOT,
      1,
      AtomNode_new(NODE_BOOLEAN_LITERAL, 1, text, 4)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 2);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_NOT);

  Node_free(node);
  ByteCode_free(&out);

  Compiler_free(&compiler);
}

void test_Compiler_emitNode_emitsAdd() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "1";
  Node* node = BinaryNode_new(
      NODE_ADD,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 11);
  assert(out.items[0] == OP_INTEGER);
  assert(out.items[5] == OP_INTEGER);
  assert(out.items[10] == OP_ADD);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_emitsSubtract() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "1";
  Node* node = BinaryNode_new(
      NODE_SUBTRACT,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 11);
  assert(out.items[0] == OP_INTEGER);
  assert(out.items[5] == OP_INTEGER);
  assert(out.items[10] == OP_SUBTRACT);

  Node_free(node);
  ByteCode_free(&out);

  Compiler_free(&compiler);
}

void test_Compiler_emitNode_emitsMultiply() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "1";
  Node* node = BinaryNode_new(
      NODE_MULTIPLY,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 11);
  assert(out.items[0] == OP_INTEGER);
  assert(out.items[5] == OP_INTEGER);
  assert(out.items[10] == OP_MULTIPLY);

  Node_free(node);
  ByteCode_free(&out);

  Compiler_free(&compiler);
}

void test_Compiler_emitNode_emitsIntegerDivide() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "1";
  Node* node = BinaryNode_new(
      NODE_INTEGER_DIVIDE,
      1,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 1)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 11);
  assert(out.items[0] == OP_INTEGER);
  assert(out.items[5] == OP_INTEGER);
  assert(out.items[10] == OP_IDIVIDE);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_emitsComparisons() {
  Compiler compiler;
  Compiler_init(&compiler);

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
    ByteCode out;
    ByteCode_init(&out);

    Compiler_emitNode(&compiler, &out, node);

    assert(out.count == 11);
    assert(out.items[0] == OP_INTEGER);
    assert(out.items[5] == OP_INTEGER);
    assert(out.items[10] == COMPARISON_INSTRUCTIONS[i]);

    Node_free(node);
    ByteCode_free(&out);
  }

  Compiler_free(&compiler);
}

void test_Compiler_emitNode_emitsAndOr() {
  Compiler compiler;
  Compiler_init(&compiler);

  NodeType LOGICAL_NODE_TYPES[] = {
    NODE_AND,
    NODE_OR,
  };

  Instruction JUMP_INSTRUCTIONS[] = {
    OP_JUMP_TRUE,
    OP_JUMP_FALSE,
  };

  Instruction SHORT_CIRCUIT_INSTRUCTIONS[] = {
    OP_FALSE,
    OP_TRUE,
  };

  for(int i = 0; i < 2; i++) {
    Node* node = BinaryNode_new(
        LOGICAL_NODE_TYPES[i],
        1,
        AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
        AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "false", 5)
    );
    ByteCode out;
    ByteCode_init(&out);

    Compiler_emitNode(&compiler, &out, node);

    assert(out.count == 9);
    assert(out.items[0] == OP_TRUE);
    assert(out.items[1] == JUMP_INSTRUCTIONS[i]);
    assert(*((int16_t*)(out.items + 2)) == 6);
    assert(out.items[4] == SHORT_CIRCUIT_INSTRUCTIONS[i]);
    assert(out.items[5] == OP_JUMP);
    assert(*((int16_t*)(out.items + 6)) == 3);
    assert(out.items[8] == OP_FALSE);

    Node_free(node);
    ByteCode_free(&out);
  }

  Compiler_free(&compiler);
}

void test_Compiler_emitNode_loop() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "42";
  Node* node = UnaryNode_new(
    NODE_LOOP,
    1,
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text, 2)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 9);
  assert(out.items[0] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 1)) == 42);
  assert(out.items[5] == OP_DROP);
  assert(out.items[6] == OP_JUMP);
  assert(*((int16_t*)(out.items + 7)) == -7);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_if() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text0 = "true";
  const char* text1 = "42";
  Node* node = TernaryNode_new(
    NODE_IF,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, text0, 4),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text1, 2),
    NULL
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 13);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_JUMP_FALSE);
  assert(*((int16_t*)(out.items + 2)) == 10);
  assert(out.items[4] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 5)) == 42);
  assert(out.items[9] == OP_JUMP);
  assert(*((int16_t*)(out.items + 10)) == 3);
  assert(out.items[12] == OP_NIL);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_ifElse() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text0 = "true";
  const char* text1 = "42";
  const char* text2 = "37";
  Node* node = TernaryNode_new(
    NODE_IF,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, text0, 4),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text1, 2),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text2, 2)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 17);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_JUMP_FALSE);
  assert(*((int16_t*)(out.items + 2)) == 10);
  assert(out.items[4] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 5)) == 42);
  assert(out.items[9] == OP_JUMP);
  assert(*((int16_t*)(out.items + 10)) == 7);
  assert(out.items[12] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 13)) == 37);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_while() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text0 = "true";
  const char* text1 = "42";
  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, text0, 4),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text1, 2),
    NULL
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 14);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_JUMP_FALSE);
  assert(*((int16_t*)(out.items + 2)) == 11);
  assert(out.items[4] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 5)) == 42);
  assert(out.items[9] == OP_DROP);
  assert(out.items[10] == OP_JUMP);
  assert(*((int16_t*)(out.items + 11)) == -11);
  assert(out.items[13] == OP_NIL);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_whileElse() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text0 = "true";
  const char* text1 = "42";
  const char* text2 = "37";
  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, text0, 4),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text1, 2),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text2, 2)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 18);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_JUMP_FALSE);
  assert(*((int16_t*)(out.items + 2)) == 11);
  assert(out.items[4] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 5)) == 42);
  assert(out.items[9] == OP_DROP);
  assert(out.items[10] == OP_JUMP);
  assert(*((int16_t*)(out.items + 11)) == -11);
  assert(out.items[13] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 14)) == 37);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_until() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text0 = "true";
  const char* text1 = "42";
  Node* node = TernaryNode_new(
    NODE_UNTIL,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, text0, 4),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text1, 2),
    NULL
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 14);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_JUMP_TRUE);
  assert(*((int16_t*)(out.items + 2)) == 11);
  assert(out.items[4] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 5)) == 42);
  assert(out.items[9] == OP_DROP);
  assert(out.items[10] == OP_JUMP);
  assert(*((int16_t*)(out.items + 11)) == -11);
  assert(out.items[13] == OP_NIL);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_untilElse() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text0 = "true";
  const char* text1 = "42";
  const char* text2 = "37";
  Node* node = TernaryNode_new(
    NODE_UNTIL,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, text0, 4),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text1, 2),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, text2, 2)
  );
  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(out.count == 18);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_JUMP_TRUE);
  assert(*((int16_t*)(out.items + 2)) == 11);
  assert(out.items[4] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 5)) == 42);
  assert(out.items[9] == OP_DROP);
  assert(out.items[10] == OP_JUMP);
  assert(*((int16_t*)(out.items + 11)) == -11);
  assert(out.items[13] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 14)) == 37);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_compile_emitsVariableInstructions() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "answer = 42; answer;";
  Parser parser;
  Parser_init(&parser, text, false);

  ByteCode out;
  ByteCode_init(&out);

  bool success = Compiler_compile(&compiler, &out, &parser);

  assert(success);
  assert(out.count == 11);
  assert(out.items[0] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 1)) == 42);
  assert(out.items[5] == OP_NIL);
  assert(out.items[6] == OP_DROP);
  assert(out.items[7] == OP_GET);
  assert(*((uint16_t*)(out.items + 8)) == 0);
  assert(out.items[10] == OP_RETURN);

  Parser_free(&parser);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_compile_emitsNilOnEmptyInput() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = "";
  Parser parser;
  Parser_init(&parser, text, false);

  ByteCode out;
  ByteCode_init(&out);

  bool success = Compiler_compile(&compiler, &out, &parser);

  assert(success);
  assert(out.count == 2);
  assert(out.items[0] == OP_NIL);
  assert(out.items[1] == OP_RETURN);

  Parser_free(&parser);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_compile_emitsNilOnBlankInput() {
  Compiler compiler;
  Compiler_init(&compiler);

  const char* text = " \t \n \r";
  Parser parser;
  Parser_init(&parser, text, false);

  ByteCode out;
  ByteCode_init(&out);

  bool success = Compiler_compile(&compiler, &out, &parser);

  assert(success);
  assert(out.count == 2);
  assert(out.items[0] == OP_NIL);
  assert(out.items[1] == OP_RETURN);

  Parser_free(&parser);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

#endif
