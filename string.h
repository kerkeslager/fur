#ifndef STRING_H
#define STRING_H

#include <stdint.h>

#define STRING_REFCOUNT (SIZE_MAX >> 2)

typedef enum {
  UTF8_STRING,
  UTF8_CONCAT,
} UTF8Type;

typedef struct {
  UTF8Type type;
  size_t refCount;
} UTF8;

typedef struct {
  UTF8* ptr;
  size_t weight;
} WeightedUTF8Ptr;

typedef struct {
  UTF8 utf8;
  size_t byteCount;
  size_t length;
  uint8_t bytes[];
} UTF8String;

typedef struct {
  UTF8 utf8;
  WeightedUTF8Ptr left;
  WeightedUTF8Ptr right;
} UTF8Concat;

typedef enum {
  UTF32_STRING,
  UTF32_CONCAT,
} UTF32Type;

typedef struct {
  UTF32Type type;
  size_t refCount;
} UTF32;

typedef struct {
  UTF32* ptr;
  size_t weight;
} WeightedUTF32Ptr;

typedef struct {
  UTF32 utf32;
  size_t length;
  uint32_t codePoints[];
} UTF32String;

typedef struct {
  UTF32 utf32;
  WeightedUTF32Ptr left;
  WeightedUTF32Ptr right;
} UTF32Concat;

#ifdef TEST

#endif

#endif

