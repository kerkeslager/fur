#include <assert.h>
#include <stdbool.h>

#include "symbol_list.h"

int32_t SymbolList_find(SymbolList* self, Symbol* symbol) {
  for(size_t i = 0; i < self->count; i++) {
    if(self->items[i] == symbol) {
      return i;
    }
  }

  return -1;
}

void SymbolList_append(SymbolList* self, Symbol* symbol, bool isMutable) {
  // TODO Handle this better
  assert(SymbolList_find(self, symbol) == -1);

  if(self->count == self->capacity) {
    uint32_t capacity = self->capacity;

    // TODO Handle this better
    assert(capacity != UINT16_MAX);

    if(capacity == 0) capacity = 16;
    else capacity *= 2;

    if(capacity > UINT16_MAX) capacity = UINT16_MAX;

    self->capacity = capacity;
    self->items = realloc(self->items, self->capacity * sizeof(Symbol*));
    self->isMutables = realloc(self->isMutables, self->capacity * sizeof(bool));

    // TODO Handle this better
    assert(self->items != NULL);
  }

  self->items[self->count] = symbol;
  self->isMutables[self->count] = isMutable;
  self->count++;
}

#ifdef TEST

#include<string.h>

void test_SymbolList_init() {
  SymbolList symbolList;
  SymbolList_init(&symbolList);

  assert(symbolList.items == NULL);
  assert(symbolList.count == 0);
  assert(symbolList.capacity == 0);

  SymbolList_free(&symbolList);
}

static const char* SYMBOLS[100] = {
  "time",
  "year",
  "people",
  "way",
  "day",
  "man",
  "thing",
  "woman",
  "life",
  "child",
  "world",
  "school",
  "state",
  "family",
  "student",
  "group",
  "country",
  "problem",
  "hand",
  "part",
  "place",
  "case",
  "week",
  "company",
  "system",
  "program",
  "question",
  "work",
  "government",
  "number",
  "night",
  "point",
  "home",
  "water",
  "room",
  "mother",
  "area",
  "money",
  "story",
  "fact",
  "month",
  "lot",
  "right",
  "study",
  "book",
  "eye",
  "job",
  "word",
  "business",
  "issue",
  "side",
  "kind",
  "head",
  "house",
  "service",
  "friend",
  "father",
  "power",
  "hour",
  "game",
  "line",
  "end",
  "member",
  "law",
  "car",
  "city",
  "community",
  "name",
  "president",
  "team",
  "minute",
  "idea",
  "kid",
  "body",
  "information",
  "back",
  "parent",
  "face",
  "others",
  "level",
  "office",
  "door",
  "health",
  "person",
  "art",
  "war",
  "history",
  "party",
  "result",
  "change",
  "morning",
  "reason",
  "research",
  "girl",
  "guy",
  "moment",
  "air",
  "teacher",
  "force",
  "education",
};

void test_SymbolList_append_many() {
  SymbolList symbolList;
  SymbolList_init(&symbolList);

  Symbol* symbols[100];

  for(int i = 0; i < 100; i++) {
    symbols[i] = Symbol_new(SYMBOLS[i], strlen(SYMBOLS[i]), 0 /* not real hash */);
  }

  for(int i = 0; i < 100; i++) {
    SymbolList_append(&symbolList, symbols[i], false);
  }

  for(int i = 0; i < 100; i++) {
    assert(SymbolList_find(&symbolList, symbols[i]) == i);
  }

  assert(symbolList.count == 100);
  assert(symbolList.capacity == 128);

  SymbolList_free(&symbolList);
}

void test_SymbolList_append_allowsUpToUINT16_MAXsymbols() {
  SymbolList symbolList;
  SymbolList_init(&symbolList);

  // This is a hack to avoid inserting this many symbols into the list
  uint16_t capacity = UINT16_MAX / 2 + 1;
  symbolList.count = capacity;
  symbolList.capacity = capacity;

  /*
   * Stick hacking--use calloc so the items are null. This avoids the
   * possibility that an assertion that the item doesn't already exits
   * will find the item by random chance.
   */
  symbolList.items = calloc(capacity, sizeof(Symbol*));

  Symbol* symbol = Symbol_new("hello", strlen("hello"), 0 /* not real hash */);

  SymbolList_append(&symbolList, symbol, false);

  assert(symbolList.count = capacity + 1);
  assert(symbolList.capacity = UINT16_MAX);

  // Can't use SymbolList_free() due to previous hack
  free(symbolList.items);
  free(symbol);
}

void test_SymbolList_isMutable() {
  SymbolList symbolList;
  SymbolList_init(&symbolList);

  Symbol* symbols[100];

  for(int i = 0; i < 100; i++) {
    symbols[i] = Symbol_new(SYMBOLS[i], strlen(SYMBOLS[i]), 0 /* not real hash */);
  }

  for(int i = 0; i < 100; i++) {
    SymbolList_append(&symbolList, symbols[i], i % 3 == 0);
  }

  for(int i = 0; i < 100; i++) {
    assert(
      SymbolList_isMutable(
        &symbolList,
        SymbolList_find(&symbolList, symbols[i])
      ) == (i % 3 == 0)
    );
  }

  SymbolList_free(&symbolList);
}

#endif
