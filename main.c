#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>

#include "compiler.h"
#include "thread.h"
#include "value.h"
#include "value_stack.h"

int main(int argc, char *argv[]) {
  for(;;) {
    char* buffer = readline("> ");

    if (buffer && *buffer) {
      add_history(buffer);

      InstructionList byteCode;
      InstructionList_init(&byteCode);
      Compiler_compile((const char*)buffer, &byteCode);
      InstructionList_append(&byteCode, OP_RETURN);

      Thread thread;
      Thread_init(&thread, byteCode.items);
      Thread_run(&thread);

      Value result = ValueStack_pop(&(thread.stack));
      assert(result.type == VALUE_INTEGER);

      printf("  %i\n", result.as.integer);
      Thread_free(&thread);
      InstructionList_free(&byteCode);
      free(buffer);
    }
  }

  return 0;
}
