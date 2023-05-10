# fur

## Ideas

### Function declaration syntax
Declaring a function is just a type of destructured assignment:

```
multiply(a,b) = a * b;

multiply(a,b) = {
  result = a * b;
  result;
}
```

We could probably get rid of the `=` like so:

```
multiply(a, b) {
  result = a * b;
  result;
}
```

...but I'm leaning away from this, because this makes the fact that this is an
assignment less explicit. This differentiates the assignment from other places
where blocks are used, which *aren't* assignments, and generally follow the
pattern:

```
keyword(...) ...;
keyword(...) {
  ...
  ...
}
```

### Lambda syntax
C-ish languages have often added some sort of arrow operator like this:

```
(a, b) => a * b;
(a, b) => {
  result = a * b;
  a * b;
}
```

This is slightly less desirable in fur because it breaks the pattern mentioned
previously, that most assignments follow a keyword/parentheses/block pattern.
This idea would lead to the following syntax:

```
fn(a, b) a * b;
fn(a, b) {
  result = a * b;
  a * b;
}
```

This syntax is also better when we start looking for a type syntax, as we can
give things types like `(int, int) -> int` without as much confusion around
arrow operators.

However, a point in favor of arrow syntax is...

### Pure for loops
Something like this?

```
for(i; 0; i < items.length; i => i + 1) {
  print(items[i]);
}
```

Note that `i` *is not* mutated, but rather bound to the variable before the
block is executed. This looks like a point in favor of the arrow syntax for
lambdas, since the following looks a bit weird:

```
for(i; 0; i < items.length; fn(i) i + 1) {
  print(items[i]);
}
```

One oddity here is that because the iteration is controlled by a lambda,
there's nothing stopping teh lambda variable from being a different name:

```
for(i; 0; i < items.length; fn(n) n + 1) {
  print(items[i]);
}
```
This needs more thought, I think.

### Loop returns
I'm of a divided mind on whether loops should return an array, or a single
value corresponding to the last iteration. It's worth noting that we can
easily avoid creating an array if the loop is not saved to a variable, but we
can't do a mixed approach, because there's a vast disparity between returning
an array and a single item, and we'd be building up an array only to return
a single item with something like `break`.

For a for loop it seems obvious to return an array at first glance:

```
> for(i in range(10)) {
  i + 1;
}
[ 1, 2, 3, 4, 5 ]
```

But this is a bit redundant, as we already have map:

```
> range(10).map(fn(i) i + 1);
[ 1, 2, 3, 4, 5 ]
```

Where the loop differentiates itself from map is with `break` and `continue`:

```
> for(i in range(10)) {
  if(i % 2 == 0) {
    continue;
  }
  i;
}
[ 1, 3, 5, 7, 9 ]
```
or

```
> for(i in range(10)) {
  if(i % 2 == 0) {
    continue(42);
  }
  i;
}
[ 42, 1, 42, 3, 42, 5, 42, 7, 42, 9 ]
```
or

```
> for(i in range(10)) {
  if(i == 4) break;
  i;
}
[ 0, 1, 2, 3 ]
```

and the perhaps weird

```
> for(i in range(10)) {
  if(i == 4) break(42);
  i;
}
[ 0, 1, 2, 3, 42 ]
```

This idea becomes strange when we get to the unconditional loop, however,
where the only sensible idea is to return the value of `break` defaulting to
`nil`:

```
> loop {
  break;
}
nil
> loop {
  break(42);
}
42
```

This also plays really nicely with the `for-else` idea from Python:

```
> for(i in range(10)) {
  if(i == 3) break(42);
} else {
  25;
}
42
> for(i in range(10)) {
  if(i == 20) break(42);
} else {
  25;
}
25
```

But then we're in an odd spot with the most common case:

```
> for(i in range(10)) {
  i;
}
9? nil?
```

### Provers
We can have a proof system based around provers:

* A theorem is a statement which may be true or false.
* An *axiom* is a theorem which we assume to be true.
* Axioms could be stated as something like `axiom() x != 0;` which would be added
  to the axioms for the current context.
* An assert would not be assumed to be true, but rather checked at runtime.
  However, under some circumstances we might be able to add it to the axioms
  for the remainder of the context, after which point the runtime check will
  have run and we can assume the checked theorem holds true.
* A prove statement (like `prove() x != 0;`) would fail at compile time if a)
  it can't be proven, or b) it is proven false.

This system becomes more useful if it comes with a bunch of prepackaged axioms,
for example `axiom(a,b) a + b = b + a;`

This should be easily able to check all properties of a type system, but it's
more powerful.

I would like the type system to be unobtrusive, i.e. the user should be able
to write code that looks like a dynamically typed language, and have it compile
and run. As such, we might want to include prove statements which only fail
compilation if we can prove they fail, for example if we can prove that the
denominator is 0 in division, like `prove() not is_provable(_ / 0);`.

Under certain circumstances, might we be able to treat functions as axioms?
