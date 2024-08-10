#ifndef RUN_H
#define RUN_H

#include "thread.h"

void run(Thread*);

#ifdef TEST
void test_nil();
void test_true();
void test_false();
void test_not();
void test_int32();
#endif

#endif
