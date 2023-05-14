#ifndef COMPILER_H
#define COMPILER_H

#include "instruction.h"

void Compiler_compile(const char* source, InstructionList* out);

#ifdef TEST

void test_Compiler_emitNode_emitsIntegerLiteral();

#endif

#endif
