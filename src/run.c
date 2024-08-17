#include "instruction.h"
#include "run.h"

void runInstruction(Thread* thread) {
  switch((Instruction)*(thread->ip++)) {
    case INST_NIL:
      {
        Value v = {
          .type = VALUE_OBJ,
          .as.obj = NULL
        };
        ValueStack_push(&(thread->stack), v);
      }
      break;

    case INST_TRUE:
      ValueStack_push(&(thread->stack), Value_fromBool(true));
      break;

    case INST_FALSE:
      ValueStack_push(&(thread->stack), Value_fromBool(false));
      break;

    case INST_NOT:
      ValueStack_push(
        &(thread->stack),
        Value_fromBool(!Value_asBool(ValueStack_pop(&(thread->stack))))
      );
      break;

    case INST_INT32:
      {
        int32_t result = *((int32_t*)(thread->ip));
        thread->ip += sizeof(int32_t);
        ValueStack_push(&(thread->stack), Value_fromInt(result));
      }
      break;

    case INST_NEG:
      ValueStack_push(
        &(thread->stack),
        Value_fromInt(-Value_asInt(ValueStack_pop(&(thread->stack))))
      );
      break;

    case INST_ADD:
      // TODO Implement
      assert(false);
      break;

    case INST_SUB:
      // TODO Implement
      assert(false);
      break;

    case INST_MUL:
      // TODO Implement
      assert(false);
      break;

    case INST_MOD_SET:
      // TODO Implement
      assert(false);
      break;

    case INST_MOD_GET:
      // TODO Implement
      assert(false);
      break;

    case INST_IMPORT:
      // TODO Implement
      assert(false);
      break;

    case INST_SCOPE_SET:
      // TODO Implement
      assert(false);
      break;

    case INST_SCOPE_GET:
      // TODO Implement
      assert(false);
      break;

    case INST_JMP:
      // TODO Implement
      assert(false);
      break;

    case INST_JMP_IF:
      // TODO Implement
      assert(false);
      break;

    case INST_CALL:
      // TODO Implement
      assert(false);
      break;

    case INST_RETURN:
      // TODO Implement
      assert(false);
      break;

    case INST_FORK:
      // TODO Implement
      assert(false);
      break;

    case INST_SEND:
      // TODO Implement
      assert(false);
      break;

    case INST_RECV:
      // TODO Implement
      assert(false);
      break;

    case INST_LIST:
      // TODO Implement
      assert(false);
      break;

    case INST_DICT:
      // TODO Implement
      assert(false);
      break;

    case INST_GETITEM:
      // TODO Implement
      assert(false);
      break;

    case INST_SETITEM:
      // TODO Implement
      assert(false);
      break;

    case INST_STRUCT:
      // TODO Implement
      assert(false);
      break;

    case INST_GETFIELD:
      // TODO Implement
      assert(false);
      break;
  }
}

#ifdef TEST
void test_nil() {
  Value stackItems[10];
  Test_init(stackItems);
  uint8_t instruction = (uint8_t)INST_NIL;

  Thread thread;
  Thread_init(&thread, &instruction);

  runInstruction(&thread);

  assert(Value_isNil(ValueStack_peek(&(thread.stack))));
  assert(thread.ip == (&instruction) + 1);
}

void test_true() {
  Value stackItems[10];
  Test_init(stackItems);
  uint8_t instruction = (uint8_t)INST_TRUE;

  Thread thread;
  Thread_init(&thread, &instruction);

  runInstruction(&thread);

  assert(Value_asBool(ValueStack_peek(&(thread.stack))));
  assert(thread.ip == (&instruction) + 1);
}

void test_false() {
  Value stackItems[10];
  Test_init(stackItems);
  uint8_t instruction = (uint8_t)INST_FALSE;

  Thread thread;
  Thread_init(&thread, &instruction);

  runInstruction(&thread);

  assert(!Value_asBool(ValueStack_peek(&(thread.stack))));
  assert(thread.ip == (&instruction) + 1);
}

void test_not() {
  Value stackItems[10];
  Test_init(stackItems);
  uint8_t instruction = (uint8_t)INST_NOT;

  Thread thread;
  Thread_init(&thread, &instruction);
  ValueStack_push(&(thread.stack), Value_fromBool(true));

  runInstruction(&thread);

  assert(!Value_asBool(ValueStack_peek(&(thread.stack))));
  assert(thread.ip == (&instruction) + 1);

  thread.ip = &instruction;

  runInstruction(&thread);

  assert(Value_asBool(ValueStack_peek(&(thread.stack))));
  assert(thread.ip == (&instruction) + 1);
}

void test_int32() {
  Value stackItems[10];
  Test_init(stackItems);
  uint8_t instructions[1 + sizeof(int32_t)];
  instructions[0] = (uint8_t)INST_INT32;
  *((int32_t*)&(instructions[1])) = 42;

  Thread thread;
  Thread_init(&thread, instructions);

  runInstruction(&thread);

  assert(Value_asInt(ValueStack_peek(&(thread.stack))) == 42);
  assert(thread.ip == instructions + 1 + sizeof(int32_t));
}

void test_neg() {
  Value stackItems[10];
  Test_init(stackItems);
  uint8_t instruction = (uint8_t)INST_NEG;

  Thread thread;
  Thread_init(&thread, &instruction);
  ValueStack_push(&(thread.stack), Value_fromInt(42));

  runInstruction(&thread);

  assert(Value_asInt(ValueStack_peek(&(thread.stack))) == -42);
}
#endif

void run(Thread* thread) {
  for(;;) {
    runInstruction(thread);
  }
}
