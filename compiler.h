#ifndef COMPILER_H
#define COMPILER_H

#include "instruction.h"

void Compiler_compile(const char* source, InstructionList* out);

#ifdef TEST

void test_Compiler_emitNode_emitsIntegerLiteral();
void test_Compiler_emitNode_emitsNegate();

void test_Compiler_emitNode_emitsAdd();
void test_Compiler_emitNode_emitsSubtract();
void test_Compiler_emitNode_emitsMultiply();
void test_Compiler_emitNode_emitsIntegerDivide();

#endif

#endif
