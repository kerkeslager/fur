#ifndef OBJECT_H
#define OBJECT_H

#include "string.h"

typedef enum {
  OBJ_UTF8_STRING,
  OBJ_UTF8_CONCAT,
  OBJ_UTF32_STRING,
  OBJ_UTF32_CONCAT,
} ObjType;

struct Obj;
typedef struct Obj Obj;
struct Obj{
  ObjType type;
  Obj* next;
};

typedef struct {
  Obj obj;
  size_t byteCount;
  size_t length;
  uint8_t bytes[];
} ObjUTF8String;

typedef struct {
  Obj obj;
  ObjUTF8String* child0;
  ObjUTF8String* child1;
} ObjUTF8Concat;

typedef struct {
  Obj obj;
  size_t length;
  uint32_t codePoints[];
} ObjUTF32String;

typedef struct {
  Obj obj;
  ObjUTF32String* child0;
  ObjUTF32String* child1;
} ObjUTF32Concat;

#ifdef TEST

#endif

#endif
