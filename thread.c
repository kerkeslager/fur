#include <stdio.h>

#include "thread.h"

#include "error.h"

void Thread_init(Thread* self, uint8_t* pc) {
  self->pc = pc;
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

/*
 * TODO
 * Intuitively something feels wrong about passing in the instruction list
 * here. It is only used for retrieving line numbers for errors. I'm not sure
 * why this feels bad, so I'll leave it until that becomes clear.
 */
void Thread_run(Thread* self, InstructionList* instructionList) {
  // TODO Consider copying the pc into a register
  ValueStack* stack = &(self->stack);

  for(;;) {
    Instruction instruction = *(self->pc);
    self->pc++;

    switch(instruction) {
      case OP_NIL:
        ValueStack_push(stack, Value_nil());
        break;

      case OP_INTEGER:
        ValueStack_push(stack, Value_fromInteger(*((int32_t*)(self->pc))));
        self->pc += sizeof(int32_t);
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
                InstructionList_getLine(instructionList, self->pc)
            );

            if(isColorAllowed()) {
              fprintf(stderr, ANSI_COLOR_RESET);
            }

            self->panic = true;
            return;
          }
        }
        ValueStack_binary(stack, opIDivide);
        break;

      case OP_DROP:
        ValueStack_pop(stack);
        break;

      case OP_RETURN:
        return;

      default:
        assert(false);
    }
  }
}

#ifdef TEST

#endif
