#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "value.h"

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

#endif
