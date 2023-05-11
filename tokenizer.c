#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"

void Tokenizer_init(Tokenizer* self, const char* source) {
  self->source = source;
  self->current = source;
  self->line = 1;
}

inline static Token Token_create(TokenType type, const char* lexeme, size_t length, size_t line) {
  Token result;
  result.type = type;
  result.lexeme = lexeme;
  result.length = length;
  result.line = line;
  return result;
}

Token Tokenizer_consume(Tokenizer* self, TokenType type, size_t length) {
  const char* lexeme = self->current;
  self->current += length;

  /*
   * TODO
   * This won't work if we have line changes inside a token, but we'll cross
   * that bridge when we come to it.
   */
  return Token_create(type, lexeme, length, self->line);
}

inline static void Tokenizer_handleWhitespace(Tokenizer* self) {
  for(;;) {
    switch(*(self->current)) {
      case '\n':
        self->line++;
        // Fall through

      case ' ':
      case '\t':
      case '\r':
        self->current++;
        break;

      default:
        return;
    }
  }
}

Token Tokenizer_getToken(Tokenizer* self) {
  Tokenizer_handleWhitespace(self);

  switch(*(self->current)) {
    case '\0':
      return Tokenizer_consume(self, TOKEN_EOF, 0);

    case '+':
      return Tokenizer_consume(self, TOKEN_PLUS, 1);
    case '-':
      return Tokenizer_consume(self, TOKEN_MINUS, 1);
    case '*':
      return Tokenizer_consume(self, TOKEN_ASTERISK, 1);
    case '/':
      {
        if(self->current[1] == '/') {
          return Tokenizer_consume(self, TOKEN_SLASH_SLASH, 2);
        } else {
          break;
        }
      }


    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
        const char* lexeme = self->current;
        for(;;) {
          self->current++;
          switch(*(self->current)) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              break;

            default:
              return Token_create(TOKEN_INTEGER_LITERAL, lexeme, self->current - lexeme, self->line);
          }
        }
      }

    default:
      break;
  }

  return Token_create(TOKEN_ERROR, "Unexpected character", strlen("Unexpected character"), self->line);
}

#ifdef TEST

#include <assert.h>

void test_Tokenizer_getToken_eof() {
  const char* source = "";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_getToken(&tokenizer);

  assert(token.type == TOKEN_EOF);
  assert(token.lexeme == source);
  assert(token.length == 0);
  assert(token.line == 1);
}

void test_Tokenizer_getToken_unexpected_character() {
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

void test_Tokenizer_getToken_integer() {
  const char* source = "42";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_getToken(&tokenizer);

  assert(token.type == TOKEN_INTEGER_LITERAL);
  assert(token.lexeme == source);
  assert(token.length == 2);
  assert(token.line == 1);
}

void test_Tokenizer_getToken_ignore_whitespace() {
  const char* source = " \t\r42";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_getToken(&tokenizer);

  assert(token.type == TOKEN_INTEGER_LITERAL);
  assert(token.lexeme == source + 3);
  assert(token.length == 2);
  assert(token.line == 1);
}

void test_Tokenizer_getToken_linebreaks_increment_line() {
  const char* source = "\n42";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_getToken(&tokenizer);

  assert(token.type == TOKEN_INTEGER_LITERAL);
  assert(token.lexeme == source + 1);
  assert(token.length == 2);
  assert(token.line == 2);
}

void test_Tokenizer_getToken_integer_math_operators() {
  const char* source = "+ - * //";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token;

  token = Tokenizer_getToken(&tokenizer);
  assert(token.type == TOKEN_PLUS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_getToken(&tokenizer);
  assert(token.type == TOKEN_MINUS);
  assert(token.lexeme == source + 2);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_getToken(&tokenizer);
  assert(token.type == TOKEN_ASTERISK);
  assert(token.lexeme == source + 4);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_getToken(&tokenizer);
  assert(token.type == TOKEN_SLASH_SLASH);
  assert(token.lexeme == source + 6);
  assert(token.length == 2);
  assert(token.line == 1);
}
#endif
