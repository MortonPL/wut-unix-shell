# Style guide

Opinionated style guide by yours truly.

## Starting remarks
* We compile with `-Wall -Werror -Wextra -Wpedantic -Wconversion` for a reason.
* We use `TODO`, `NOTE`, `HACK` and `DEBUG` in comments to mark code if necessary. If you're using VS Code, the workspace recommends an extension for this.

## Naming conventions
* Preprocessor macros: SCREAMING_SNAKE_CASE.
* Local variables, enums and local (or `static`) functions: camelCase.
* Global variables, enums and global functions: PascalCase.
* Local struct names and struct members: camelCase.
* Global struct names and struct members: PascalCase.
* Pointers: pPascalCase or `ptr` for trivial, temporary ones.

## Preprocessor
* We use `#pragma once` instead of `#ifndef FILE_H #define FILE_H #endif` pattern.
* We use `<foo.h>` for system headers and `"foo.h"` for custom headers.
* We use preprocessor directives only for `#pragma once`, `#include` and `#define`.
* When using `#define`, we use `#undef` as soon as possible.
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
* We don't like magic values. We like enums and `const` variables.
