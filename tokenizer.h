#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "stack.h"

typedef enum {
  TOKEN_INTEGER_LITERAL,

  TOKEN_SEMICOLON,

  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_ASTERISK,
  TOKEN_SLASH_SLASH,

  TOKEN_OPEN_PAREN,
  TOKEN_CLOSE_PAREN,

  TOKEN_TRUE,
  TOKEN_FALSE,

  TOKEN_IDENTIFIER,

  TOKEN_ERROR,
  TOKEN_EOF,

  /*
   * This is used internally to the tokenizer and parser to indicate that
   * there is no token, for lookaheads or matching outfix operators.
   */
  NO_TOKEN,
} TokenType;

typedef struct {
  TokenType type;
  const char* lexeme;
  size_t length;
  size_t line;
} Token;

typedef struct {
  const char* source;
  const char* current;
  size_t line;
  Token lookahead;
} Tokenizer;

void Tokenizer_init(Tokenizer* self, const char* source);
Token Tokenizer_scan(Tokenizer* self);
Token Tokenizer_peek(Tokenizer* self);

#ifdef TEST

void test_Tokenizer_scan_eof();
void test_Tokenizer_scan_unexpected_character();
void test_Tokenizer_scan_integer();
void test_Tokenizer_scan_ignore_whitespace();
void test_Tokenizer_scan_linebreaks_increment_line();
void test_Tokenizer_scan_integerMathOperators();
void test_Tokenizer_scan_semicolon();
void test_Tokenizer_peek_returnsScan();
void test_Tokenizer_peek_doesNotProgress();
void test_Tokenizer_scan_after_Tokenizer_peek();
void test_Tokenizer_scan_parentheses();
void test_Tokenizer_scan_identifier();
void test_Tokenizer_scan_booleans();
void test_Tokenizer_scan_differentiateKeywords();

#endif

#endif
