#ifndef TOKENIZER_H
#define TOKENIZER_H

struct Token;
typedef struct Token Token;

struct Tokenizer;
typedef struct Tokenizer Tokenizer;

void Tokenizer_init(Tokenizer* self, const char* source);
Token Tokenizer_getToken(Tokenizer* self);

#ifdef TEST

void test_eof();
void test_unexpected_character();
void test_integer();
void test_ignore_whitespace();
void test_linebreaks_increment_line();

#endif

#endif
