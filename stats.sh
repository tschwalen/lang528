#!/bin/sh
# For LoC of all C++ files (run from home dir)
printf "\nC++ LoC:\n" 
find . -type f \( -name "*.cpp" -o -name "*.h" \) -maxdepth 2 -exec wc -l {} + 

# The same, but for python
printf "\nPython LoC:\n" 
find . -type f -name "*.py" -exec wc -l {} +

printf "\nC LoC:\n" 
find ./runtime -type f \( -name "*.c" -o -name "*.h" \) -maxdepth 2 -exec wc -l {} + 
