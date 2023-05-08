#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"

struct Tokenizer {
  const char* source;
  const char* current;
  size_t line;
};

void Tokenizer_init(Tokenizer* self, const char* source) {
  self->source = source;
  self->current = source;
  self->line = 1;
}

typedef enum {
  TOKEN_INTEGER,

  TOKEN_ERROR,
  TOKEN_EOF,
} TokenType;

struct Token {
  TokenType type;
  const char* lexeme;
  size_t length;
  size_t line;
};

Token Token_create(TokenType type, const char* lexeme, size_t length, size_t line) {
  Token result;
  result.type = type;
  result.lexeme = lexeme;
  result.length = length;
  result.line = line;
  return result;
}

Token Tokenizer_getToken(Tokenizer* self) {
  switch(*(self->current)) {
    case '\0':
      return Token_create(TOKEN_EOF, self->current, 0, self->line);

    default:
      return Token_create(TOKEN_ERROR, "Unexpected character", strlen("Unexpected character"), self->line);
  }
}

#ifdef TEST

#include <assert.h>

void test_eof() {
  const char* source = "";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_getToken(&tokenizer);

  assert(token.type == TOKEN_EOF);
  assert(token.lexeme == source);
  assert(token.length == 0);
  assert(token.line == 1);
}

void test_unexpected_character() {
  // There is currently no plan to use the backtick character for anything
  const char* source = "`";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_getToken(&tokenizer);

  assert(token.type == TOKEN_ERROR);
  assert(strcmp(token.lexeme, "Unexpected character") == 0);
  assert(token.length == strlen("Unexpected character"));
  assert(token.line == 1);
}

#endif
