#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>

#include "compiler.h"
#include "thread.h"
#include "value.h"
#include "value_stack.h"

int main() {
  for(;;) {
    char* buffer = readline("> ");

    if (buffer && *buffer) {
      add_history(buffer);

      InstructionList byteCode;
      InstructionList_init(&byteCode);
      Compiler_compile(&byteCode, (const char*)buffer);

      Thread thread;
      Thread_init(&thread, byteCode.items);
      Thread_run(&thread);

      Value result = ValueStack_pop(&(thread.stack));
      Value_println(result);

      Thread_free(&thread);
      InstructionList_free(&byteCode);
      free(buffer);
    }
  }

  return 0;
}
