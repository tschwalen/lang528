## Installation

#### Dependencies:

- cmake, make, and a C++17 compiler
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
      -  At this point, you should have a `Makefile`, a `compile_commands.json` and several other files that cmake generates (and which git ignores).
  3. Run `make` in the same directory as the `Makefile`
      - Address any errors, e.g. if you haven't resolved the `<nlohmann/json.hpp>` dependency yet, it will fail to compile.
  4. If `make` ran with no errors, and you have an executable file named `output` in the project root directory, then the your build was successful.
  
