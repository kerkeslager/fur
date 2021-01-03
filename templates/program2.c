#include<assert.h>
#include<stdbool.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

enum Type;
typedef enum Type Type;
enum Type {
  BOOLEAN,
  BUILTIN,
  CLOSURE,
  INTEGER,
  LIST,
  STRING,
  STRUCTURE,
  SYMBOL
};

enum Builtin;
typedef enum Builtin Builtin;
enum Builtin {
  NIL,
  POW,
  PRINT
};

struct Object;
typedef struct Object Object;
struct Environment;
typedef struct Environment Environment;
struct Thread;
typedef struct Thread Thread;

struct Closure;
typedef struct Closure Closure;
struct Closure {
  Environment* environment;
  size_t entry;
};

struct List;
typedef struct List List;

struct Structure;
typedef struct Structure Structure;

union Value;
typedef union Value Value;
union Value {
  Builtin builtin;
  bool boolean;
  Closure closure;
  int32_t integer;
  List* list;
  char* string;
  Structure* structure;
  char* symbol;
};

struct Object {
  Type type;
  Value value;
};

struct List {
  Object head;
  List* tail;
};

struct Structure {
  char* key;
  Object value;
  Structure* next;
};

#define BUILTIN_NIL (Object) { BUILTIN, (Value)(Builtin)NIL }

void Object_deinitialize(Object* self) {
}

{% include "environment.c" %}
{% include "stack.c" %}
{% include "frame.c" %}

struct Thread {
  Frame frame;
  Stack stack;
};

void Thread_initialize(Thread* self, size_t programCounter) {
  Frame_initialize(&(self->frame), Environment_construct(NULL), NULL, programCounter);
  Stack_initialize(&(self->stack));
}

void Thread_deinitialize(Thread* self) {
  Frame_deinitialize(&(self->frame));
  Stack_deinitialize(&(self->stack));
}

Environment* Thread_getEnvironment(Thread* self) {
  return self->frame.environment;
}

void Thread_setProgramCounter(Thread* self, size_t programCounter) {
  self->frame.programCounter = programCounter;
}

void Thread_incrementProgramCounter(Thread* self) {
  self->frame.programCounter++;
}

size_t Thread_getProgramCounter(Thread* self) {
  return self->frame.programCounter;
}

union Argument;
typedef const union Argument Argument;
union Argument {
  size_t label;
  void* pointer;
  char* string;
  int32_t integer;
  char* symbol;
};

void callBuiltinPow(Thread* thread, size_t argumentCount) {
  assert(argumentCount == 2);
  assert(!Stack_isEmpty(&(thread->stack)));
  Object exponent = Stack_pop(&(thread->stack));
  assert(exponent.type == INTEGER);
  assert(exponent.value.integer >= 0);

  assert(!Stack_isEmpty(&(thread->stack)));
  Object base = Stack_pop(&(thread->stack));
  assert(base.type == INTEGER);

  Object result;
  result.type = INTEGER;
  result.value.integer = 1;

  while(exponent.value.integer > 0) {
    result.value.integer *= base.value.integer;
    exponent.value.integer--;
  }

  Stack_push(&(thread->stack), result);
}

void callBuiltinPrint(Thread* thread, size_t argumentCount) {
  assert(argumentCount > 0);

  Object arguments[argumentCount];
  size_t count;

  for(count = 0; count < argumentCount; count++) {
    assert(!Stack_isEmpty(&(thread->stack)));
    arguments[argumentCount - count - 1] = Stack_pop(&(thread->stack));
  }

  for(count = 0; count < argumentCount; count ++) {
    Object arg = arguments[count];

    switch(arg.type) {
      case BOOLEAN:
        if(arg.value.boolean) printf("true");
        else printf("false");
        break;

      case INTEGER:
        printf("%i", arg.value.integer);
        break;

      case STRING:
        printf("%s", arg.value.string);
        break;

      default:
        assert(false);
    }
  }

  Stack_push(&(thread->stack), BUILTIN_NIL);
}

void callBuiltin(Thread* thread, Builtin b, size_t argumentCount) {
  switch(b) {
    case POW:
      callBuiltinPow(thread, argumentCount);
      break;

    case PRINT:
      callBuiltinPrint(thread, argumentCount);
      break;

    default:
      assert(false);
  }
}

void callClosure(Thread* thread, Closure closure, size_t argumentCount) {
  // TODO Find a way to assert the argument count

  Frame* returnFrame = malloc(sizeof(Frame));
  *returnFrame = thread->frame;
  Frame_initialize(
    &(thread->frame),
    Environment_construct(Environment_reference(closure.environment)),
    returnFrame,
    closure.entry - 1 // We will increment the frame immediately after this
  );
}

void inst_call(Thread* thread, Argument argument) {
  assert(!Stack_isEmpty(&(thread->stack)));
  Object f = Stack_pop(&(thread->stack));
  size_t argumentCount = argument.label;

  switch(f.type) {
    case BUILTIN:
      callBuiltin(thread, f.value.builtin, argumentCount);
      break;

    case CLOSURE:
      callClosure(thread, f.value.closure, argumentCount);
      break;

    default:
      assert(false);
  }
}

void inst_concat(Thread* thread, Argument argument) {
  assert(!Stack_isEmpty(&(thread->stack)));
  Object left = Stack_pop(&(thread->stack));
  assert(!Stack_isEmpty(&(thread->stack)));
  Object right = Stack_pop(&(thread->stack));

  assert(left.type == STRING);
  assert(right.type == STRING);

  char* resultString = malloc(strlen(left.value.string) + strlen(right.value.string) + 1);
  resultString[0] = '\0';

  strcat(resultString, left.value.string);
  strcat(resultString, right.value.string);

  Object resultObject = (Object) {
    STRING,
    (Value)resultString
  };

  Stack_push(&(thread->stack), resultObject);
}

{% with name='add', operation='+' %}
  {% include "arithmetic_instruction.c" %}
{% endwith %}

void inst_close(Thread* thread, Argument argument) {
  Object result;
  result.type = CLOSURE;
  result.value.closure.environment = Thread_getEnvironment(thread);
  result.value.closure.entry = argument.label;

  Stack_push(&(thread->stack), result);
}

void inst_drop(Thread* thread, Argument argument) {
  assert(!Stack_isEmpty(&(thread->stack)));
  Object result = Stack_pop(&(thread->stack));
  Object_deinitialize(&result);
}

void inst_end(Thread* thread, Argument argument) {
}

{% with name='eq', operation='==' %}
  {% include "comparison_instruction.c" %}
{% endwith %}

void inst_field(Thread* thread, Argument argument) {
  assert(!Stack_isEmpty(&(thread->stack)));
  Object key = Stack_pop(&(thread->stack));
  assert(key.type == SYMBOL);

  assert(!Stack_isEmpty(&(thread->stack)));
  Object structure = Stack_pop(&(thread->stack));
  assert(structure.type == STRUCTURE);

  while(structure.value.structure != NULL) {
    if(strcmp(structure.value.structure->key, key.value.string) == 0) {
      Stack_push(&(thread->stack), structure.value.structure->value);
      return;
    }
    structure.value.structure = structure.value.structure->next;
  }

  assert(false); // Symbol wasn't found in structure
}

void inst_get(Thread* thread, Argument argument) {
  assert(!Stack_isEmpty(&(thread->stack)));
  Object indexObject = Stack_pop(&(thread->stack));
  assert(indexObject.type == INTEGER);
  int32_t index = indexObject.value.integer;

  assert(!Stack_isEmpty(&(thread->stack)));
  Object listObject = Stack_pop(&(thread->stack));
  assert(listObject.type == LIST);
  List* list = listObject.value.list;

  while(index > 0) {
    assert(list != NULL);
    list = list->tail;
    index--;
  }

  assert(list != NULL);
  Stack_push(&(thread->stack), list->head);
}

{% with name='gt', operation='>' %}
  {% include "comparison_instruction.c" %}
{% endwith %}

{% with name='gte', operation='>=' %}
  {% include "comparison_instruction.c" %}
{% endwith %}

{% with name='idiv', operation='/' %}
  {% include "arithmetic_instruction.c" %}
{% endwith %}

void inst_jump(Thread* thread, Argument argument) {
  Thread_setProgramCounter(thread, argument.label - 1); // We will increment before running
}

void inst_jump_if_false(Thread* thread, Argument argument) {
  assert(!Stack_isEmpty(&(thread->stack)));
  Object result = Stack_pop(&(thread->stack));
  assert(result.type == BOOLEAN);

  if(!(result.value.boolean)) {
    inst_jump(thread, argument);
  }
}

void inst_list(Thread* thread, Argument argument) {
  Object result;
  result.type = LIST;
  result.value.list = NULL;

  int32_t count = argument.integer;

  while(count > 0) {
    assert(!Stack_isEmpty(&(thread->stack)));
    Object item = Stack_pop(&(thread->stack));

    List* node = malloc(sizeof(List));
    node->head = item;
    node->tail = result.value.list;

    result.value.list = node;
    count--;
  }

  Stack_push(&(thread->stack), result);
}

{% with name='lt', operation='<' %}
  {% include "comparison_instruction.c" %}
{% endwith %}

{% with name='lte', operation='<=' %}
  {% include "comparison_instruction.c" %}
{% endwith %}

{% with name='mod', operation='%' %}
  {% include "arithmetic_instruction.c" %}
{% endwith %}

{% with name='mul', operation='*' %}
  {% include "arithmetic_instruction.c" %}
{% endwith %}

{% with name='neq', operation='!=' %}
  {% include "comparison_instruction.c" %}
{% endwith %}

void inst_neg(Thread* thread, Argument argument) {
  assert(!Stack_isEmpty(&(thread->stack)));
  Object result = Stack_pop(&(thread->stack));
  assert(result.type == INTEGER);

  result.value.integer = -(result.value.integer);

  Stack_push(&(thread->stack), result);
}

void inst_pop(Thread* thread, Argument argument) {
  char* argumentString = argument.string;

  assert(!Stack_isEmpty(&(thread->stack)));
  Object result = Stack_pop(&(thread->stack));

  if(strcmp(argumentString, "print") == 0) {
    assert(false);
  } else if(strcmp(argumentString, "pow") == 0) {
    assert(false);
  }


  Environment_set(Thread_getEnvironment(thread), argumentString, result);
}

void inst_push(Thread* thread, Argument argument) {
  char* argumentString = argument.string;

  if(strcmp(argumentString, "false") == 0) {
    Stack_push(&(thread->stack), (Object){ BOOLEAN, false });
  } else if(strcmp(argumentString, "pow") == 0) {
    Object result;
    result.type = BUILTIN;
    result.value.builtin = POW;
    Stack_push(&(thread->stack), result);
  } else if(strcmp(argumentString, "print") == 0) {
    Object result;
    result.type = BUILTIN;
    result.value.builtin = PRINT;
    Stack_push(&(thread->stack), result);
  } else if(strcmp(argumentString, "true") == 0) {
    Stack_push(&(thread->stack), (Object){ BOOLEAN, true });
  } else {
    Environment_get_Result result = Environment_get(
      Thread_getEnvironment(thread),
      argumentString
    );
    if(!result.found) {
      fprintf(stderr, "Variable `%s` not found", argumentString);
      assert(false);
    }
    Stack_push(&(thread->stack), result.result);
  }
}

void inst_push_integer(Thread* thread, Argument argument) {
  Object result;
  result.type = INTEGER;
  result.value.integer = argument.integer;

  Stack_push(&(thread->stack), result);
}

void inst_push_string(Thread* thread, Argument argument) {
  Object result;
  result.type = STRING;
  result.value.string = argument.string;

  Stack_push(&(thread->stack), result);
}

void inst_push_symbol(Thread* thread, Argument argument) {
  // TODO Store symbols in a symbol table so they can be looked up by reference
  // without string comparison
  Object result;
  result.type = SYMBOL;
  result.value.symbol = argument.symbol;

  Stack_push(&(thread->stack), result);
}

{% with name='sub', operation='-' %}
  {% include "arithmetic_instruction.c" %}
{% endwith %}

void inst_return(Thread* thread, Argument argument) {
  Frame* returnFrame = thread->frame.returnFrame;

  Frame_deinitialize(&(thread->frame));

  Frame_initialize(
    &(thread->frame),
    returnFrame->environment,
    returnFrame->returnFrame,
    returnFrame->programCounter
  );

  free(returnFrame);
}

void inst_structure(Thread* thread, Argument argument) {
  Object result;
  result.type = STRUCTURE;
  result.value.structure = NULL;

  int32_t count = argument.integer;

  while(count > 0) {
    assert(!Stack_isEmpty(&(thread->stack)));
    Object key = Stack_pop(&(thread->stack));
    assert(key.type == SYMBOL);

    assert(!Stack_isEmpty(&(thread->stack)));
    Object value = Stack_pop(&(thread->stack));

    Structure* node = malloc(sizeof(Structure));
    node->key = key.value.string;
    node->value = value;
    node->next = result.value.structure;

    result.value.structure = node;
    count--;
  }

  Stack_push(&(thread->stack), result);
}

struct Instruction;
typedef const struct Instruction Instruction;
struct Instruction {
  void (*instruction)(Thread*,Argument);
  Argument argument;
};

{% for label in labels_to_instruction_indices.keys() %}
#define LABEL_{{ label }} {{ labels_to_instruction_indices[label] }}
{% endfor %}

const Instruction program[] = {
{% for instruction in instruction_list %}
  (Instruction){ inst_{{ instruction.instruction }}, (Argument){{ generate_argument(instruction) }} },
{% endfor %}
};

int main() {
  Thread thread;
  Thread_initialize(&thread, LABEL___main__);

  for(; program[Thread_getProgramCounter(&thread)].instruction != inst_end; Thread_incrementProgramCounter(&thread)) {
    program[Thread_getProgramCounter(&thread)].instruction(
      &thread,
      program[Thread_getProgramCounter(&thread)].argument
    );
  }

  Thread_deinitialize(&thread);

  return 0;
}
