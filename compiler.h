#ifndef COMPILER_H
#define COMPILER_H

#include <stdbool.h>

#include "instruction.h"
#include "parser.h"
#include "symbol_list.h"
#include "symbol_table.h"

typedef struct {
  size_t index;
  size_t depth;
} Break;

typedef struct {
  SymbolTable symbolTable;
  SymbolList symbolList;
  bool hasErrors;

  Break* breaks;
  size_t breakCount;
  size_t breakCapacity;
} Compiler;

void Compiler_init(Compiler*);
void Compiler_free(Compiler*);
bool Compiler_compile(Compiler*, ByteCode* out, Parser*);

#ifdef TEST

void test_Compiler_emitNode_emitsIntegerLiteral();
void test_Compiler_emitNode_emitsNegate();
void test_Compiler_emitNode_emitsNot();

void test_Compiler_emitNode_emitsAdd();
void test_Compiler_emitNode_emitsSubtract();
void test_Compiler_emitNode_emitsMultiply();
void test_Compiler_emitNode_emitsIntegerDivide();

void test_Compiler_emitNode_emitsComparisons();

void test_Compiler_emitNode_emitsAndOr();

void test_Compiler_emitNode_loop();
void test_Compiler_emitNode_if();
void test_Compiler_emitNode_ifElse();
void test_Compiler_emitNode_while();
void test_Compiler_emitNode_whileElse();
void test_Compiler_emitNode_until();
void test_Compiler_emitNode_untilElse();

void test_Compiler_emitNode_loopContinue();

/*
 * TODO
 * The following tests have all been sketched out, but actually writing them
 * is a tedious chore.
 */
//void test_Compiler_emitNode_loopContinueTo();
//void test_Compiler_emitNode_whileContinue();
//void test_Compiler_emitNode_whileContinueTo();
//void test_Compiler_emitNode_whileElseContinue();
//void test_Compiler_emitNode_whileElseContinueTo();

//void test_Compiler_emitNode_loopBreak();
//void test_Compiler_emitNode_loopBreakTo();
//void test_Compiler_emitNode_loopBreakWith();
//void test_Compiler_emitNode_loopBreakToWith();
//void test_Compiler_emitNode_whileBreak();
//void test_Compiler_emitNode_whileBreakTo();
//void test_Compiler_emitNode_whileBreakWith();
//void test_Compiler_emitNode_whileBreakToWith();
//void test_Compiler_emitNode_whileElseBreak();
//void test_Compiler_emitNode_whileElseBreakTo();
//void test_Compiler_emitNode_whileElseBreakWith();
//void test_Compiler_emitNode_whileElseBreakToWith();

void test_Compiler_compile_emitsVariableInstructions();

void test_Compiler_compile_emitsNilOnEmptyInput();
void test_Compiler_compile_emitsNilOnBlankInput();

#endif

#endif
