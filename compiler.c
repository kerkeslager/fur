#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "builtins.h"
#include "compiler.h"
#include "error.h"
#include "node.h"
#include "parser.h"

void Compiler_init(Compiler* self) {
  SymbolTable_init(&(self->symbolTable));
  SymbolList_init(&(self->symbolList));
  self->breaks = NULL;
  self->breakCount = 0;
  self->breakCapacity = 0;

  /*
   * hasErrors is initialized in Compiler_compile because we want it to be
   * reset each time Compiler_compile is called. We also reset breakCount but
   * we don't reset breaks because we don't want to reallocate if it's
   * already allocated.
   */
}

void Compiler_free(Compiler* self) {
  SymbolTable_free(&(self->symbolTable));
  SymbolList_free(&(self->symbolList));

  if(self->breaks != NULL) free(self->breaks);
}

void Compiler_emitBreak(Compiler* self, ByteCode* out, size_t breakDepth) {
  Break b;
  b.depth = breakDepth;
  b.index = ByteCode_count(out);

  if(self->breakCount == self->breakCapacity) {
    if(self->breakCapacity == 0) {
      self->breakCapacity = 4;
    } else {
      self->breakCapacity *= 2;
    }

    self->breaks = realloc(self->breaks, self->breakCapacity * sizeof(Break));
    assert(self->breaks != NULL); // TODO Handle this better
  }

  self->breaks[self->breakCount++] = b;
}

void Compiler_patchBreaks(Compiler* self, ByteCode* out) {
  size_t currentIndex = ByteCode_count(out);
  size_t patchedCount = 0;

  for(size_t i = 0; i < self->breakCount; i++) {
    self->breaks[i].depth--;

    if(self->breaks[i].depth == 0) {
      size_t breakIndex = self->breaks[i].index;

      // TODO Prevent overflowing int16_t
      *((int16_t*)ByteCode_pc(out, breakIndex)) = currentIndex - breakIndex;

      patchedCount++;
    } else if(patchedCount) {
      // Clean up breaks which have already been patched
      self->breaks[i - patchedCount] = self->breaks[i];
    }
  }

  self->breakCount -= patchedCount;
}

inline static void Compiler_emitOp(ByteCode* out, Instruction op, size_t line) {
  ByteCode_append(out, (uint8_t)op, line);
}

inline static void Compiler_emitInt16(ByteCode* out, int16_t i, size_t line) {
  ByteCode_appendInt16(out, i, line);
}

inline static void Compiler_emitUInt8(ByteCode* out, uint8_t i, size_t line) {
  ByteCode_append(out, i, line);
}

inline static void Compiler_emitUInt16(ByteCode* out, uint16_t i, size_t line) {
  ByteCode_appendUInt16(out, i, line);
}
inline static void Compiler_emitInt32(ByteCode* out, int32_t i, size_t line) {
  ByteCode_appendInt32(out, i, line);
}

inline static uint64_t Compiler_atomNodeToInteger(Node* node) {
  assert(node->type == NODE_INTEGER_LITERAL);

  uint64_t result = 0;

  for(size_t i = 0; i < ((AtomNode*)node)->length; i++) {
    // TODO Handle this better
    assert(result <= UINT64_MAX / 10);

    result *= 10;

    uint8_t digit = ((AtomNode*)node)->text[i] - '0';

    // TODO Handle this better
    assert(result <= UINT64_MAX - digit);

    result += digit;
  }

  return result;
}

inline static void Compiler_emitInteger(ByteCode* out, Node* node) {
  int64_t result = Compiler_atomNodeToInteger(node);

  // TODO Handle this better
  assert(result <= INT32_MAX);

  Compiler_emitOp(out, OP_INTEGER, node->line);
  Compiler_emitInt32(out, result, node->line);
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

static inline void Compiler_openScope(Compiler* self, ByteCode* out, Node* node, ScopeType type) {
  SymbolList_openScope(&(self->symbolList), type, ByteCode_count(out));
  Compiler_emitOp(out, OP_SCOPE_OPEN, node->line);
}

static inline void Compiler_closeScope(Compiler* self, ByteCode* out, Node* node) {
  Compiler_emitOp(out, OP_SCOPE_CLOSE, node->line);
  SymbolList_closeScope(&(self->symbolList));
}

inline static void Compiler_emitBinaryNode(Compiler* self, ByteCode* out, Instruction op, Node* node) {
  Compiler_emitNode(self, out, ((BinaryNode*)node)->arg0);
  Compiler_emitNode(self, out, ((BinaryNode*)node)->arg1);
  ByteCode_append(out, op, node->line);
}

inline static bool isComparison(Node* node) {
  switch(node->type) {
    case NODE_LESS_THAN:
    case NODE_LESS_THAN_EQUAL:
    case NODE_GREATER_THAN:
    case NODE_GREATER_THAN_EQUAL:
    case NODE_EQUAL:
    case NODE_NOT_EQUAL:
      return true;

    default:
      return false;
  }
}

inline static void Compiler_emitComparison(Compiler* self, ByteCode* out, Instruction op, BinaryNode* node) {
  if(!isComparison(node->arg0)) {
    return Compiler_emitBinaryNode(self, out, op, (Node*)node);
  }

  BinaryNode* stack[256];
  int stackDepth = 0;

  BinaryNode* b = node;

  while(isComparison(b->arg0)) {
    if(stackDepth > 256) {
      self->hasErrors = true;

      printError(
        node->node.line,
        "Cannot chain more than 256 comparison operators."
      );

      return;
    }

    stack[stackDepth++] = b;
    b = (BinaryNode*)(b->arg0);
  }

  Compiler_emitNode(self, out, b->arg0);
  Compiler_emitNode(self, out, b->arg1);

  size_t shortCircuitStarts[255];
  uint8_t shortCircuitStartCount = 0;

  while(stackDepth > 0) {
    // TODO We can probably optimize this by making it one instruction
    Compiler_emitOp(out, OP_DUP, b->node.line);
    Compiler_emitOp(out, OP_ROT3, b->node.line);

    switch(b->node.type) {
      case NODE_LESS_THAN:
        Compiler_emitOp(out, OP_LESS_THAN, b->node.line);
        break;
      case NODE_LESS_THAN_EQUAL:
        Compiler_emitOp(out, OP_LESS_THAN_EQUAL, b->node.line);
        break;
      case NODE_GREATER_THAN:
        Compiler_emitOp(out, OP_GREATER_THAN, b->node.line);
        break;
      case NODE_GREATER_THAN_EQUAL:
        Compiler_emitOp(out, OP_GREATER_THAN_EQUAL, b->node.line);
        break;
      case NODE_EQUAL:
        Compiler_emitOp(out, OP_EQUAL, b->node.line);
        break;
      case NODE_NOT_EQUAL:
        Compiler_emitOp(out, OP_NOT_EQUAL, b->node.line);
        break;
      default:
        assert(false);
    }

    Compiler_emitOp(out, OP_JUMP_FALSE, b->node.line);
    shortCircuitStarts[shortCircuitStartCount++] = ByteCode_count(out);
    Compiler_emitInt16(out, 0, b->node.line);

    stackDepth--;
    b = stack[stackDepth];

    Compiler_emitNode(self, out, b->arg1);
  }

  switch(b->node.type) {
    case NODE_LESS_THAN:
      Compiler_emitOp(out, OP_LESS_THAN, b->node.line);
      break;
    case NODE_LESS_THAN_EQUAL:
      Compiler_emitOp(out, OP_LESS_THAN_EQUAL, b->node.line);
      break;
    case NODE_GREATER_THAN:
      Compiler_emitOp(out, OP_GREATER_THAN, b->node.line);
      break;
    case NODE_GREATER_THAN_EQUAL:
      Compiler_emitOp(out, OP_GREATER_THAN_EQUAL, b->node.line);
      break;
    case NODE_EQUAL:
      Compiler_emitOp(out, OP_EQUAL, b->node.line);
      break;
    case NODE_NOT_EQUAL:
      Compiler_emitOp(out, OP_NOT_EQUAL, b->node.line);
      break;
    default:
      assert(false);
  }

  Compiler_emitOp(out, OP_JUMP, b->node.line);
  Compiler_emitInt16(out, 4, b->node.line);

  size_t shortCircuitEnd = ByteCode_count(out);
  Compiler_emitOp(out, OP_DROP, b->node.line);
  Compiler_emitOp(out, OP_FALSE, b->node.line);

  for(uint8_t i = 0; i < shortCircuitStartCount; i++) {
    *(ByteCode_pc(out, shortCircuitStarts[i]))
      = shortCircuitEnd - shortCircuitStarts[i];
  }
}

void Compiler_emitBlock(Compiler* self, ByteCode* out, Node* node, bool isScoped) {
  ListNode* block = (ListNode*)node;

  if(isScoped) Compiler_openScope(self, out, node, SCOPE_GENERIC);

  for(size_t i = 0; i < block->count; i++) {
    Compiler_emitNode(self, out, block->items[i]);

    if(i < block->count - 1) {
      Compiler_emitOp(out, OP_DROP, node->line);
    }
  }

  if(isScoped) Compiler_closeScope(self, out, node);
}

void Compiler_emitNode(Compiler* self, ByteCode* out, Node* node) {
  switch(node->type) {
    case NODE_INTEGER_LITERAL:
      return Compiler_emitInteger(out, node);

    case NODE_NIL_LITERAL:
      return Compiler_emitOp(out, OP_NIL, node->line);

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
          index = Builtin_index(aNode->text, aNode->length);

          if(index != -1) {
            Compiler_emitOp(out, OP_BUILTIN, node->line);
            Compiler_emitUInt8(out, index, node->line);
            return;
          }

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

    case NODE_ADD:
      return Compiler_emitBinaryNode(self, out, OP_ADD, node);
    case NODE_SUBTRACT:
      return Compiler_emitBinaryNode(self, out, OP_SUBTRACT, node);
    case NODE_MULTIPLY:
      return Compiler_emitBinaryNode(self, out, OP_MULTIPLY, node);
    case NODE_INTEGER_DIVIDE:
      return Compiler_emitBinaryNode(self, out, OP_IDIVIDE, node);

    case NODE_LESS_THAN:
      return Compiler_emitComparison(self, out, OP_LESS_THAN, (BinaryNode*)node);
    case NODE_LESS_THAN_EQUAL:
      return Compiler_emitComparison(self, out, OP_LESS_THAN_EQUAL, (BinaryNode*)node);
    case NODE_GREATER_THAN:
      return Compiler_emitComparison(self, out, OP_GREATER_THAN, (BinaryNode*)node);
    case NODE_GREATER_THAN_EQUAL:
      return Compiler_emitComparison(self, out, OP_GREATER_THAN_EQUAL, (BinaryNode*)node);
    case NODE_EQUAL:
      return Compiler_emitComparison(self, out, OP_EQUAL, (BinaryNode*)node);
    case NODE_NOT_EQUAL:
      return Compiler_emitComparison(self, out, OP_NOT_EQUAL, (BinaryNode*)node);

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

    case NODE_BLOCK:
      Compiler_emitBlock(self, out, node, true);
      return;

    case NODE_LOOP:
      {
        size_t start = ByteCode_count(out);

        Compiler_openScope(self, out, node, SCOPE_BREAKABLE);
        Compiler_emitNode(self, out, ((UnaryNode*)node)->arg0);
        Compiler_closeScope(self, out, node);

        Compiler_emitOp(out, OP_DROP, node->line);
        Compiler_emitOp(out, OP_JUMP, node->line);

        Compiler_patchBreaks(self, out);

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

        Compiler_openScope(self, out, node, SCOPE_GENERIC);
        Compiler_emitNode(self, out, tNode->arg1);
        Compiler_closeScope(self, out, node);

        Compiler_emitOp(out, OP_JUMP, node->line);

        size_t elseJumpStart = ByteCode_count(out);
        int16_t* elseJumpBackpatch = (int16_t*)ByteCode_pc(out, elseJumpStart);
        Compiler_emitInt16(out, 0, node->line);

        // TODO Bounds-check fits in an int16_t
        *ifJumpBackpatch = ByteCode_count(out) - ifJumpStart;

        if(tNode->arg2 == NULL) {
          Compiler_emitOp(out, OP_NIL, node->line);
        } else {
          Compiler_openScope(self, out, node, SCOPE_GENERIC);
          Compiler_emitNode(self, out, tNode->arg2);
          Compiler_closeScope(self, out, node);
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

        Compiler_openScope(self, out, node, SCOPE_BREAKABLE);
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
        Compiler_closeScope(self, out, node);

        Compiler_emitOp(out, OP_DROP, node->line);

        // TODO Bounds-check fits in an int16_t
        Compiler_emitOp(out, OP_JUMP, node->line);
        Compiler_emitInt16(out, beforeLoop - ByteCode_count(out), node->line);

        // TODO Bounds-check fits in an int16_t
        *loopJumpBackpatch = ByteCode_count(out) - loopJumpStart;

        if(tNode->arg2 == NULL) {
          Compiler_emitOp(out, OP_NIL, node->line);
        } else {
          Compiler_openScope(self, out, node, SCOPE_GENERIC);
          Compiler_emitNode(self, out, tNode->arg2);
          Compiler_closeScope(self, out, node);
        }

        Compiler_patchBreaks(self, out);

        return;
      }

    case NODE_CONTINUE:
      {
        UnaryNode* uNode = (UnaryNode*)node;

        /*
         * TODO
         * This is a hack to deal with the fact that everything assumes the
         * loop body will return a value. Notably, we emit an OP_DROP
         * after exiting the scope to clear any lingering variable that was
         * retained by OP_SCOPE_CLOSE. But if no variables were defined
         * within the scope, that OP_DROP would drop a value too far.
         *
         * Ideally we wouldn't push a nil onto the stack just to drop it.
         */
        Compiler_emitOp(out, OP_NIL, node->line);

        size_t continueDepth = 1; // continue the current loop by default

        if(uNode->arg0 != NULL) {
          assert(uNode->arg0->type == NODE_INTEGER_LITERAL);

          continueDepth = Compiler_atomNodeToInteger(uNode->arg0);

          // TODO Handle this better
          assert(continueDepth < 256);

          // TODO Handle this better
          assert(continueDepth > 0);
        }

        size_t brokenCount = 0;
        int i = (int)(self->symbolList.scopeDepth) - 1;

        for(; i >= 0; i--) {
          /*
           * We don't call Compiler_closeScope() because we are still
           * syntactically inside the loop. We're emitting an OP_SCOPE_CLOSE
           * because the program counter is leaving the scope.
           *
           * We have to close all scopes, not just the breakable (loop) scopes,
           * because we may be inside another scope such as an if scope.
           */
          /*
           * TODO
           * Rather than emit multiple OP_SCOPE_CLOSEs, can we emit 1 that
           * takes an argument?
           */
          Compiler_emitOp(out, OP_SCOPE_CLOSE, node->line);

          if(self->symbolList.scopes[i].type == SCOPE_BREAKABLE) {
            brokenCount++;
            if(brokenCount == continueDepth) break;
          }
        }

        assert(i >= 0);

        /*
         * TODO
         * See earlier comment about emitting OP_NIL just so we can
         * drop it here.
         */
        Compiler_emitOp(out, OP_DROP, node->line);
        Compiler_emitOp(out, OP_JUMP, node->line);
        Compiler_emitInt16(
          out,
          self->symbolList.scopes[i].start - ByteCode_count(out),
          node->line
        );
      }
      return;

    case NODE_BREAK:
      {
        // break; break 1 loop, with return nil
        // break 2; break 2 loops, with return nil
        // break with "Hello"; break 1 loop, with return "Hello"
        // break 2 with "Hello"; break 2 loops, with return "hello"
        BinaryNode* bNode = (BinaryNode*)node;

        if(bNode->arg1 == NULL) {
          Compiler_emitOp(out, OP_NIL, node->line);
        } else {
          Compiler_emitNode(self, out, bNode->arg1);
        }

        size_t breakDepth = 1; // break out of one loop by default
        if(bNode->arg0 != NULL) {
          // TODO Handle this better
          assert(bNode->arg0->type == NODE_INTEGER_LITERAL);

          breakDepth = Compiler_atomNodeToInteger(bNode->arg0);

          // TODO Handle this better
          assert(breakDepth < 256);

          // TODO Handle this better
          assert(breakDepth > 0);
        }

        /*
         * TODO
         * Wrap scopeDepth so we aren't directly accessing symbolList's
         * fields.
         */

        size_t brokenCount = 0;
        for(int i = (int)(self->symbolList.scopeDepth) - 1; i >= 0; i--) {
          /*
           * We don't call Compiler_closeScope() because we are still
           * syntactically inside the loop. We're emitting an OP_SCOPE_CLOSE
           * because the program counter is leaving the scope.
           *
           * We have to close all scopes, not just the breakable (loop) scopes,
           * because we may be inside another scope such as an if scope.
           */
          /*
           * TODO
           * Rather than emit multiple OP_SCOPE_CLOSEs, can we emit 1 that
           * takes an argument?
           */
          Compiler_emitOp(out, OP_SCOPE_CLOSE, node->line);

          if(self->symbolList.scopes[i].type == SCOPE_BREAKABLE) {
            brokenCount++;
            if(brokenCount == breakDepth) break;
          }
        }

        Compiler_emitOp(out, OP_JUMP, node->line);
        Compiler_emitBreak(self, out, breakDepth);
        Compiler_emitInt16(out, 0, node->line);

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
  self->breakCount = 0;

  /*
   * Take some checkpoints so we can back out what we've emitted if there
   * are errors.
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

  assert(out.count == 11);
  assert(out.items[0] == OP_SCOPE_OPEN);
  assert(out.items[1] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 2)) == 42);
  assert(out.items[6] == OP_SCOPE_CLOSE);
  assert(out.items[7] == OP_DROP);
  assert(out.items[8] == OP_JUMP);
  assert(*((int16_t*)(out.items + 9)) == -9);

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

  assert(out.count == 15);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_JUMP_FALSE);
  assert(*((int16_t*)(out.items + 2)) == 12);
  assert(out.items[4] == OP_SCOPE_OPEN);
  assert(out.items[5] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 6)) == 42);
  assert(out.items[10] == OP_SCOPE_CLOSE);
  assert(out.items[11] == OP_JUMP);
  assert(*((int16_t*)(out.items + 12)) == 3);
  assert(out.items[14] == OP_NIL);

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

  assert(out.count == 21);
  assert(out.items[0] == OP_TRUE);
  assert(out.items[1] == OP_JUMP_FALSE);
  assert(*((int16_t*)(out.items + 2)) == 12);
  assert(out.items[4] == OP_SCOPE_OPEN);
  assert(out.items[5] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 6)) == 42);
  assert(out.items[10] == OP_SCOPE_CLOSE);
  assert(out.items[11] == OP_JUMP);
  assert(*((int16_t*)(out.items + 12)) == 9);
  assert(out.items[14] == OP_SCOPE_OPEN);
  assert(out.items[15] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 16)) == 37);
  assert(out.items[20] == OP_SCOPE_CLOSE);

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

  assert(out.count == 16);
  assert(out.items[0] == OP_SCOPE_OPEN);
  assert(out.items[1] == OP_TRUE);
  assert(out.items[2] == OP_JUMP_FALSE);
  assert(*((int16_t*)(out.items + 3)) == 12);
  assert(out.items[5] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 6)) == 42);
  assert(out.items[10] == OP_SCOPE_CLOSE);
  assert(out.items[11] == OP_DROP);
  assert(out.items[12] == OP_JUMP);
  assert(*((int16_t*)(out.items + 13)) == -13);
  assert(out.items[15] == OP_NIL);

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

  assert(out.count == 22);
  assert(out.items[0] == OP_SCOPE_OPEN);
  assert(out.items[1] == OP_TRUE);
  assert(out.items[2] == OP_JUMP_FALSE);
  assert(*((int16_t*)(out.items + 3)) == 12);
  assert(out.items[5] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 6)) == 42);
  assert(out.items[10] == OP_SCOPE_CLOSE);
  assert(out.items[11] == OP_DROP);
  assert(out.items[12] == OP_JUMP);
  assert(*((int16_t*)(out.items + 13)) == -13);
  assert(out.items[15] == OP_SCOPE_OPEN);
  assert(out.items[16] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 17)) == 37);
  assert(out.items[21] == OP_SCOPE_CLOSE);

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

  assert(out.count == 16);
  assert(out.items[0] == OP_SCOPE_OPEN);
  assert(out.items[1] == OP_TRUE);
  assert(out.items[2] == OP_JUMP_TRUE);
  assert(*((int16_t*)(out.items + 3)) == 12);
  assert(out.items[5] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 6)) == 42);
  assert(out.items[10] == OP_SCOPE_CLOSE);
  assert(out.items[11] == OP_DROP);
  assert(out.items[12] == OP_JUMP);
  assert(*((int16_t*)(out.items + 13)) == -13);
  assert(out.items[15] == OP_NIL);

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

  assert(out.count == 22);
  assert(out.items[0] == OP_SCOPE_OPEN);
  assert(out.items[1] == OP_TRUE);
  assert(out.items[2] == OP_JUMP_TRUE);
  assert(*((int16_t*)(out.items + 3)) == 12);
  assert(out.items[5] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 6)) == 42);
  assert(out.items[10] == OP_SCOPE_CLOSE);
  assert(out.items[11] == OP_DROP);
  assert(out.items[12] == OP_JUMP);
  assert(*((int16_t*)(out.items + 13)) == -13);
  assert(out.items[15] == OP_SCOPE_OPEN);
  assert(out.items[16] == OP_INTEGER);
  assert(*((int32_t*)(out.items + 17)) == 37);
  assert(out.items[21] == OP_SCOPE_CLOSE);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_loopContinue() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = UnaryNode_new(
    NODE_LOOP,
    1,
    UnaryNode_new(
      NODE_CONTINUE,
      1,
      NULL
    )
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(ByteCode_count(&out) == 12);
  assert(out.items[0] == OP_SCOPE_OPEN);
  assert(out.items[1] == OP_NIL);
  assert(out.items[2] == OP_SCOPE_CLOSE);
  assert(out.items[3] == OP_DROP);
  assert(out.items[4] == OP_JUMP);
  assert(*((int16_t*)(out.items + 5)) == -5);
  assert(out.items[7] == OP_SCOPE_CLOSE);
  assert(out.items[8] == OP_DROP);
  assert(out.items[9] == OP_JUMP);
  assert(*((int16_t*)(out.items + 10)) == -10);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_loopContinueTo() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = UnaryNode_new(
    NODE_LOOP,
    1,
    UnaryNode_new(
      NODE_LOOP,
      1,
      UnaryNode_new(
        NODE_LOOP,
        1,
        UnaryNode_new(
          NODE_CONTINUE,
          1,
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "2", 1)
        )
      )
    )
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  assert(ByteCode_count(&out) == 25);
  assert(out.items[0] == OP_SCOPE_OPEN);
  assert(out.items[1] == OP_SCOPE_OPEN);
  assert(out.items[2] == OP_SCOPE_OPEN);
  assert(out.items[3] == OP_NIL);
  assert(out.items[4] == OP_SCOPE_CLOSE);
  assert(out.items[5] == OP_SCOPE_CLOSE);
  assert(out.items[6] == OP_DROP);
  assert(out.items[7] == OP_JUMP);
  assert(*((int16_t*)(out.items + 8)) == -7);
  assert(out.items[10] == OP_SCOPE_CLOSE);
  assert(out.items[11] == OP_DROP);
  assert(out.items[12] == OP_JUMP);
  assert(*((int16_t*)(out.items + 13)) == -11);
  assert(out.items[15] == OP_SCOPE_CLOSE);
  assert(out.items[16] == OP_DROP);
  assert(out.items[17] == OP_JUMP);
  assert(*((int16_t*)(out.items + 18)) == -17);
  assert(out.items[20] == OP_SCOPE_CLOSE);
  assert(out.items[21] == OP_DROP);
  assert(out.items[22] == OP_JUMP);
  assert(*((int16_t*)(out.items + 23)) == -23);

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);
}

void test_Compiler_emitNode_whileContinue() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    UnaryNode_new(
      NODE_CONTINUE,
      1,
      NULL
    ),
    NULL
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileContinueTo() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    TernaryNode_new(
      NODE_WHILE,
      1,
      AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
      TernaryNode_new(
        NODE_WHILE,
        1,
        AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
        UnaryNode_new(
          NODE_CONTINUE,
          1,
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "2", 1)
        ),
        NULL
      ),
      NULL
    ),
    NULL
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileElseContinue() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    UnaryNode_new(
      NODE_CONTINUE,
      1,
      NULL
    ),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileElseContinueTo() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    TernaryNode_new(
      NODE_WHILE,
      1,
      AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
      TernaryNode_new(
        NODE_WHILE,
        1,
        AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
        UnaryNode_new(
          NODE_CONTINUE,
          1,
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "2", 1)
        ),
        AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
      ),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, "43", 2)
    ),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, "44", 2)
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_loopBreak() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = UnaryNode_new(
    NODE_LOOP,
    1,
    BinaryNode_new(
      NODE_BREAK,
      1,
      NULL,
      NULL
    )
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_loopBreakTo() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = UnaryNode_new(
    NODE_LOOP,
    1,
    UnaryNode_new(
      NODE_LOOP,
      1,
      UnaryNode_new(
        NODE_LOOP,
        1,
        BinaryNode_new(
          NODE_BREAK,
          1,
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "2", 1),
          NULL
        )
      )
    )
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_loopBreakWith() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = UnaryNode_new(
    NODE_LOOP,
    1,
    BinaryNode_new(
      NODE_BREAK,
      1,
      NULL,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
    )
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_loopBreakToWith() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = UnaryNode_new(
    NODE_LOOP,
    1,
    UnaryNode_new(
      NODE_LOOP,
      1,
      UnaryNode_new(
        NODE_LOOP,
        1,
        BinaryNode_new(
          NODE_BREAK,
          1,
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "2", 1),
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
        )
      )
    )
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileBreak() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    BinaryNode_new(
      NODE_BREAK,
      1,
      NULL,
      NULL
    ),
    NULL
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileBreakTo() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    TernaryNode_new(
      NODE_WHILE,
      1,
      AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
      TernaryNode_new(
        NODE_WHILE,
        1,
        AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
        BinaryNode_new(
          NODE_BREAK,
          1,
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "2", 1),
          NULL
        ),
        NULL
      ),
      NULL
    ),
    NULL
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileBreakWith() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    BinaryNode_new(
      NODE_BREAK,
      1,
      NULL,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
    ),
    NULL
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileBreakToWith() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    TernaryNode_new(
      NODE_WHILE,
      1,
      AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
      TernaryNode_new(
        NODE_WHILE,
        1,
        AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
        BinaryNode_new(
          NODE_BREAK,
          1,
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "2", 1),
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
        ),
        NULL
      ),
      NULL
    ),
    NULL
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileElseBreak() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    BinaryNode_new(
      NODE_BREAK,
      1,
      NULL,
      NULL
    ),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileElseBreakTo() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    TernaryNode_new(
      NODE_WHILE,
      1,
      AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
      TernaryNode_new(
        NODE_WHILE,
        1,
        AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
        BinaryNode_new(
          NODE_BREAK,
          1,
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "2", 1),
          NULL
        ),
        AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
      ),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, "43", 2)
    ),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, "44", 2)
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileElseBreakWith() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    BinaryNode_new(
      NODE_BREAK,
      1,
      NULL,
      AtomNode_new(NODE_INTEGER_LITERAL, 1, "37", 2)
    ),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
}

void test_Compiler_emitNode_whileElseBreakToWith() {
  Compiler compiler;
  Compiler_init(&compiler);

  Node* node = TernaryNode_new(
    NODE_WHILE,
    1,
    AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
    TernaryNode_new(
      NODE_WHILE,
      1,
      AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
      TernaryNode_new(
        NODE_WHILE,
        1,
        AtomNode_new(NODE_BOOLEAN_LITERAL, 1, "true", 4),
        BinaryNode_new(
          NODE_BREAK,
          1,
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "2", 1),
          AtomNode_new(NODE_INTEGER_LITERAL, 1, "37", 2)
        ),
        AtomNode_new(NODE_INTEGER_LITERAL, 1, "42", 2)
      ),
      AtomNode_new(NODE_INTEGER_LITERAL, 1, "43", 2)
    ),
    AtomNode_new(NODE_INTEGER_LITERAL, 1, "44", 2)
  );

  ByteCode out;
  ByteCode_init(&out);

  Compiler_emitNode(&compiler, &out, node);

  // TODO Assertions here

  Node_free(node);
  ByteCode_free(&out);
  Compiler_free(&compiler);

  assert(false);
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
