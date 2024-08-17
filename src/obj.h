#ifndef OBJ_H
#define OBJ_H

#include <stdbool.h>

struct Obj;
typedef struct Obj Obj;

typedef enum {
  OBJ_BYTES,
  OBJ_BYTES_CONCAT,
  OBJ_UTF8,
  OBJ_UTF8_CONCAT,
  OBJ_UTF16,
  OBJ_UTF16_CONCAT,
  OBJ_UTF32,
  OBJ_UTF32_CONCAT,
  OBJ_LIST,
  OBJ_DICT,
  OBJ_STRUCT
} ObjType;

struct Obj {
  Obj* next;
  bool marked;
  ObjType type;
};

#endif
