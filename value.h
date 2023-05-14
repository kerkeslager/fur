#ifndef VALUE_H
#define VALUE_H

typedef enum {
  V_INTEGER
} ValueType;

typedef struct {
  ValueType type;
  union {
    int32_t integer;
  } as;
} Value;

#ifdef TEST

#endif

#endif
