#include <stdio.h>

#include "thread.h"

#include "error.h"

void Thread_init(Thread* self, InstructionList* instructionList) {
  self->instructionList = instructionList;
  self->pcIndex = 0;
  ValueStack_init(&(self->stack));
  self->panic = false;
}

void Thread_free(Thread* self) {
  ValueStack_free(&(self->stack));
}

inline static Value opNegate(Value a) {
  return Value_fromInteger(-Value_asInteger(a));
}

inline static Value opAdd(Value a, Value b) {
  return Value_fromInteger(Value_asInteger(a) + Value_asInteger(b));
}
inline static Value opSubtract(Value a, Value b) {
  return Value_fromInteger(Value_asInteger(a) - Value_asInteger(b));
}
inline static Value opMultiply(Value a, Value b) {
  return Value_fromInteger(Value_asInteger(a) * Value_asInteger(b));
}
inline static Value opIDivide(Value a, Value b) {
  return Value_fromInteger(Value_asInteger(a) / Value_asInteger(b));
}

Value Thread_run(Thread* self) {
  // TODO Consider copying the pc into a register
  ValueStack* stack = &(self->stack);

  /*
   * The program counter is stored on the thread as an index, rather than a
   * pointer, so that resizing the InstructionList with realloc doesn't leave
   * the program counter hanging. This line restores the program counter
   * pointer from the index, but *everywhere* that we exit from this function,
   * we have to sync the index with InstructionList_index().
   *
   * Another implication of this is that we can't modify the index on while
   * another thread while Thread_run() is running.
   */
  register uint8_t* pc = InstructionList_pc(self->instructionList, self->pcIndex);

  for(;;) {
    Instruction instruction = *pc;
    pc++;

    switch(instruction) {
      case OP_NIL:
        ValueStack_push(stack, NIL);
        break;

      case OP_INTEGER:
        ValueStack_push(stack, Value_fromInteger(*((int32_t*)pc)));
        pc += sizeof(int32_t);
        break;

      case OP_NEGATE:
        ValueStack_unary(stack, opNegate);
        break;

      case OP_ADD:
        ValueStack_binary(stack, opAdd);
        break;

      case OP_SUBTRACT:
        ValueStack_binary(stack, opSubtract);
        break;

      case OP_MULTIPLY:
        ValueStack_binary(stack, opMultiply);
        break;

      case OP_IDIVIDE:
        {
          int32_t denominator = Value_asInteger(ValueStack_peek(stack));
          if(denominator == 0) {
            if(isColorAllowed()) {
              fprintf(stderr, ANSI_COLOR_RED);
            }

            fprintf(
                stderr,
                "Error (line %zu): Division by 0.\n",
                InstructionList_getLine(self->instructionList, pc)
            );

            if(isColorAllowed()) {
              fprintf(stderr, ANSI_COLOR_RESET);
            }

            self->panic = true;

            self->pcIndex = InstructionList_index(self->instructionList, pc);
            return NIL;
          }
        }
        ValueStack_binary(stack, opIDivide);
        break;

      case OP_DROP:
        ValueStack_pop(stack);
        break;

      case OP_RETURN:
        self->pcIndex = InstructionList_index(self->instructionList, pc);
        return ValueStack_pop(stack);

      default:
        assert(false);
    }
  }
}

#ifdef TEST

// TODO Tests

#endif
