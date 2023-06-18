#include <stdarg.h>
#include <stdio.h>

#include "thread.h"

#include "error.h"

void Thread_init(Thread* self, ByteCode* byteCode) {
  self->byteCode = byteCode;
  self->pcIndex = 0;
  Stack_init(&(self->stack));
  self->panic = false;
}

void Thread_free(Thread* self) {
  Stack_free(&(self->stack));
}

static const char* Instruction_toOperatorCString(uint8_t* pc) {
  Instruction op = (Instruction)(*pc);

  switch(op) {
    case OP_NIL:
    case OP_TRUE:
    case OP_FALSE:
    case OP_INTEGER:
    case OP_GET:
    case OP_DUP:
    case OP_DROP:
    case OP_ROT3:
    case OP_JUMP:
    case OP_JUMP_TRUE:
    case OP_JUMP_FALSE:
    case OP_SCOPE_OPEN:
    case OP_SCOPE_CLOSE:
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

    case OP_SET:
      return "=";
  }

  // Should never get here
  assert(false);
  return ""; // Silence warnings
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

  // Should never get here
  assert(false);
  return ""; // silence warnings
}

Value Thread_run(Thread* self) {
  Stack* stack = &(self->stack);

  /*
   * The program counter is stored on the thread as an index, rather than a
   * pointer, so that resizing the ByteCode with realloc doesn't leave
   * the program counter hanging. This line restores the program counter
   * pointer from the index to a register pointer for speed.
   *
   * One implication of this is that we can't modify the index on another
   * thread while Thread_run() is running.
   */
  register uint8_t* pc = ByteCode_pc(self->byteCode, self->pcIndex);

  #define THREAD_ERROR(...) \
    printError(__VA_ARGS__); \
    self->panic = true; \
    return NIL

  #define CHECK_UNARY_TYPE(tt) \
    if(operand.type != tt) { \
      THREAD_ERROR( \
        ByteCode_getLine(self->byteCode, pc - 1), \
        "Cannot apply prefix operator '%s' to value of type '%s'.", \
        Instruction_toOperatorCString(pc - 1), \
        ValueType_toCString(operand.type) \
      ); \
    }
  #define CHECK_BINARY_TYPE(t0, t1) \
    if(operand0.type != t0 || operand1.type != t1) { \
      THREAD_ERROR( \
        ByteCode_getLine(self->byteCode, pc - 1), \
        "Cannot apply infix operator `%s` to values of type `%s` and `%s`.", \
        Instruction_toOperatorCString(pc - 1), \
        ValueType_toCString(operand0.type), \
        ValueType_toCString(operand1.type) \
      ); \
    }
  #define CHECK_SAME_TYPE() \
    if(operand0.type != operand1.type) { \
      THREAD_ERROR( \
        ByteCode_getLine(self->byteCode, pc - 1), \
        "Cannot apply infix operator `%s` to values of type `%s` and `%s`.", \
        Instruction_toOperatorCString(pc - 1), \
        ValueType_toCString(operand0.type), \
        ValueType_toCString(operand1.type) \
      ); \
    }

  for(;;) {
    Instruction instruction = *pc;
    pc++;

    switch(instruction) {
      case OP_NIL:
        Stack_push(stack, NIL);
        break;

      case OP_TRUE:
        Stack_push(stack, TRUE);
        break;

      case OP_FALSE:
        Stack_push(stack, FALSE);
        break;

      case OP_INTEGER:
        Stack_push(stack, Value_fromInteger(*((int32_t*)pc)));
        pc += sizeof(int32_t);
        break;

      case OP_GET:
        {
          uint16_t index = *((uint16_t*)pc);
          pc += sizeof(uint16_t);
          Stack_pushIndex(stack, index);
          break;
        }

      case OP_SET:
        {
          uint16_t index = *((uint16_t*)pc);
          pc += sizeof(uint16_t);
          Stack_popToIndex(stack, index);
          break;
        }

      case OP_NEGATE:
        {
          Value operand = Stack_pop(stack);
          CHECK_UNARY_TYPE(VALUE_INTEGER);
          Stack_push(stack, Value_fromInteger(-Value_asInteger(operand)));
        }
        break;

      case OP_NOT:
        {
          Value operand = Stack_pop(stack);
          CHECK_UNARY_TYPE(VALUE_BOOLEAN);
          Stack_push(stack, Value_fromBoolean(!Value_asBoolean(operand)));
        }
        break;

      case OP_ADD:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_BINARY_TYPE(VALUE_INTEGER, VALUE_INTEGER);

          Stack_push(
            stack,
            Value_fromInteger(Value_asInteger(operand0) + Value_asInteger(operand1))
          );
        }
        break;

      case OP_SUBTRACT:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_BINARY_TYPE(VALUE_INTEGER, VALUE_INTEGER);

          Stack_push(
            stack,
            Value_fromInteger(Value_asInteger(operand0) - Value_asInteger(operand1))
          );
        }
        break;

      case OP_MULTIPLY:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_BINARY_TYPE(VALUE_INTEGER, VALUE_INTEGER);

          Stack_push(
            stack,
            Value_fromInteger(Value_asInteger(operand0) * Value_asInteger(operand1))
          );
        }
        break;

      case OP_IDIVIDE:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_BINARY_TYPE(VALUE_INTEGER, VALUE_INTEGER);

          if(Value_asInteger(operand1) == 0) {
            THREAD_ERROR(
              ByteCode_getLine(self->byteCode, pc - 1),
              "Division by 0."
            );
          }

          Stack_push(
            stack,
            Value_fromInteger(Value_asInteger(operand0) / Value_asInteger(operand1))
          );
        }
        break;

      case OP_LESS_THAN:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_BINARY_TYPE(VALUE_INTEGER, VALUE_INTEGER);

          Stack_push(
            stack,
            Value_fromBoolean(Value_asInteger(operand0) < Value_asInteger(operand1))
          );
        }
        break;

      case OP_LESS_THAN_EQUAL:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_BINARY_TYPE(VALUE_INTEGER, VALUE_INTEGER);

          Stack_push(
            stack,
            Value_fromBoolean(Value_asInteger(operand0) <= Value_asInteger(operand1))
          );
        }
        break;

      case OP_GREATER_THAN:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_BINARY_TYPE(VALUE_INTEGER, VALUE_INTEGER);

          Stack_push(
            stack,
            Value_fromBoolean(Value_asInteger(operand0) > Value_asInteger(operand1))
          );
        }
        break;

      case OP_GREATER_THAN_EQUAL:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_BINARY_TYPE(VALUE_INTEGER, VALUE_INTEGER);

          Stack_push(
            stack,
            Value_fromBoolean(Value_asInteger(operand0) >= Value_asInteger(operand1))
          );
        }
        break;

      case OP_EQUAL:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_SAME_TYPE();

          switch(operand0.type) {
            case VALUE_NIL:
              // If both types are nil, that implies both values are nil
              Stack_push(stack, Value_fromBoolean(true));
              break;

            case VALUE_BOOLEAN:
              Stack_push(
                stack,
                Value_fromBoolean(Value_asBoolean(operand0) == Value_asBoolean(operand1))
              );
              break;

            case VALUE_INTEGER:
              Stack_push(
                stack,
                Value_fromBoolean(Value_asInteger(operand0) == Value_asInteger(operand1))
              );
              break;
          }
        }
        break;

      case OP_NOT_EQUAL:
        {
          Value operand1 = Stack_pop(stack);
          Value operand0 = Stack_pop(stack);

          CHECK_SAME_TYPE();

          switch(operand0.type) {
            case VALUE_NIL:
              // If both types are nil, that implies both values are nil
              Stack_push(stack, Value_fromBoolean(false));
              break;

            case VALUE_BOOLEAN:
              Stack_push(
                stack,
                Value_fromBoolean(Value_asBoolean(operand0) != Value_asBoolean(operand1))
              );
              break;

            case VALUE_INTEGER:
              Stack_push(
                stack,
                Value_fromBoolean(Value_asInteger(operand0) != Value_asInteger(operand1))
              );
              break;
          }
        }
        break;

      case OP_DUP:
        {
          // TODO There's probably a way to optimize this
          Value value = Stack_pop(stack);
          Value copy = Value_copy(&value);
          Stack_push(stack, value);
          Stack_push(stack, copy);
        }
        break;

      case OP_DROP:
        Stack_pop(stack);
        break;

      case OP_ROT3:
        {
          // TODO There's probably a way to optimize this
          Value values[3] = {
            Stack_pop(stack),
            Stack_pop(stack),
            Stack_pop(stack),
          };

          Stack_push(stack, values[0]);
          Stack_push(stack, values[2]);
          Stack_push(stack, values[1]);
        }
        break;

      case OP_JUMP:
        pc += *((int16_t*)pc);
        break;

      case OP_JUMP_TRUE:
        {
          Value operand = Stack_pop(stack);

          // TODO Handle this
          assert(operand.type == VALUE_BOOLEAN);

          if(Value_asBoolean(operand)) {
            pc += *((int16_t*)pc);
          } else {
            pc += sizeof(int16_t) / sizeof(uint8_t);
          }
        }
        break;

      case OP_JUMP_FALSE:
        {
          Value operand = Stack_pop(stack);

          // TODO Handle this
          assert(operand.type == VALUE_BOOLEAN);

          if(Value_asBoolean(operand)) {
            pc += sizeof(int16_t) / sizeof(uint8_t);
          } else {
            pc += *((int16_t*)pc);
          }
        }
        break;

      case OP_SCOPE_OPEN:
        Stack_openScope(&(self->stack));
        break;

      case OP_SCOPE_CLOSE:
        Stack_closeScope(&(self->stack));
        break;

      case OP_RETURN:
        self->pcIndex = ByteCode_index(self->byteCode, pc);
        return Stack_pop(stack);
    }
  }

  #undef CHECK_UNARY_TYPE
  #undef CHECK_BINARY_TYPE
  #undef CHECK_SAME_TYPE
  #undef THREAD_ERROR
}

void Thread_printStack(Thread* self) {
  Stack_println(&(self->stack));
}

#ifdef TEST

void test_Thread_run_executesIntegerMathOps() {
  #define TEST_COUNT 4

  typedef struct {
    Instruction instruction;
    int32_t operand0;
    int32_t operand1;
    int32_t result;
  } TestCase;

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
    ByteCode byteCode;
    ByteCode_init(&byteCode);
    ByteCode_append(&byteCode, OP_INTEGER, 1);
    ByteCode_appendInt32(&byteCode, tests[i].operand0, 1);
    ByteCode_append(&byteCode, OP_INTEGER, 1);
    ByteCode_appendInt32(&byteCode, tests[i].operand1, 1);
    ByteCode_append(&byteCode, tests[i].instruction, 1);
    ByteCode_append(&byteCode, OP_RETURN, 1);
    Thread thread;
    Thread_init(&thread, &byteCode);

    Value result = Thread_run(&thread);

    assert(result.type == VALUE_INTEGER);
    assert(Value_asInteger(result) == tests[i].result);

    ByteCode_free(&byteCode);
    Thread_free(&thread);
  }
  #undef TEST_COUNT
}

void test_Thread_run_integerComparison() {
  #define TEST_COUNT 18

  typedef struct {
    Instruction instruction;
    int32_t operand0;
    int32_t operand1;
    bool result;
  } TestCase;

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
    ByteCode byteCode;
    ByteCode_init(&byteCode);
    ByteCode_append(&byteCode, OP_INTEGER, 1);
    ByteCode_appendInt32(&byteCode, tests[i].operand0, 1);
    ByteCode_append(&byteCode, OP_INTEGER, 1);
    ByteCode_appendInt32(&byteCode, tests[i].operand1, 1);
    ByteCode_append(&byteCode, tests[i].instruction, 1);
    ByteCode_append(&byteCode, OP_RETURN, 1);
    Thread thread;
    Thread_init(&thread, &byteCode);

    Value result = Thread_run(&thread);

    assert(result.type == VALUE_BOOLEAN);
    assert(Value_asBoolean(result) == tests[i].result);

    ByteCode_free(&byteCode);
    Thread_free(&thread);
  }

  #undef TEST_COUNT
}

void test_Thread_clearPanic_setsPanicFalse() {
  ByteCode byteCode;
  ByteCode_init(&byteCode);

  ByteCode_append(&byteCode, OP_NIL, 1);
  ByteCode_append(&byteCode, OP_RETURN, 2);

  Thread thread;
  Thread_init(&thread, &byteCode);
  thread.panic = true;

  Thread_clearPanic(&thread);

  assert(!(thread.panic));
}

void test_Thread_clearPanic_setsPCIndexToEnd() {
  ByteCode byteCode;
  ByteCode_init(&byteCode);

  ByteCode_append(&byteCode, OP_NIL, 1);
  ByteCode_append(&byteCode, OP_RETURN, 2);

  Thread thread;
  Thread_init(&thread, &byteCode);
  thread.panic = true;

  Thread_clearPanic(&thread);

  assert(thread.pcIndex == 2);
}

// TODO Need a lot more tests here

#endif
