#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "value.h"

#include <stdlib.h>

struct Message;
typedef struct Message Message;

struct Message {
  Value item;
  Message* next;
};

typedef struct {
  Message* first;
  Message* last;
} MessageQueue;

inline static void MessageQueue_init(MessageQueue* self) {
  self->first = NULL;
  self->last = NULL;
}

#ifdef TEST
void test_MessageQueue_init();
#endif

#endif
