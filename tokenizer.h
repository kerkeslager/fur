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

#endif

#endif
