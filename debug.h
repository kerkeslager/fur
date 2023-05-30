#ifndef DEBUG_H
#define DEBUG_H

#include "tokenizer.h"

#include <stdio.h>

#define PRINT_CASE(tt) case tt: \
  printf("%5zu %-25s %.*s\n", token.line, #tt, (int)token.length, token.lexeme); \
  break
inline static void Token_println(Token token) {
  switch(token.type) {
    PRINT_CASE(TOKEN_INTEGER_LITERAL);

    PRINT_CASE(TOKEN_PLUS);
    PRINT_CASE(TOKEN_MINUS);
    PRINT_CASE(TOKEN_ASTERISK);
    PRINT_CASE(TOKEN_SLASH_SLASH);

    PRINT_CASE(TOKEN_LESS_THAN);
    PRINT_CASE(TOKEN_LESS_THAN_EQUALS);
    PRINT_CASE(TOKEN_GREATER_THAN);
    PRINT_CASE(TOKEN_GREATER_THAN_EQUALS);
    PRINT_CASE(TOKEN_EQUALS_EQUALS);
    PRINT_CASE(TOKEN_BANG_EQUALS);

    PRINT_CASE(TOKEN_SEMICOLON);

    PRINT_CASE(TOKEN_OPEN_PAREN);
    PRINT_CASE(TOKEN_CLOSE_PAREN);

    PRINT_CASE(TOKEN_TRUE);
    PRINT_CASE(TOKEN_FALSE);

    PRINT_CASE(TOKEN_IDENTIFIER);

    PRINT_CASE(TOKEN_ERROR);
    PRINT_CASE(TOKEN_EOF);

    PRINT_CASE(NO_TOKEN);
  }
}
#undef PRINT_CASE

#define PRINT_CASE(tt) case tt: printf("Token: " #tt "\n"); break
inline static void TokenType_println(TokenType tokenType) {
  switch(tokenType) {
    PRINT_CASE(TOKEN_INTEGER_LITERAL);

    PRINT_CASE(TOKEN_PLUS);
    PRINT_CASE(TOKEN_MINUS);
    PRINT_CASE(TOKEN_ASTERISK);
    PRINT_CASE(TOKEN_SLASH_SLASH);

    PRINT_CASE(TOKEN_LESS_THAN);
    PRINT_CASE(TOKEN_LESS_THAN_EQUALS);
    PRINT_CASE(TOKEN_GREATER_THAN);
    PRINT_CASE(TOKEN_GREATER_THAN_EQUALS);
    PRINT_CASE(TOKEN_EQUALS_EQUALS);
    PRINT_CASE(TOKEN_BANG_EQUALS);

    PRINT_CASE(TOKEN_SEMICOLON);

    PRINT_CASE(TOKEN_OPEN_PAREN);
    PRINT_CASE(TOKEN_CLOSE_PAREN);

    PRINT_CASE(TOKEN_TRUE);
    PRINT_CASE(TOKEN_FALSE);

    PRINT_CASE(TOKEN_IDENTIFIER);

    PRINT_CASE(TOKEN_ERROR);
    PRINT_CASE(TOKEN_EOF);

    PRINT_CASE(NO_TOKEN);
  }
}
#undef PRINT_CASE

#endif
