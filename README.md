# fur

## Ideas

### A design for avoiding GC bugs
A lot of GC bugs consist of the following pattern:

1. You perform some operation which either allocates an object, or removes a
   reference to an object.
2. Before you reference or re-reference the object, you perform an allocation
   which might trigger a GC.
3. Because the object in step 1 is not referenced from any roots, it is
   collected out from under you mid-operation.

Solutions to this vary, from placing the object in the roots temporarily, to
reordering code to avoid the gap where the object isn't referenced, etc. But
since these solutions have to be applied on a case-by-case basis, they are
error-prone.

My idea for Fur is to place a limitation on Fur such that Fur ops (instructions)
are atomic with regards to GC: GC cannot occur in the middle of an instruction.
Instead, when the allocator decides to GC during an instruction, it schedules a
GC for after the current instruction has completed. It does this by changing the
program counter to point to a two-instruction bytecode chunk stored on the
thread.  These two instructions are an OP_GC instruction (which runs the GC) and
an OP_JMP instructon which jumps back to the location of the program counter
from when the GC was scheduled. Essential in this is the idea that GC is an
instruction, between two other instructions, which it cannot interrupt. It is
still the responsibility of all other instructions to leave memory in a state
such that GC can run, so this isn't a panacea. However, his avoids the most
common form of GC bugs described above.

There is some runtime performance cost to this approach, but it's notble that
the *other* solutions to the above problem are *also* not without runtime
performance costs.

### n-ary Comparison operators

```
> 1 < 2 < 3
  true
> n() = {
    print("Hello\n");
    1;
  }
  nil
> 1 < 1 + n() < 3 # n is only called once
Hello
  true
> 1 < 1 < 10 // 0 # no error, because 1 < 1 returns false and short circuits before division by zero
  false
```

This is as if...

```
(A) < (B) < (C)
```

...expanded to...

```
(A) < (B) and (B) < (C)
```

...except that `(A)`, `(B)` and `(C)` are guaranteed to evaluate only once and
in that order, so it's actually expanded to something  more like...

```
let a = (A);
let b = (B);

if(a < b) {
  b < c;
} else {
  false;
}
```

...except it doesn't create variables `a` and `b`. A final note is that this works
for any string of left-associative comparison operators, so things like...

```
a != b == c < d > e <= f >= g
```

...are legal, although of questionable utility.

*Note: If we're going to implement this, we should do so before much code is
written in Fur, because there exist "false ternary" situations with comparison
operators, and we want this implemented before people use those. For example:*

```
false == true == false
```

*...returns `true` at the time of this writing, because `false == true` returns
`false`, which is then compared to the `false` on the right. But with n-ary
comparisons this would evaluate to false because the three are not all equal
to each other.*

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

### Threading benchmarks
I'd like to have benchmarks around, well, everything, but a high priority
is threading scalability. Memory usage is one limiting factor on threading.
Looking at benchmarks for other languages, I think a reasonable goal is that
the Fur equivalent of the following benchmark should stay below 50MB of
memory usage:

    #include <pthread.h>
    #include <unistd.h>

    void* threadFunction(void* arg) {
      sleep(10);
      pthread_exit(NULL);
    }

    int main() {
      int numThreads = 1000000;
      pthread_t threads[numThreads];

      for (int i = 0; i < numThreads; i++) {
        pthread_create(&threads[i], NULL, threadFunction, NULL);
      }

      for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
      }

      return 0;
    }

At 1 million threads, 50MB is approximately 52 bytes per thread. Keeping in
mind that the above benchmark runs in &lt;16MB, spiking up to &lt;23MB during
cleanup, this would give us about 30 bytes per thread to work with. With
just a stack and program counter pointer, we're already at 32 bytes,
and we don't even have message queues for threads at the time of this
writing, but there are some packing options we can explore.

### Multiline in REPL

The REPL used readline for history support and to integrate system-wide
user keymaps on \*nix systems. I explored multiline support in the REPL by
allowing the user to hold shift while pressing enter. This is a convention
introduced by websites.

This turns out to be not possible with readline because readline doesn't
actually track keypresses, it tracks *characters* received from `stdin`.
The shift key doesn't emit a character, and since the enter key is not
cased, we have no way of knowing if shift is depressed when the enter key
is pressed. There are platform-specific ways of tracking the shift key, but
I'm not ready to commit to a multiplatform maintenance nightmare any time
soon.

There are a few other options we could explore:

* A different key combo. Ctrl+Enter is the most obvious. Ctrl emits
  characters to readline for historical reasons, but is bound in the emacs
  keymap.
* An actual key sequence. Enter Enter is an obvious choice, but then we have
  to choose if Enter Enter submits the line, or inserts a newline. If it
  inserts the newline, we have to inject a delay before submitting to allow
  the user to press Enter a second time, which might make the REPL appear
  laggy (let's experiment with this?). If we have Enter Enter submit the
  line, that slows down the most common case where the user is simply
  entering a single expression.
* Another possible key sequence would be \ Enter, which is intuitive in a way
  because \ is used elsewhere as an escape character, so it's like you're
  escaping the enter. A possible problem with this is that it could mess
  with copy/pasting (more on this later). However this does generally seem
  like a reasonable option. We could also use other escape sequences to
  perform other commands in the REPL.
* There are only a few built-in keymaps by default in Readline. We could
  come up with a separate keybinding for each that fits in with the spirit
  of the keybinding. This also seems like a sensible option, but maybe a
  bit more work.
* The option used by Python and some other REPLs is syntactic analysis, i.e.
  if a user presses Enter when the expression is clearly not complete, insert
  a newline. The problem with this in Fur is that many expressions don't
  require clear ending delimiters because we're using a 1-lookahead, For
  example: `choose(a, b, c) = if(a) b; else c;` could easily break at
  `choose(a, b, c)` or `choose(a, b, c) = if(a) b;`. There's nothing stopping
  us from doing syntactic analysis *in addition* to one of the other
  approaches.

Another related note is that we probably want users to be able to paste
multiline input into the REPL. We might be able to handle this by detecting
keypresses more rapid than are humanly possible.
