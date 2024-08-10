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
      // TODO Implement
      assert(false);
      break;

    case INST_FALSE:
      // TODO Implement
      assert(false);
      break;

    case INST_NOT:
      // TODO Implement
      assert(false);
      break;

    case INST_INT32:
      // TODO Implement
      assert(false);
      break;

    case INST_NEG:
      // TODO Implement
      assert(false);
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
  uint8_t instruction = (uint8_t)INST_NIL;;

  Thread thread;
  Thread_init(&thread, &instruction);

  runInstruction(&thread);

  assert(Value_isNil(ValueStack_peek(&(thread.stack))));
  assert(thread.ip == (&instruction) + 1);
}
#endif

void run(Thread* thread) {
  for(;;) {
    runInstruction(thread);
  }
}
