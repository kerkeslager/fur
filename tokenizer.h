#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum {
  TOKEN_INTEGER_LITERAL,

  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_ASTERISK,
  TOKEN_SLASH_SLASH,

  TOKEN_ERROR,
  TOKEN_EOF,
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
} Tokenizer;

void Tokenizer_init(Tokenizer* self, const char* source);
Token Tokenizer_scan(Tokenizer* self);

#ifdef TEST

void test_Tokenizer_scan_eof();
void test_Tokenizer_scan_unexpected_character();
void test_Tokenizer_scan_integer();
void test_Tokenizer_scan_ignore_whitespace();
void test_Tokenizer_scan_linebreaks_increment_line();
void test_Tokenizer_scan_integer_math_operators();

#endif

#endif
