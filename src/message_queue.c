#include "message_queue.h"

#ifdef TEST
void test_MessageQueue_init() {
  MessageQueue mq;
  MessageQueue_init(&mq);
  assert(mq.first == NULL);
  assert(mq.last == NULL);
}
#endif
