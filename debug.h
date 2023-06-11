#ifndef DEBUG_H
#define DEBUG_H

#include "tokenizer.h"
#include "node.h"

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

    PRINT_CASE(TOKEN_EQUALS);

    PRINT_CASE(TOKEN_LESS_THAN);
    PRINT_CASE(TOKEN_LESS_THAN_EQUALS);
    PRINT_CASE(TOKEN_GREATER_THAN);
    PRINT_CASE(TOKEN_GREATER_THAN_EQUALS);
    PRINT_CASE(TOKEN_EQUALS_EQUALS);
    PRINT_CASE(TOKEN_BANG_EQUALS);

    PRINT_CASE(TOKEN_SEMICOLON);

    PRINT_CASE(TOKEN_OPEN_PAREN);
    PRINT_CASE(TOKEN_CLOSE_PAREN);
    PRINT_CASE(TOKEN_OPEN_BRACE);
    PRINT_CASE(TOKEN_CLOSE_BRACE);

    PRINT_CASE(TOKEN_TRUE);
    PRINT_CASE(TOKEN_FALSE);
    PRINT_CASE(TOKEN_NOT);

    PRINT_CASE(TOKEN_SYMBOL);

    PRINT_CASE(TOKEN_LOOP);
    PRINT_CASE(TOKEN_IF);
    PRINT_CASE(TOKEN_ELSE);
    PRINT_CASE(TOKEN_WHILE);
    PRINT_CASE(TOKEN_UNTIL);

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

    PRINT_CASE(TOKEN_EQUALS);

    PRINT_CASE(TOKEN_LESS_THAN);
    PRINT_CASE(TOKEN_LESS_THAN_EQUALS);
    PRINT_CASE(TOKEN_GREATER_THAN);
    PRINT_CASE(TOKEN_GREATER_THAN_EQUALS);
    PRINT_CASE(TOKEN_EQUALS_EQUALS);
    PRINT_CASE(TOKEN_BANG_EQUALS);

    PRINT_CASE(TOKEN_SEMICOLON);

    PRINT_CASE(TOKEN_OPEN_PAREN);
    PRINT_CASE(TOKEN_CLOSE_PAREN);
    PRINT_CASE(TOKEN_OPEN_BRACE);
    PRINT_CASE(TOKEN_CLOSE_BRACE);

    PRINT_CASE(TOKEN_TRUE);
    PRINT_CASE(TOKEN_FALSE);
    PRINT_CASE(TOKEN_NOT);

    PRINT_CASE(TOKEN_SYMBOL);

    PRINT_CASE(TOKEN_LOOP);
    PRINT_CASE(TOKEN_IF);
    PRINT_CASE(TOKEN_ELSE);
    PRINT_CASE(TOKEN_WHILE);
    PRINT_CASE(TOKEN_UNTIL);

    PRINT_CASE(TOKEN_ERROR);
    PRINT_CASE(TOKEN_EOF);

    PRINT_CASE(NO_TOKEN);
  }
}
#undef PRINT_CASE

#define PRINT_CASE(nt) case nt: printf("NodeType: " #nt "\n"); break
inline static void NodeType_println(NodeType nodeType) {
  switch(nodeType) {
    PRINT_CASE(NODE_INTEGER_LITERAL);
    PRINT_CASE(NODE_BOOLEAN_LITERAL);
    PRINT_CASE(NODE_SYMBOL);

    PRINT_CASE(NODE_ASSIGN);

    PRINT_CASE(NODE_NEGATE);
    PRINT_CASE(NODE_LOGICAL_NOT);

    PRINT_CASE(NODE_ADD);
    PRINT_CASE(NODE_SUBTRACT);
    PRINT_CASE(NODE_MULTIPLY);
    PRINT_CASE(NODE_INTEGER_DIVIDE);

    PRINT_CASE(NODE_LESS_THAN);
    PRINT_CASE(NODE_LESS_THAN_EQUAL);
    PRINT_CASE(NODE_GREATER_THAN);
    PRINT_CASE(NODE_GREATER_THAN_EQUAL);
    PRINT_CASE(NODE_EQUAL);
    PRINT_CASE(NODE_NOT_EQUAL);

    PRINT_CASE(NODE_LOOP);
    PRINT_CASE(NODE_IF);
    PRINT_CASE(NODE_WHILE);
    PRINT_CASE(NODE_UNTIL);

    PRINT_CASE(NODE_ERROR);
    PRINT_CASE(NODE_EOF);
  }
}

#undef PRINT_CASE

#endif
