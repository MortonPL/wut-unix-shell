# Style guide

Opinionated style guide by yours truly.

## Starting remarks
* We compile with `-Wall -Werror -Wextra -Wpedantic -Wconversion` for a reason.

## Naming conventions
* Preprocessor macros: SCREAMING_SNAKE_CASE.
* Local variables and local (or `static`) functions: camelCase.
* Global variables and global functions: PascalCase.
* Struct names and struct members: PascalCase.
* Pointers: pPascalCase.

## Preprocessor
* We use `#pragma once` instead of `#ifndef FILE_H #define FILE_H #endif` pattern.
* We use `<foo.h>` for system headers and `"foo.h"` for custom headers.
* We use preprocessor directives only for `#pragma once`, `#include` and `#define`.
* We include same-named header first (if applicable), then system headers, then custom headers, in this order.

In example:
```c
// file foo.c
#include "foo.h"
#include <cstdio.h>
#include "something.h"
```

## Variables
* We use `const` wherever possible (including `const type* const` for constant pointers).
* We always pass structs to functions as pointers.
