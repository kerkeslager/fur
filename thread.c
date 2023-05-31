#include <stdio.h>

#include "thread.h"

#include "error.h"

typedef enum {
  ERROR_DIVISON_BY_ZERO,
  ERROR_COMPARISON_TYPE_ERROR,
} RuntimeError;

void Thread_init(Thread* self, InstructionList* instructionList) {
  self->instructionList = instructionList;
  self->pcIndex = 0;
  ValueStack_init(&(self->stack));
  self->panic = false;
}

void Thread_free(Thread* self) {
  ValueStack_free(&(self->stack));
}

static Value Thread_error(Thread* self, RuntimeError error, uint8_t* pc) {
  if(isColorAllowed()) {
    fprintf(stderr, ANSI_COLOR_RED);
  }

  size_t line = InstructionList_getLine(self->instructionList, pc);
  fprintf(stderr, "Error (line %zu): ", line);

  switch(error) {
    case ERROR_COMPARISON_TYPE_ERROR:
      /*
       * TODO
       * We have access to the stack, so we can find out what the errors are
       * and give a better message than this.
       */
      fprintf(stderr, "Attempting to compare incomparably-typed values.");
      break;

    case ERROR_DIVISON_BY_ZERO:
      fprintf(stderr, "Division by 0.");
      break;
  }

  fprintf(stderr, "\n");

  if(isColorAllowed()) {
    fprintf(stderr, ANSI_COLOR_RESET);
  }

  self->panic = true;

  self->pcIndex = InstructionList_index(self->instructionList, pc);
  return NIL;
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

inline static Value opLessThan(Value a, Value b) {
  return Value_fromBoolean(Value_asInteger(a) < Value_asInteger(b));
}
inline static Value opLessThanEqual(Value a, Value b) {
  return Value_fromBoolean(Value_asInteger(a) <= Value_asInteger(b));
}
inline static Value opGreaterThan(Value a, Value b) {
  return Value_fromBoolean(Value_asInteger(a) > Value_asInteger(b));
}
inline static Value opGreaterThanEqual(Value a, Value b) {
  return Value_fromBoolean(Value_asInteger(a) >= Value_asInteger(b));
}
inline static Value opEqual(Value a, Value b) {
  /*
   * We have already checked that the types are the same, so we can infer the
   * type of b from a.
   */
  switch(a.type) {
    case VALUE_BOOLEAN:
      return Value_fromBoolean(Value_asBoolean(a) == Value_asBoolean(b));

    case VALUE_INTEGER:
      return Value_fromBoolean(Value_asInteger(a) == Value_asInteger(b));

    case VALUE_NIL:
      return Value_fromBoolean(true);
  }
}
inline static Value opNotEqual(Value a, Value b) {
  /*
   * We have already checked that the types are the same, so we can infer the
   * type of b from a.
   */
  switch(a.type) {
    case VALUE_BOOLEAN:
      return Value_fromBoolean(Value_asBoolean(a) != Value_asBoolean(b));

    case VALUE_INTEGER:
      return Value_fromBoolean(Value_asInteger(a) != Value_asInteger(b));

    case VALUE_NIL:
      return Value_fromBoolean(false);
  }
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

      case OP_TRUE:
        ValueStack_push(stack, TRUE);
        break;

      case OP_FALSE:
        ValueStack_push(stack, FALSE);
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
            return Thread_error(self, ERROR_DIVISON_BY_ZERO, pc);
          }
        }
        ValueStack_binary(stack, opIDivide);
        break;

      case OP_LESS_THAN:
        if(ValueStack_peekN(stack, 1).type != VALUE_INTEGER
            || ValueStack_peek(stack).type != VALUE_INTEGER) {
          return Thread_error(self, ERROR_COMPARISON_TYPE_ERROR, pc);
        }
        ValueStack_binary(stack, opLessThan);
        break;
      case OP_LESS_THAN_EQUAL:
        if(ValueStack_peekN(stack, 1).type != VALUE_INTEGER
            || ValueStack_peek(stack).type != VALUE_INTEGER) {
          return Thread_error(self, ERROR_COMPARISON_TYPE_ERROR, pc);
        }
        ValueStack_binary(stack, opLessThanEqual);
        break;
      case OP_GREATER_THAN:
        if(ValueStack_peekN(stack, 1).type != VALUE_INTEGER
            || ValueStack_peek(stack).type != VALUE_INTEGER) {
          return Thread_error(self, ERROR_COMPARISON_TYPE_ERROR, pc);
        }
        ValueStack_binary(stack, opGreaterThan);
        break;
      case OP_GREATER_THAN_EQUAL:
        if(ValueStack_peekN(stack, 1).type != VALUE_INTEGER
            || ValueStack_peek(stack).type != VALUE_INTEGER) {
          return Thread_error(self, ERROR_COMPARISON_TYPE_ERROR, pc);
        }
        ValueStack_binary(stack, opGreaterThanEqual);
        break;
      case OP_EQUAL:
        if(ValueStack_peekN(stack, 1).type != ValueStack_peek(stack).type) {
          return Thread_error(self, ERROR_COMPARISON_TYPE_ERROR, pc);
        }
        ValueStack_binary(stack, opEqual);
        break;
      case OP_NOT_EQUAL:
        if(ValueStack_peekN(stack, 1).type != ValueStack_peek(stack).type) {
          return Thread_error(self, ERROR_COMPARISON_TYPE_ERROR, pc);
        }
        ValueStack_binary(stack, opNotEqual);
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

void test_Thread_run_executesIntegerMathOps() {
  typedef struct {
    Instruction instruction;
    int32_t operand0;
    int32_t operand1;
    int32_t result;
  } TestCase;

  const int TEST_COUNT = 4;

  /*
   * Don't infer the array length--we want an error if we add tests and don't
   * update the test count, otherwise the loop won't execute new tests.
   */
  TestCase tests[TEST_COUNT] = {
    { OP_ADD, 1, 2, 3 },
    { OP_SUBTRACT, 5, 2, 3 },
    { OP_MULTIPLY, 2, 3, 6 },
    { OP_IDIVIDE, 7, 2, 3 },
  };

  for(int i = 0; i < TEST_COUNT; i++) {
    InstructionList instructionList;
    InstructionList_init(&instructionList);
    InstructionList_append(&instructionList, OP_INTEGER, 1);
    InstructionList_appendInt32(&instructionList, tests[i].operand0, 1);
    InstructionList_append(&instructionList, OP_INTEGER, 1);
    InstructionList_appendInt32(&instructionList, tests[i].operand1, 1);
    InstructionList_append(&instructionList, tests[i].instruction, 1);
    InstructionList_append(&instructionList, OP_RETURN, 1);
    Thread thread;
    Thread_init(&thread, &instructionList);

    Value result = Thread_run(&thread);

    assert(result.type == VALUE_INTEGER);
    assert(Value_asInteger(result) == tests[i].result);

    InstructionList_free(&instructionList);
    Thread_free(&thread);
  }
}

void test_Thread_run_integerComparison() {
  typedef struct {
    Instruction instruction;
    int32_t operand0;
    int32_t operand1;
    bool result;
  } TestCase;

  const int TEST_COUNT = 18;

  /*
   * Don't infer the array length--we want an error if we add tests and don't
   * update the test count, otherwise the loop won't execute new tests.
   */
  TestCase tests[TEST_COUNT] = {
    { OP_LESS_THAN, 1, 2, true },
    { OP_LESS_THAN, 2, 2, false },
    { OP_LESS_THAN, 3, 2, false },
    { OP_LESS_THAN_EQUAL, 1, 2, true },
    { OP_LESS_THAN_EQUAL, 2, 2, true },
    { OP_LESS_THAN_EQUAL, 3, 2, false },
    { OP_GREATER_THAN, 1, 2, false },
    { OP_GREATER_THAN, 2, 2, false },
    { OP_GREATER_THAN, 3, 2, true },
    { OP_GREATER_THAN_EQUAL, 1, 2, false },
    { OP_GREATER_THAN_EQUAL, 2, 2, true },
    { OP_GREATER_THAN_EQUAL, 3, 2, true },
    { OP_EQUAL, 1, 2, false },
    { OP_EQUAL, 2, 2, true },
    { OP_EQUAL, 3, 2, false },
    { OP_NOT_EQUAL, 1, 2, true },
    { OP_NOT_EQUAL, 2, 2, false },
    { OP_NOT_EQUAL, 3, 2, true },
  };

  for(int i = 0; i < TEST_COUNT; i++) {
    InstructionList instructionList;
    InstructionList_init(&instructionList);
    InstructionList_append(&instructionList, OP_INTEGER, 1);
    InstructionList_appendInt32(&instructionList, tests[i].operand0, 1);
    InstructionList_append(&instructionList, OP_INTEGER, 1);
    InstructionList_appendInt32(&instructionList, tests[i].operand1, 1);
    InstructionList_append(&instructionList, tests[i].instruction, 1);
    InstructionList_append(&instructionList, OP_RETURN, 1);
    Thread thread;
    Thread_init(&thread, &instructionList);

    Value result = Thread_run(&thread);

    assert(result.type == VALUE_BOOLEAN);
    assert(Value_asBoolean(result) == tests[i].result);

    InstructionList_free(&instructionList);
    Thread_free(&thread);
  }
}

void test_Thread_clearPanic_setsPanicFalse() {
  InstructionList instructionList;
  InstructionList_init(&instructionList);

  InstructionList_append(&instructionList, OP_NIL, 1);
  InstructionList_append(&instructionList, OP_RETURN, 2);

  Thread thread;
  Thread_init(&thread, &instructionList);
  thread.panic = true;

  Thread_clearPanic(&thread);

  assert(!(thread.panic));
}

void test_Thread_clearPanic_setsPCIndexToEnd() {
  InstructionList instructionList;
  InstructionList_init(&instructionList);

  InstructionList_append(&instructionList, OP_NIL, 1);
  InstructionList_append(&instructionList, OP_RETURN, 2);

  Thread thread;
  Thread_init(&thread, &instructionList);
  thread.panic = true;

  Thread_clearPanic(&thread);

  assert(thread.pcIndex == 2);
}

// TODO Need a lot more tests here

#endif
