# Operator precedence and associativity

| Order | Operator(s) | Description        | Associativity |
| :---: | :---------: | ------------------ | ------------- |
| 1     | `[]`        | Subscript          | Left          |
|       | `()`        | Call               |               |
|       | `.`         | Field access       |               |
| 2     | `-`         | Negation           | Right         |
|       | `~`         | Bitwise not        |               |
| 3     | *(see bitwise and mathematical tables)* | |      |
| 4     | `<`         | Less than          | Left          |
|       | `>`         | Greater than       |               |
|       | `<=`        | Less than equal    |               |
|       | `>=`        | Greater than equal |               |
|       | `==`        | Equal              |               |
|       | `!=`        | Not equal          |               |
|       | `in`        | Membership         | Left          |
|       | `is`        | Identity           |               |
| 5     | `not`       | Logical not        | Right         |
| 6     | `and`       | Logical and        | Left          |
| 7     | `or`        | Logical or         | Left          |
|       | `xor`       | Logical xor        |               |
| 8     | `=`         | Assignment         | Left          |
|       | `<-`        | Storage (mutation) |               |

Fur's operator precedence refuses to order two categories of operators:
mathematical and bitwise. This is because we don't have any consistent logical
justification for ordering these. We want Fur code to be intuitive, and we
don't have a strong intuition for what the precedence of these operators would
be. Parentheses clarify the order, and refusing to order these operators forces
developers to use clarifying parentheses to make their intent explicit.

Unary operators for negation (`-`) and bitwise not (`~`) don't have this
problem because their ordering is unambiguous.

As a general rule we err on the side of left associativity with all the same
precedence unless we have a good reason to do otherwise.

## Mathematical
| Order | Operator(s) | Description       | Associativity |
| :---: | :---------: | ----------------- | ------------- |
| 3a    | `*`         | Multiplication    | Left          |
|       | `//`        | Integer division  |               |
|       | `mod`       | Modular division  |               |
|       | `/`         | Rational division |               |
|       | `./`        | Float division    |               |
|       | `.//`       | Fixed division    |               |
| 3b    | `+`         | Addition          | Left          |
|       | `-`         | Subtraction       |               |

## Bitwise
| Order | Operator(s) | Description       | Associativity |
| :---: | :---------: | ----------------- | ------------- |
| 3     | `<<`        | Left shift        | Left          |
|       | `>>`        | Right shift       |               |
|       | `&`         | Bitwise and       |               |
|       | `\|`        | Bitwise or        |               |
|       | `^`         | Bitwise xor       |               |
