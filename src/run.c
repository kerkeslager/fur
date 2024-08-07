#include "instruction.h"
#include "run.h"

void runInstruction(Thread* thread) {
  switch((Instruction)*(thread->ip)) {
    case INST_NIL:
      // TODO Implement
      assert(false);
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

void run(Thread* thread) {
  for(;;) {
    runInstruction(thread);
  }
}
