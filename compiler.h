#ifndef COMPILER_H
#define COMPILER_H

#include "instruction.h"

void Compiler_compile(InstructionList* out, const char* source);

#ifdef TEST

void test_Compiler_emitNode_emitsIntegerLiteral();
void test_Compiler_emitNode_emitsNegate();

void test_Compiler_emitNode_emitsAdd();
void test_Compiler_emitNode_emitsSubtract();
void test_Compiler_emitNode_emitsMultiply();
void test_Compiler_emitNode_emitsIntegerDivide();

void test_Compiler_compile_emitsNilOnEmptyInput();
void test_Compiler_compile_emitsNilOnBlankInput();

#endif

#endif
