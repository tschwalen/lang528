## Lang528

This is an interpreter and compiler for a programming language of my own making which I'm using to explore different approaches to language design and implementation. Specifically, I wanted to explore different approaches to compiling dynamic languages.

The name Lang528 was chosen because I started work on May 28th. One day I'll come up with a better name.

The language is a dynamically-typed scripting language, similar to Python, Javascript and Lua.
It supports basic data types, strings, vectors/arrays, dictionaries, as well as user-defined functions and modules.

Here is an example program:

```
function custom ()
  return 22;
..

function main ()
  let x = 44;
  let y = 800;

  let dict = {};
  let vec = [];

  dict["hello"] = x;
  vec.append(y);

  print("This should be 866: " + (dict['hello'] + vec[0] + custom()));
..
```

See `examples/` and `examples/e2e` for more sample programs.

## Technical Details

The implementation is written in C++20. The interpreter is a simple AST-walk type, while the compiler
generates a C program to be compiled and linked against a runtime library written in C that provides much of the
language's dynamic functionality (see `runtime/`).

For example, if your code contains:
```
function something (a, b)
  c = a + b;
..
```
Then the compiler generates something like:
```
Object* something(Object* a, Object* b) {
   Object* c = op_add(a, b);
}
```
Where `op_add()` is a function provided by the runtime which unboxes, type-checks, and lowers the values so that the `something()` function works regardless of whether strings, integers, or floats are passed in. 

## Installation

#### Dependencies:

- cmake, make, and a C++20 compiler
- [Niels Lohmann's JSON library for C++](https://github.com/nlohmann/json)
  - download version 3.11.2 as a header only library and place it in the include directory like so:
  ```
  include
    | ...
    \- nlohmann
      \- json.hpp
  ```

#### Setting up Dev environment

1. Clone the repository
2. Ensure you have cmake installed, then run `cmake .` in the same directory as the `CMakeLists.txt` file

   - At this point, you should have a `Makefile`, a `compile_commands.json` and several other files that cmake generates (and which git ignores).

3. Run `make` in the same directory as the `Makefile`

   - Address any errors, e.g. if you haven't resolved the `<nlohmann/json.hpp>` dependency yet, it will fail to compile.

4. If `make` ran with no errors, and you have an executable file named `output` in the project root directory, then your build was successful.
5. At this point your development environment should be configured correctly and you can follow standard cmake C++ procedures. E.g. changing any source file requires a rebuild with `make` and adding, removing, or renaming source files requires regeneration of the makefile with `cmake .`

   - Additionally, because I rolled my own basic codegen, if you change any target or template files you will need to rerun cmake before your changes are reflected in the build.

6. Once cmake generates `compile_commands.json`, you can enjoy full linting and static analysis for this project in any LSP-compatible editor or IDE, assuming that you install and configure clangd correctly.

## Running

#### Compiling a program

To compile a program and produce a static binary executable use the `--comp-e2e` option, passing in both an `--input` and `--output` file. Currently the compiler assumes that any modules you import will be in the same directory as the input file, and expects to have a C compiler named `cc` available in the process's path.

```
$ ./output --comp-e2e --input=examples/hello_world.src --output=./hello_world_executable
```

#### Running a script

Directly execute a file by passing the `--exec` option and passing the path to using the `--input` option.

```

$ ./output --exec --input=examples/hello_world.src
Hello, World!

```

#### Passing command-line arguments to the script

Pass arguments using the `--argv` option. Use quotes to pass multiple arguments.

```

$ ./output --exec --input=examples/hello_world.src --argv=Vanya
Hello, Vanya!
$ ./output --exec --input=examples/hello_world.src --argv='Vanya Mitya Alyosha'
Hello, Vanya!
Hello, Mitya!
Hello, Alyosha!

```

#### Running unit tests

Passing the `--test` option runs all unit tests. Right now every build includes the option to run all tests, but at some point they probably be excluded using compile flags.

```

$ ./output --test
test option passed
Running all unit tests:
...
  - test_that_token_serializes_to_json_as_expected [✓]
  - test_that_token_serializes_from_json_as_expected [✓]
```

#### Getting JSON-serialized output from the parser or lexer

Combining `--lex` or `--parse` with the `--dump-json` option will output a JSON-serialized representation of either the lexer or parser stage of compilation.
This was originally added for debugging purposes during development, but they could be used to write integration or regression tests, or generally as a way to export token stream or AST data in a serialized form for another program to process.

Dumping lexer output (truncated):

```

$ ./output --lex --dump-json --input=examples/test1.src
[
{
  "metadata": {
    "column": 0,
    "line": 1
  },
  "type_int": 0,
  "type_string": "FUNCTION",
  "value": null
},
...

```

Dumping parser output (truncated):

```

$ ./output --parse --dump-json --input=examples/test1.src | head
{
  "data": null,
  "type_int": 0,
  "type_string": "TOP_LEVEL",
  "xmetadata": {
    "column": 0,
    "line": 1
  },
  "zchildren": [
{
...

```

## References/Source

I did not directly work based off of any prior codebase for this, save for one or two of my earlier interpreter/compiler projects. However the general background knowledge for the stuff implemented here (e.g. recursive-descent parsing, Pratt/Operator-precedence parsing, abstract syntax tree representation, single-pass code generation) came largely from the following, as well as reading wikipedia.

[CS345 Programming Languages, Taught by Dr. William Cook at UT Austin in Spring 2020](https://www.cs.utexas.edu/~wcook/)

[Crafting Interpreters, Robert Nystrom](https://craftinginterpreters.com/)

[Programming Language Tutorial, Mihai Bazon](https://lisperator.net/pltut/)

[Let's Build a Compiler, Jack Crenshaw](https://compilers.iecc.com/crenshaw/)





