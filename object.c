#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "object.h"

#ifdef TEST

void Obj_init(Obj* self, ObjType type, Obj* next) {
  self->type = type;
  self->next = next;
}

void Obj_del(Obj* self) {
  switch(self->type) {
    case OBJ_UTF8_STRING:
    case OBJ_UTF8_CONCAT:
    case OBJ_UTF32_STRING:
    case OBJ_UTF32_CONCAT:
      /*
       * These objects may have children, but freeing those children is
       * handled by the garbage collector if they are allocated at runtime,
       * or by freeing the module if they are interned objects.
       */
      free(self);
      return;

    case OBJ_BIGINT:
      mpz_clear(((ObjBigInt*)self)->bigInt);
      free(self);
      return;
  }
}

#endif
