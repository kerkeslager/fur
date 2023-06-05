#include <assert.h>
#include <string.h>

#include "symbol_table.h"

static uint32_t hashSymbol(const char* text, int length) {
  /*
   * This is the FNV-1a hash algorithm.
   */
  uint32_t hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)text[i];
    hash *= 16777619;
  }

  return hash;
}

inline static size_t logBase2(size_t n) {
  size_t lb2 = 0;

  while(n > 1) {
    lb2++;
    n >>= 1;
  }

  return lb2;
}

inline static size_t getIndex(uint32_t hash, size_t capacity) {
  /*
   * This magic number is 2^64 / phi, where phi is the golden ratio.
   *
   * See https://probablydance.com/2018/06/16/fibonacci-hashing-the-optimization-that-the-world-forgot-or-a-better-alternative-to-integer-modulo/
   * for more information.
   */
  return (11400714819323198485llu * (unsigned long long)hash) >> (64 - logBase2(capacity));
}

Symbol* SymbolTable_getOrCreate(SymbolTable* self, const char* text, size_t length) {
  if(self->capacity == 0) {
    self->capacity = 16;
    self->items = calloc(self->capacity, sizeof(Symbol*));
  } else if(self->load >= self->capacity * 0.75) {
    size_t capacity = self->capacity;
    Symbol** items = self->items;

    self->capacity *= 2;
    self->items = calloc(self->capacity, sizeof(Symbol*));

    for(size_t i = 0; i < capacity; i++) {
      if(items[i] != NULL) {
        size_t index = getIndex(items[i]->hash, self->capacity);

        while(self->items[index] != NULL) {
          index++;
          if(index == self->capacity) index = 0;
        }

        self->items[index] = items[i];
      }
    }

    free(items);
  }

  uint32_t hash = hashSymbol(text, length);
  size_t index = getIndex(hash, self->capacity);

  for(;;) {
    if(self->items[index] == NULL) {
      self->items[index] = Symbol_new(text, length, hashSymbol(text, length));
      self->load++;
      return self->items[index];
    } else {
      Symbol* symbol = self->items[index];
      if(symbol->hash == hash && symbol->length == length && !strncmp(symbol->text, text, length)) {
        return symbol;
      }

      index++;
      if(index == self->capacity) index = 0;
    }
  }
}

#ifdef TEST

void test_SymbolTable_init() {
  SymbolTable st;
  SymbolTable_init(&st);

  assert(st.items == NULL);
  assert(st.load == 0);
  assert(st.capacity == 0);

  SymbolTable_free(&st);
}

void test_SymbolTable_addFirst() {
  SymbolTable st;
  SymbolTable_init(&st);
  const char* symbolText = "Hello, world"; // not a valid symbol, but it doesn't matter
  size_t symbolLength = strlen(symbolText);

  Symbol* symbol = SymbolTable_getOrCreate(&st, symbolText, symbolLength);

  assert(symbol != NULL);
  assert(symbol->length == symbolLength);
  assert(!strncmp(symbol->text, symbolText, symbolLength));
  assert(symbol->hash == 2497247677);

  assert(st.items != NULL);
  assert(st.load == 1);
  assert(st.capacity == 16);

  for(int i = 0; i < 16; i++) {
    if(i == 11) {
      assert(st.items[i] == symbol);
    } else {
      assert(st.items[i] == NULL);
    }
  }

  SymbolTable_free(&st);
}

void test_SymbolTable_sameSymbolsSamePointers() {
  SymbolTable st;
  SymbolTable_init(&st);
  const char* symbol0Text = "Hello, world";
  size_t symbol0Length = strlen(symbol0Text);
  const char* symbol1Text = "Hello, world";
  size_t symbol1Length = strlen(symbol1Text);

  Symbol* symbol0 = SymbolTable_getOrCreate(&st, symbol0Text, symbol0Length);
  Symbol* symbol1 = SymbolTable_getOrCreate(&st, symbol1Text, symbol1Length);

  assert(symbol0 == symbol1);
  assert(st.load == 1);

  SymbolTable_free(&st);
}

void test_SymbolTable_differentSymbolsDifferentPointers() {
  SymbolTable st;
  SymbolTable_init(&st);
  const char* symbol0Text = "Hello, world";
  size_t symbol0Length = strlen(symbol0Text);
  const char* symbol1Text = "Goodnight, moon";
  size_t symbol1Length = strlen(symbol1Text);

  Symbol* symbol0 = SymbolTable_getOrCreate(&st, symbol0Text, symbol0Length);
  Symbol* symbol1 = SymbolTable_getOrCreate(&st, symbol1Text, symbol1Length);

  assert(symbol0 != symbol1);
  assert(st.load == 2);

  SymbolTable_free(&st);
}

void test_SymbolTable_growth() {
  SymbolTable st;
  SymbolTable_init(&st);

  char* symbolTexts[12] = {
    "Goodnight, moon",
    "Goodnight, stars",
    "Goodnight, po-pos",
    "Goodnight, fiends",
    "Goodnight, hoppers",
    "Goodnight, hustlers",
    "Goodnight, scammers",
    "Goodnight, to everybody",
    "Goodnight to one and all",
    "--Kima",
    "*The Wire*",
    "by David Simon",
  };

  Symbol* symbols[12];

  for(int i = 0; i < 12; i++) {
    symbols[i] = SymbolTable_getOrCreate(&st, symbolTexts[i], strlen(symbolTexts[i]));
  }

  assert(st.load == 12);
  assert(st.capacity == 16);

  SymbolTable_getOrCreate(&st, "Hello, world", strlen("Hello, world"));

  assert(st.load == 13);
  assert(st.capacity == 32);

  for(int i = 0; i < 12; i++) {
    Symbol* symbol = SymbolTable_getOrCreate(&st, symbolTexts[i], strlen(symbolTexts[i]));
    assert(symbols[i] == symbol);
  }

  SymbolTable_free(&st);
}

#endif
