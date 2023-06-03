#include <stdarg.h>
#include <stdio.h>

#include "thread.h"

#include "error.h"

typedef enum {
  ERROR_DIVISON_BY_ZERO,
  ERROR_UNARY_OP_TYPE,
  ERROR_BINARY_OP_TYPE,
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

static const char* Instruction_toOperatorCString(uint8_t* pc) {
  Instruction op = (Instruction)(*pc);

  switch(op) {
    case OP_NIL:
    case OP_TRUE:
    case OP_FALSE:
    case OP_INTEGER:
    case OP_DROP:
    case OP_RETURN:
      assert(false);

    case OP_NOT:
      return "not";

    case OP_NEGATE:
      return "-";

    case OP_ADD:
      return "+";

    case OP_SUBTRACT:
      return "-";

    case OP_MULTIPLY:
      return "*";

    case OP_IDIVIDE:
      return "//";

    case OP_LESS_THAN:
      return "<";

    case OP_LESS_THAN_EQUAL:
      return "<=";

    case OP_GREATER_THAN:
      return ">";

    case OP_GREATER_THAN_EQUAL:
      return ">=";

    case OP_EQUAL:
      return "==";

    case OP_NOT_EQUAL:
      return "!=";
  }
}

const char* ValueType_toCString(ValueType type) {
  switch(type) {
    case VALUE_NIL:
      return "Void";

    case VALUE_BOOLEAN:
      return "Boolean";

    case VALUE_INTEGER:
      return "Integer";
  }
}

static Value Thread_error(Thread* self, RuntimeError error, uint8_t* pc, ...) {
  /*
   * Thread_run increments the program counter before the instruction is
   * executed, so we decrement it here to get the proper instruction, rather
   * than forcing every caller to pass in a decremented program counter.
   */
  // TODO Can we get this off self?
  pc--;

  va_list varargs;

  if(isColorAllowed()) {
    fprintf(stderr, ANSI_COLOR_RED);
  }

  size_t line = InstructionList_getLine(self->instructionList, pc);
  fprintf(stderr, "Error (line %zu): ", line);

  switch(error) {
    case ERROR_UNARY_OP_TYPE:
      {
        va_start(varargs, pc);

        // Currently all unary operations are prefix operators
        fprintf(
          stderr,
          "Cannot apply prefix operator `%s` to value of type `%s`.",
          Instruction_toOperatorCString(pc),
          ValueType_toCString(va_arg(varargs, ValueType))
        );

        va_end(varargs);
      }
      break;

    case ERROR_BINARY_OP_TYPE:
      {
        va_start(varargs, pc);

        // Currently all binary operations are infix operators
        fprintf(
          stderr,
          "Cannot apply infix operator `%s` to values of type `%s` and `%s`.",
          Instruction_toOperatorCString(pc),
          ValueType_toCString(va_arg(varargs, ValueType)),
          ValueType_toCString(va_arg(varargs, ValueType))
        );

        va_end(varargs);
      }
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
   * Another implication of this is that we can't modify the index on another
   * thread while Thread_run() is running.
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
        {
          Value operand = ValueStack_pop(stack);

          if(!(operand.type == VALUE_INTEGER)) {
            return Thread_error(self, ERROR_UNARY_OP_TYPE, pc, operand.type);
          }

          ValueStack_push(stack, Value_fromInteger(-Value_asInteger(operand)));
        }
        break;

      case OP_NOT:
        {
          Value operand = ValueStack_pop(stack);

          if(operand.type == VALUE_BOOLEAN) {
            return Thread_error(self, ERROR_UNARY_OP_TYPE, pc, operand.type);
          }

          ValueStack_push(stack, Value_fromBoolean(!Value_asBoolean(operand)));
        }
        break;

      case OP_ADD:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != VALUE_INTEGER || operand1.type != VALUE_INTEGER) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          ValueStack_push(
            stack,
            Value_fromInteger(Value_asInteger(operand0) + Value_asInteger(operand1))
          );
        }
        break;

      case OP_SUBTRACT:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != VALUE_INTEGER || operand1.type != VALUE_INTEGER) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          ValueStack_push(
            stack,
            Value_fromInteger(Value_asInteger(operand0) - Value_asInteger(operand1))
          );
        }
        break;

      case OP_MULTIPLY:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != VALUE_INTEGER || operand1.type != VALUE_INTEGER) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          ValueStack_push(
            stack,
            Value_fromInteger(Value_asInteger(operand0) * Value_asInteger(operand1))
          );
        }
        break;

      case OP_IDIVIDE:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != VALUE_INTEGER || operand1.type != VALUE_INTEGER) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          if(Value_asInteger(operand1) == 0) {
            return Thread_error(self, ERROR_DIVISON_BY_ZERO, pc);
          }

          ValueStack_push(
            stack,
            Value_fromInteger(Value_asInteger(operand0) / Value_asInteger(operand1))
          );
        }
        break;

      case OP_LESS_THAN:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != VALUE_INTEGER || operand1.type != VALUE_INTEGER) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          ValueStack_push(
            stack,
            Value_fromBoolean(Value_asInteger(operand0) < Value_asInteger(operand1))
          );
        }
        break;

      case OP_LESS_THAN_EQUAL:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != VALUE_INTEGER || operand1.type != VALUE_INTEGER) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          ValueStack_push(
            stack,
            Value_fromBoolean(Value_asInteger(operand0) <= Value_asInteger(operand1))
          );
        }
        break;

      case OP_GREATER_THAN:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != VALUE_INTEGER || operand1.type != VALUE_INTEGER) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          ValueStack_push(
            stack,
            Value_fromBoolean(Value_asInteger(operand0) > Value_asInteger(operand1))
          );
        }
        break;

      case OP_GREATER_THAN_EQUAL:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != VALUE_INTEGER || operand1.type != VALUE_INTEGER) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          ValueStack_push(
            stack,
            Value_fromBoolean(Value_asInteger(operand0) >= Value_asInteger(operand1))
          );
        }
        break;

      case OP_EQUAL:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != operand1.type) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          switch(operand0.type) {
            case VALUE_NIL:
              // If both types are nil, that implies both values are nil
              ValueStack_push(stack, Value_fromBoolean(true));
              break;

            case VALUE_BOOLEAN:
              ValueStack_push(
                stack,
                Value_fromBoolean(Value_asBoolean(operand0) == Value_asBoolean(operand1))
              );
              break;

            case VALUE_INTEGER:
              ValueStack_push(
                stack,
                Value_fromBoolean(Value_asInteger(operand0) == Value_asInteger(operand1))
              );
              break;
          }
        }
        break;

      case OP_NOT_EQUAL:
        {
          Value operand1 = ValueStack_pop(stack);
          Value operand0 = ValueStack_pop(stack);

          if(operand0.type != operand1.type) {
            return Thread_error(self, ERROR_BINARY_OP_TYPE, pc, operand0.type, operand1.type);
          }

          switch(operand0.type) {
            case VALUE_NIL:
              // If both types are nil, that implies both values are nil
              ValueStack_push(stack, Value_fromBoolean(false));
              break;

            case VALUE_BOOLEAN:
              ValueStack_push(
                stack,
                Value_fromBoolean(Value_asBoolean(operand0) != Value_asBoolean(operand1))
              );
              break;

            case VALUE_INTEGER:
              ValueStack_push(
                stack,
                Value_fromBoolean(Value_asInteger(operand0) != Value_asInteger(operand1))
              );
              break;
          }
        }
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
