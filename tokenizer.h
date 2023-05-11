#ifndef TOKENIZER_H
#define TOKENIZER_H

struct Token;
typedef struct Token Token;

struct Tokenizer;
typedef struct Tokenizer Tokenizer;

void Tokenizer_init(Tokenizer* self, const char* source);
Token Tokenizer_getToken(Tokenizer* self);

#ifdef TEST

void test_Tokenizer_getToken_eof();
void test_Tokenizer_getToken_unexpected_character();
void test_Tokenizer_getToken_integer();
void test_Tokenizer_getToken_ignore_whitespace();
void test_Tokenizer_getToken_linebreaks_increment_line();
void test_Tokenizer_getToken_integer_math_operators();

#endif

#endif
