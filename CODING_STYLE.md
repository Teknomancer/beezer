# Beezer Coding Style

This file aims to document some of the coding conventions used in this project and to provide guidance for future contributions.

Beezer was started in ~2001-2002, so the coding style has gone through a few iterations.
There are still lots of places in the code that are inconsistent or outdated.
We'll fix these as and when we get time.

There are still several areas where I'm torn on what the better or "right" style is including some fundamental choices which I now regret.
For e.g., I regret choosing `char* whatever` over `char *whatever`, so this is in no way a style guide set in stone.


## Headers

### Inclusion order in .cpp files
1. Header file of the `.cpp` should always be the first header (e.g, `Foobar.cpp` must always include `Foobar.h` as its first file).
2. Other Beezer headers.
3. Third-party library headers.
4. OS headers (e.g, `Window.h`, `BStringView.h`).
5. C++ standard headers (e.g., `iostream.h`, `vector`).
6. C standard headers (e.g., `stdio.h`, `stdlib.h`).

Rationale: John Lakos Large Scale C++ Design.
Quote/further explanation here: https://stackoverflow.com/a/14243688

### Inclusion in .h files
Use forward declations as far as possible, only include a header in a header if absolutely necessary.
They have a cascading effect and have a direct impact on compilation times.

### Prefer C++ style headers
When including standard C headers in C++ code, prefer the C++-style header wherever possible.
Use `<cstdio>` over `<stdio.h>`.

Rationale: Since C++11 C-style headers are listed under "Compatibility features" and are deprecated.
Symbols in the C++-style headers are guaranteed to be available under the `std` namespace while including C-style headers gives us no such guarantee.
They pollute the global namespace but whether it's also available under `std` is implementation specific.


## Whitespace

- Whitespace between functions (or between pre-processor macros and functions) is 2 blank lines.
- Even if the function is encapsulated in `#ifdef/#endif`, use 2 blank lines before and after the pre-processor line.
- Indentation is 4 spaces (no tabs).
- Empty lines must be not be indented (configure your editor to strip trailing spaces for CPP and H files).
- One blank line at end of each source file.


## Comments

- Keep comments to an absolute minimum.
- Prefer single-line (aka C++-style/C99) comments wherever possible. In some situations open/close-style comments (`/* and */`) are preferred (like when ignoring unused function parameters).
- Avoid obvious comments.
   ```
   private:
       // Private members    <-- Avoid these
       bool    m_IsWhatever;
   ```
- Comments that distinguishes overridden public/protected members from those newly introduced in the class are fine.
- Don't explain features of the C/C++ language unless it's very non-obvious or obscure.
  ```
  // strFaux must be de-allocated by free()     <-- Avoid these
  char *strFaux = strdup(strSomeother);
  ```
  Assume the reader is familiar with C++ to a certain degree.


## Naming

- Class names and class member functions are in capitalized words case.\
  `class PrefsWindow` and `PrefsWindow::IsHidden()`
- Class members variables are prefixed with `m_` and follow camel case.\
  `bool m_isFaulting`
- Message constants are upper case with underscores separating words.\
  `const uint32 K_MSG_DONE = 'done';`
- Message constants that are static members of a class are prefixed with `k` and follow capitalized words.\
  `static const uint32 kInputMessage;`
- Enums are snake case with their members upper case with underscores separating words.
  ```
  enum bevel_type
  {
      INSET,
      OUTSET,
      BULGE,
      DEEP,
      NO_BEVEL
  };
  ```
- Align following lines of the same statement to one column after the start of the previous logical bracket.
  ```
  m_innerView = new BevelView(BRect(K_MARGIN, sepView1->Frame().bottom + K_MARGIN,
                                    Bounds().right - K_MARGIN, Bounds().bottom - K_MARGIN),
                              "FileJoinerWindow:InnerView", BevelView::bevel_type::INSET,
                              B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
  ```


## Constness

- Always make variables/members/data etc. `const` wherever possible regardless of whether you think the compiler might produce better optimized code.
  `const` makes the intention clear (both to the compiler and the programmer) and prevents accidental changes.
- Always use `const` on the right-side of what is being made constant.\
  Use `int32 const idRef = 32;` rather than `const int32 idRef = 32;`


## General

- Avoid using `#define`s for constants wherever as possible, prefer concrete types.
- Beezer should compile with gcc2 to be compatible with Haiku x86/gcc2 so this means any advanced C++14/whatever features are out.
- Avoid using C++ templates as far as possible.
- Avoid multiple-inheritence and `friend` as far as possible.
