# Contributing

## C Code Conventions

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

Tests are named `test_` or they won't be picked up by the test runner,
are `void`, and take no arguments. `test_` is followed by the function they
test, followed by what aspect they test. For example:
`void test_Tokenizer_scan_scansIntegerLiteral();`.
