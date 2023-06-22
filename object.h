#ifndef OBJECT_H
#define OBJECT_H

#include "string.h"

typedef enum {
  OBJ_UTF8,
  OBJ_UTF32,
} ObjType;

typedef struct {
  ObjType type;
  Obj* next;
} Obj;

typedef struct {
  Obj obj;
  WeightedUTF8Ptr ptr;
} ObjUTF8;

typedef struct {
  Obj obj;
  WeightedUTF32Ptr ptr;
} ObjUTF32;

#ifdef TEST

#endif

#endif
