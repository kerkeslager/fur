#ifndef TEXT_H
#define TEXT_H

#define FMT_EXPECTED_CLOSE_OUTFIX \
  "Expected \"%s\" to close \"%.*s\" from line %zu, but received \"%.*s\"."
#define FMT_EXPECTED_OPEN_PAREN \
  "Unexpected token \"%.*s\". Expected \"(\"."
#define FMT_REASSIGNING_IMMUTABLE_VARIABLE \
  "Reassigning immutable variable `%.*s` after definition on line %zu."
#define FMT_REDECLARATION \
  "Re-declaring symbol `%.*s` already declared on line %zu."
#define FMT_UNDEFINED_SYMBOL \
  "Symbol \"%.*s\" is not (yet) defined."
#define FMT_UNEXPECTED_TOKEN "Unexpected token \"%.*s\"."

#define MSG_TOO_MANY_CHAINED_COMPARISONS \
  "Cannot chain more than 256 comparison operators."
#define MSG_MISSING_SEMICOLON "Missing \";\"."
#define MSG_UNEXPECTED_EOF "Unexpected end of file."

#endif
