#include <stdio.h>
#include "thread.h"

void Thread_init(Thread* self, uint8_t* pc) {
  self->pc = pc;
  ValueStack_init(&(self->stack));
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

void Thread_run(Thread* self) {
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
