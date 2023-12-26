# ec

`ec` is a tool that lets you couple files (source code in particular) and
actions on those files. These actions, or commands, are defined in a plaintext
header at the beginning of the file and resemble comments in many programming
languages.

### `ec` headers

An `ec` header is a collection of consecutive lines at the beginning of a file
beginning with `//` or `##`. Once `ec` encounters a line that does not begin
with either of these patterns, it stops parsing the header.

Actions/commands are defined in the header by the following format:
```
//commandname:shell to execute
```
Everything than comes before the colon (`:`) is part of the command name and
everything after is part of the shell command that will executed if the command
is invoked by the user. There must be no white space between the starting
pattern (`//` or `##`) and the command name. If there is white space, then the
line will be interpreted as a continuation from the last command definition.

In order to act on the current file, the environment variable `SRC` is set to
the current filename before executing the command's shell code. To use user
specified arguments, use the environment variables `A0`, `A1`, `A2`, up to a
maximum of `A99`.

For example:
```c
//compile:gcc -Wall -Wextra -Werror -pedantic -O3 $SRC -o $A0
//debug:gcc -Wall -Wextra -Werror -pedantic -g -O0 -fsanitize=address
// -fsanitize=undefined $SRC -o $A0

// This is a comment and not part of the ec header.

#include <stdio.h>

int main(void) {
  printf("Hello, world!\n");
  return 0;
}
```
is a source file for a small C program along with an `ec` header to allow for
quick compilation. The header specifies two actions/commands, `compile` and
`debug`. Both actions compile the current file and write an executable to a file
specified by the user.

### Running `ec`

To run an `ec` on a file, pass the filename, the action you want to run, and any
arguments you wish to pass.

If the example from above was in a file named `hello.c`, then the `debug`
action/command could be run with the following invocation of `ec`:
```
ec hello.c debug output_program
```
