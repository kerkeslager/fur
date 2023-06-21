# Contributing

## C Code Conventions

### Naming functions

Functions which operate on a type are prefixed with the name of the type on
which they operate, and receive a pointer to that type as their first
argument, named `self`. For example, the function which pushes to the
stack has signature `Stack_push(Stack* self, Value item);`.

Types typically have a function which initializes the type and a function
which releases any resources held by the type. If the function which
initializes the type allocates the memory and returns a pointer, it is
called `_new`, otherwise it takes a pointer to the type and is called `_init`.
If the function which releases resources frees the pointer to the type it
is called `_del`, otherwise it is called `_free`. This is so that you can
know what the function's effect on memory is, without having to look into
the function definition. Some examples function signatures:

```
void Stack_init(Stack* self);
void Stack_free(Stack* self); // does not free self
Symbol* Symbol_new(const char* text, size_t length, uint32_t hash);
void Symbol_del(Symbol*); // frees self
```

The general concept here is that `*_init()` and `_free()` are inverse
operations, and `*_new()` and `*_del()` are inverse operations. That is to
say, the general pattern is either:

```
Type variable;
Type_init(&variable /* maybe some args */);

/* use variable */

Type_Free(&variable);
```

...or:

```
Type* variable = Node_new(/* maybe some args*/);

/* use variable */

Type_del(variable);
```

A related convention is to call `*_init()` from `*_new()` and call `*_free()`
from `*_del()`, like this:

```
typedef struct {
  Subtype s;
  OtherSubType o;
} Type;

void Type_init(Type* self, Subtype s) {
  self->s = s;
  self->o = DEFAULT_O;
}

Type* Type_new(Subtype s) {
  Type* self = malloc(sizeof(Type));
  Type_init(self, s);
  return self;
}

void Type_free(Type* self) {
  Subtype_free(&(self->s));
}

void Type_del(Type* self) {
  Type_free(self);
  free(self);
}
```

#### Naming tests

Tests are named `test_` or they won't be picked up by the test runner,
are `void`, and take no arguments. `test_` is followed by the function they
test, followed by what aspect they test. For example, a test that
`Tokenizer_scan()` scans integer literals might have the type signature
`void test_Tokenizer_scan_integerLiteral();`.

### Error handling
#### Error handling in the parser

The compiler function `Compiler_compile()` calls `Parser_parseStatement()`
to obtain statement ASTs from the parser. Afterward it checks `parser->panic`
to see if an error occurred. If so, it sets its own `self->hasErrors` flag,
then calls `Parser_clearPanic()` to attempt to leave the parser in a state
where it can parse further statements.

When `Compiler_compile()` detects an error, it rewinds all emitted bytecode
and symbols to a snapshot taken at the beginning of `Compiler_compile()`.
It continues calling `Parser_parseStatement()` and emitting errors if they
occur, but no further bytecode or symbols are emitted. The hope is that we
will be able to detect and display all the errors in the source code in one
run, so that the user doesn't have to run the compiler multiple times to find
all the errors.

To facilitate this, when an error occurs in the parser, we do the following:

1. Set `self->panic = true` on the parser, to indicate to the caller
   (which is usually the compiler) that an error occurred.
2. Call `printError()` to print an error (which should be defined in
   `text.h`).
3. Free any heap-allocated local variables.
4. In some cases it may be sensible to consume a token so that when the
   compiler calls `Parser_clearPanic()` it leaves the parser in a better
   state.
5. Return `NULL`.

If a parser function calls another parser function which may find errors,
it is the responsibility of the former function to:

1. Check `self->panic` to see if an error occurred.
2. Free any heap-allocated local variables.
3. Return `NULL`.

Note that in most cases we won't want to print a second error.

### Error handling in the compiler
As described in the previous section, errors in `Parser_parseStatement()`
are bubbled up to the compiler, but the majority of possible errors in the
compiler occur after a successful parse, during validations the AST which
the compiler performs before generating code. When such an error occurs, we:

1. Set `self->hasErrors = true`.
2. Call `printError()` to print an error (which should be defined in
   `text.h`).
3. Free any heap-allocated local variables.
4. Return (most code generation functions are of type `void`.

If a compiler function calls another compiler function which may find errors,
it is generally unimportant what we do, because the top-level call to
`Compiler_compile()` will rewind any generated code and symbols. However,
care should be taken to avoid heap allocations which might leak. Code
emitting functions called by `Compiler_compile()` should not progress the
parser, so this also should not be a concern.
