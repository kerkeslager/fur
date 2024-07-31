#ifndef BLOB_H
#define BLOB_H

#include <assert.h>
#include <stdlib.h>

typedef struct {
  size_t count;
  uint8_t bytes[];
} Blob;

typedef struct {
  size_t count;
  size_t capacity;
  Blob** items;
} BlobList;

inline static void BlobList_init(BlobList* self) {
  self->count = 0;
  self->capacity = 0;
  self->items = NULL;
}

inline static void BlobList_free(BlobList* self) {
  free(self->items);
}

inline static size_t BlobList_append(BlobList* self, Blob* item) {
  if(self->capacity == self->count) {
    if(self->capacity == 0) {
      self->capacity = 8;
    } else {
      self->capacity *= 2;
    }

    self->items = realloc(self->items, self->capacity * sizeof(Blob*));
    assert(self->items != NULL);
  }

  self->items[self->count] = item;
  return self->count++;
}

#endif
