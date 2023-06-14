#ifndef COMPILER_H
#define COMPILER_H

#include <stdbool.h>

#include "instruction.h"
#include "parser.h"
#include "symbol_list.h"
#include "symbol_table.h"

typedef struct {
  SymbolTable symbolTable;
  SymbolList symbolList;
  bool hasErrors;
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

void test_Compiler_emitNode_loop();
void test_Compiler_emitNode_if();
void test_Compiler_emitNode_ifElse();
void test_Compiler_emitNode_while();
void test_Compiler_emitNode_whileElse();
void test_Compiler_emitNode_until();
void test_Compiler_emitNode_untilElse();

void test_Compiler_compile_emitsVariableInstructions();

void test_Compiler_compile_emitsNilOnEmptyInput();
void test_Compiler_compile_emitsNilOnBlankInput();

#endif

#endif
