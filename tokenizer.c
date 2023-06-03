#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"

void Tokenizer_init(Tokenizer* self, const char* source) {
  self->source = source;
  self->current = source;
  self->line = 1;
  self->lookahead.type = NO_TOKEN;
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

bool isIdentifierChar(char c) {
  // TODO Allow more charcters in identifiers
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

Token Tokenizer_keywordOrIdentifier(Tokenizer* self, const char* lexeme, const char* suffix, TokenType type) {
  for(;;) {
    if(*suffix == '\0') {
      if(isIdentifierChar(*(self->current))) {
        self->current++;
        break;
      } else {
        return Token_create(type, lexeme, self->current - lexeme, self->line);
      }
    } else if(*suffix == *(self->current)) {
      suffix++;
      self->current++;
    } else {
      break;
    }
  }

  while(isIdentifierChar(*(self->current))) self->current++;

  return Token_create(TOKEN_SYMBOL, lexeme, self->current - lexeme, self->line);
}

Token Tokenizer_scan(Tokenizer* self) {
  if(self->lookahead.type != NO_TOKEN) {
    Token result = self->lookahead;
    self->lookahead.type = NO_TOKEN;
    return result;
  }

  Tokenizer_handleWhitespace(self);

  switch(*(self->current)) {
    case '\0':
      return Tokenizer_consume(self, TOKEN_EOF, 0);

    case ';':
      return Tokenizer_consume(self, TOKEN_SEMICOLON, 1);

    case '+':
      return Tokenizer_consume(self, TOKEN_PLUS, 1);
    case '-':
      return Tokenizer_consume(self, TOKEN_MINUS, 1);
    case '*':
      return Tokenizer_consume(self, TOKEN_ASTERISK, 1);
    case '/':
      if(self->current[1] == '/') {
        return Tokenizer_consume(self, TOKEN_SLASH_SLASH, 2);
      } else {
        return Token_create(TOKEN_ERROR, "Not implemented", strlen("Not implemented"), self->line);
      }

    case '<':
      if(self->current[1] == '=') {
        return Tokenizer_consume(self, TOKEN_LESS_THAN_EQUALS, 2);
      } else {
        return Tokenizer_consume(self, TOKEN_LESS_THAN, 1);
      }

    case '>':
      if(self->current[1] == '=') {
        return Tokenizer_consume(self, TOKEN_GREATER_THAN_EQUALS, 2);
      } else {
        return Tokenizer_consume(self, TOKEN_GREATER_THAN, 1);
      }

    case '=':
      if(self->current[1] == '=') {
        return Tokenizer_consume(self, TOKEN_EQUALS_EQUALS, 2);
      } else {
        return Tokenizer_consume(self, TOKEN_EQUALS, 1);
      }

    case '!':
      if(self->current[1] == '=') {
        return Tokenizer_consume(self, TOKEN_BANG_EQUALS, 2);
      } else {
        return Token_create(TOKEN_ERROR, "Not implemented", strlen("Not implemented"), self->line);
      }

    case '(':
      return Tokenizer_consume(self, TOKEN_OPEN_PAREN, 1);
    case ')':
      return Tokenizer_consume(self, TOKEN_CLOSE_PAREN, 1);

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

    case 'f':
      {
        const char* lexeme = self->current;
        self->current++;
        return Tokenizer_keywordOrIdentifier(self, lexeme, "alse", TOKEN_FALSE);
      }

    case 'n':
      {
        const char* lexeme = self->current;
        self->current++;
        return Tokenizer_keywordOrIdentifier(self, lexeme, "ot", TOKEN_NOT);
      }

    case 't':
      {
        const char* lexeme = self->current;
        self->current++;
        return Tokenizer_keywordOrIdentifier(self, lexeme, "rue", TOKEN_TRUE);
      }

    default:
      return Token_create(TOKEN_ERROR, "Unexpected character", strlen("Unexpected character"), self->line);
  }
}

Token Tokenizer_peek(Tokenizer* self) {
  if(self->lookahead.type == NO_TOKEN) {
    self->lookahead = Tokenizer_scan(self);
  }
  return self->lookahead;
}

#ifdef TEST

#include <assert.h>

void test_Tokenizer_scan_eof() {
  const char* source = "";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_scan(&tokenizer);

  assert(token.type == TOKEN_EOF);
  assert(token.lexeme == source);
  assert(token.length == 0);
  assert(token.line == 1);
}

void test_Tokenizer_scan_unexpected_character() {
  // There is currently no plan to use the backtick character for anything
  const char* source = "`";
  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_scan(&tokenizer);

  assert(token.type == TOKEN_ERROR);
  assert(strcmp(token.lexeme, "Unexpected character") == 0);
  assert(token.length == strlen("Unexpected character"));
  assert(token.line == 1);
}

void test_Tokenizer_scan_integer() {
  const char* source = "42";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_scan(&tokenizer);

  assert(token.type == TOKEN_INTEGER_LITERAL);
  assert(token.lexeme == source);
  assert(token.length == 2);
  assert(token.line == 1);
}

void test_Tokenizer_scan_ignore_whitespace() {
  const char* source = " \t\r42";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_scan(&tokenizer);

  assert(token.type == TOKEN_INTEGER_LITERAL);
  assert(token.lexeme == source + 3);
  assert(token.length == 2);
  assert(token.line == 1);
}

void test_Tokenizer_scan_linebreaks_increment_line() {
  const char* source = "\n42";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_scan(&tokenizer);

  assert(token.type == TOKEN_INTEGER_LITERAL);
  assert(token.lexeme == source + 1);
  assert(token.length == 2);
  assert(token.line == 2);
}

void test_Tokenizer_scan_integerMathOperators() {
  const char* source = "+ - * //";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token;

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_PLUS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_MINUS);
  assert(token.lexeme == source + 2);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_ASTERISK);
  assert(token.lexeme == source + 4);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_SLASH_SLASH);
  assert(token.lexeme == source + 6);
  assert(token.length == 2);
  assert(token.line == 1);
}

void test_Tokenizer_scan_semicolon() {
  const char* source = ";";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_SEMICOLON);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);
}

void test_Tokenizer_scan_equals() {
  const char* source = "=";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_EQUALS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);
}

void test_Tokenizer_peek_returnsScan() {
  const char* source = "+ - * //";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_peek(&tokenizer);
  assert(token.type == TOKEN_PLUS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);
}

void test_Tokenizer_peek_doesNotProgress() {
  const char* source = "+ - * //";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token;

  token = Tokenizer_peek(&tokenizer);
  assert(token.type == TOKEN_PLUS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_peek(&tokenizer);
  assert(token.type == TOKEN_PLUS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_peek(&tokenizer);
  assert(token.type == TOKEN_PLUS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_peek(&tokenizer);
  assert(token.type == TOKEN_PLUS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);
}

void test_Tokenizer_scan_after_Tokenizer_peek() {
  const char* source = "+ - * //";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token;

  token = Tokenizer_peek(&tokenizer);
  assert(token.type == TOKEN_PLUS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_PLUS);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_peek(&tokenizer);
  assert(token.type == TOKEN_MINUS);
  assert(token.lexeme == source + 2);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_MINUS);
  assert(token.lexeme == source + 2);
  assert(token.length == 1);
  assert(token.line == 1);
}

void test_Tokenizer_scan_parentheses() {
  const char* source = "()";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token;

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_OPEN_PAREN);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_CLOSE_PAREN);
  assert(token.lexeme == source + 1);
  assert(token.length == 1);
  assert(token.line == 1);
}

void test_Tokenizer_scan_identifier() {
  const char* source = "foo";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_SYMBOL);
  assert(token.lexeme == source);
  assert(token.length == 3);
  assert(token.line == 1);
}

void test_Tokenizer_scan_booleans() {
  const char* source = "true false";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token;

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_TRUE);
  assert(token.lexeme == source);
  assert(token.length == 4);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_FALSE);
  assert(token.lexeme == source + 5);
  assert(token.length == 5);
  assert(token.line == 1);
}

void test_Tokenizer_scan_differentiateKeywords() {
  const char* source = "tru fals truey falsey";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token;

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_SYMBOL);
  assert(token.lexeme == source);
  assert(token.length == 3);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_SYMBOL);
  assert(token.lexeme == source + 4);
  assert(token.length == 4);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_SYMBOL);
  assert(token.lexeme == source + 9);
  assert(token.length == 5);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_SYMBOL);
  assert(token.lexeme == source + 15);
  assert(token.length == 6);
  assert(token.line == 1);
}

void test_Tokenizer_scan_not() {
  const char* source = "not";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_NOT);
  assert(token.lexeme == source);
  assert(token.length == 3);
  assert(token.line == 1);
}

void test_Tokenizer_scan_comparisonOperators() {
  const char* source = "< > == <= >= !=";

  Tokenizer tokenizer;
  Tokenizer_init(&tokenizer, source);

  Token token;

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_LESS_THAN);
  assert(token.lexeme == source);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_GREATER_THAN);
  assert(token.lexeme == source + 2);
  assert(token.length == 1);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_EQUALS_EQUALS);
  assert(token.lexeme == source + 4);
  assert(token.length == 2);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_LESS_THAN_EQUALS);
  assert(token.lexeme == source + 7);
  assert(token.length == 2);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_GREATER_THAN_EQUALS);
  assert(token.lexeme == source + 10);
  assert(token.length == 2);
  assert(token.line == 1);

  token = Tokenizer_scan(&tokenizer);
  assert(token.type == TOKEN_BANG_EQUALS);
  assert(token.lexeme == source + 13);
  assert(token.length == 2);
  assert(token.line == 1);
}

#endif
