#!/bin/bash

# Find all .cpp, .hpp, .h files excluding the build directory and run clang-format on them
find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -not -path "./build/*" -print0 | xargs -0 clang-format -style=file -i

echo "Formatting complete"
