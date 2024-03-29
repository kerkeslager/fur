#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "stack.h"

typedef enum {
  TOKEN_INTEGER_LITERAL,

  TOKEN_CODE_POINT_LITERAL,
  TOKEN_UTF8_LITERAL,
  TOKEN_UTF32_LITERAL,

  TOKEN_EQUALS,
  TOKEN_SEMICOLON,
  TOKEN_COMMA,

  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_ASTERISK,
  TOKEN_SLASH_SLASH,

  TOKEN_LESS_THAN,
  TOKEN_LESS_THAN_EQUALS,
  TOKEN_GREATER_THAN,
  TOKEN_GREATER_THAN_EQUALS,
  TOKEN_EQUALS_EQUALS,
  TOKEN_BANG_EQUALS,

  TOKEN_AND,
  TOKEN_OR,

  TOKEN_OPEN_PAREN,
  TOKEN_CLOSE_PAREN,

  TOKEN_OPEN_BRACE,
  TOKEN_CLOSE_BRACE,

  TOKEN_NIL,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_NOT,

  TOKEN_SYMBOL,

  TOKEN_LOOP,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_UNTIL,

  TOKEN_CONTINUE,
  TOKEN_BREAK,
  TOKEN_WITH,

  TOKEN_MUT,

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
  bool hasLookahead;
} Tokenizer;

void Tokenizer_init(Tokenizer*, const char* source, size_t startLine);
Token Tokenizer_scan(Tokenizer*);
Token Tokenizer_peek(Tokenizer*);
void Tokenizer_appendLine(Tokenizer*, const char*);

#ifdef TEST

void test_Tokenizer_scan_eof();
void test_Tokenizer_scan_unexpected_character();
void test_Tokenizer_scan_integer();
void test_Tokenizer_scan_ignore_whitespace();
void test_Tokenizer_scan_linebreaks_increment_line();
void test_Tokenizer_scan_integerMathOperators();
void test_Tokenizer_scan_semicolon();
void test_Tokenizer_scan_comma();
void test_Tokenizer_scan_equals();
void test_Tokenizer_scan_after_Tokenizer_peek();
void test_Tokenizer_scan_parentheses();
void test_Tokenizer_scan_braces();
void test_Tokenizer_scan_symbol();
void test_Tokenizer_scan_nil();
void test_Tokenizer_scan_booleans();
void test_Tokenizer_scan_differentiateKeywords();
void test_Tokenizer_scan_not();
void test_Tokenizer_scan_comparisonOperators();
void test_Tokenizer_scan_booleanOperators();
void test_Tokenizer_scan_jumpKeywords();
void test_Tokenizer_scan_mut();
void test_Tokenizer_scan_continue();
void test_Tokenizer_scan_breakWith();

void test_Tokenizer_scan_singleQuoteString();
void test_Tokenizer_scan_doubleQuoteString();
void test_Tokenizer_scan_singleQuoteUTF8();
void test_Tokenizer_scan_doubleQuoteUTF8();
void test_Tokenizer_scan_singleQuoteUTF32();
void test_Tokenizer_scan_doubleQuoteUTF32();
void test_Tokenizer_scan_singleQuoteCodePoint();
void test_Tokenizer_scan_doubleQuoteCodePoint();

void test_Tokenizer_peek_returnsScan();
void test_Tokenizer_peek_doesNotProgress();

#endif

#endif
