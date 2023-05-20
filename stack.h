#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <stdbool.h>

#define STACK_DECLARE(itemType) \
  typedef struct { \
    size_t capacity; \
    itemType* items; \
    size_t top; \
  } itemType##Stack; \
  \
  void itemType##Stack_init(itemType##Stack*); \
  void itemType##Stack_free(itemType##Stack*); \
  \
  inline static bool itemType##Stack_isEmpty(itemType##Stack* self) { \
    return self->top == 0; \
  } \
  \
  inline static void itemType##Stack_push(itemType##Stack* self, itemType item) { \
    if(self->top == self->capacity) { \
      self->capacity *= 2; \
      self->items = realloc(self->items, sizeof(itemType) * self->capacity); \
      assert(self->items != NULL); \
    } \
    \
    self->items[self->top++] = item; \
  } \
  \
  inline static itemType itemType##Stack_pop(itemType##Stack* self) { \
    assert(!itemType##Stack_isEmpty(self)); \
    return self->items[--(self->top)]; \
  } \
  \
  inline static itemType itemType##Stack_peek(itemType##Stack* self) { \
    assert(!itemType##Stack_isEmpty(self)); \
    return self->items[self->top - 1]; \
  }

#define STACK_IMPLEMENT(itemType, initialCapacity) \
  void itemType##Stack_init(itemType##Stack* self) { \
    self->capacity = initialCapacity; \
    self->items = malloc(sizeof(itemType) * self->capacity); \
    self->top = 0; \
  } \
  \
  void itemType##Stack_free(itemType##Stack* self) { \
    free(self->items); \
  }

#endif
