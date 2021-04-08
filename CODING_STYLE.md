Beezer Coding Style
===================

This file aims to document some of the coding conventions used in this project and to provide guidance for future contributions.

Beezer was started in ~2001-2002, so the coding style has gone through a few iterations.
There are still lots of places in the code that are inconsistent or outdated.
We'll fix these as and when we get time.

There are still several areas where I'm torn on what the better or "right" style is including some fundamental choices which I now regret.
For e.g., I regret choosing `char* whatever` over `char *whatever`, so this is in no way a style guide set in stone.

Header inclusion order in .cpp files
------------------------------------
1. Header file of the `.cpp` should always be the first header (e.g, `Foobar.cpp` must always include `Foobar.h` as its first file).
2. Other Beezer headers.
3. Third-party library headers.
4. OS headers (e.g, `Window.h`, `BStringView.h`).
5. C++ standard headers (e.g., `iostream.h`, `vector`).
6. C standard headers (e.g., `stdio.h`, `stdlib.h`).

Rationale: John Lakos Large Scale C++ Design.
Quote/further explanation here: https://stackoverflow.com/a/14243688

Header inclusion in .h files
----------------------------
Use forward declations as far as possible, only include a header in a header if absolutely necessary.
They have a cascading effect and have a direct impact on compilation times.

Whitespace
----------
- Whitespace between functions (or between pre-processor macros and functions) is 2 blank lines.
- Even if the function is encapsulated in `#ifdef/#endif`, use 2 blank lines before and after the pre-processor line.
- Indentation is 4 spaces (no tabs).
- Empty lines must be not be indented (configure your editor to strip trailing spaces).
- One blank line at end of each source file.

Comments
--------
- Keep comments to an absolute minimum.
- Avoid obvious comments.
   ```
       private:
           // Private members    <-- Avoid these
           bool    m_IsWhatever;
   ```
- Comments that distinguishes overridden public/protected members from newly introduced in the class are fine.
- Don't explain features of the C/C++ language unless it's very non-obvious.
   ```
       // strFaux must be de-allocated by free()     <-- Avoid these
       char *strFaux = strdup(strSomeother);
   ```
   Assume the reader is familiar with C++ to a certain degree.

Naming
------
- Class names and class member functions are in capitalized words case. E.g., `class PrefsWindow` and `PrefsWindow::IsHidden()`.
- Class members variables are prefixed with `m_` and follow camel case. E.g.,`bool m_isFaulting`.
- Message constants are upper case with underscores separating words. E.g., `const uint32 K_MSG_DONE = 'done';`.
- Message constants that are static members of a class are prefixed with `k` and follow capitalized words. E.g., `static const uint32 kInputMessage;`.
- Align following lines of the same statement to one column after the start of the previous logical bracket. E.g.,
```
m_innerView = new BevelView(BRect(K_MARGIN, sepView1->Frame().bottom + K_MARGIN,
                                  Bounds().right - K_MARGIN,
                                  Bounds().bottom - K_MARGIN),
                            "FileJoinerWindow:InnerView", btNoBevel, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
```

General
-------
- Avoid using `#define`s for constants wherever as possible, prefer concrete types.
- Beezer should compile with gcc2 to be compatible with Haiku x86/gcc2 so this means any advanced C++14/whatever features are out.
- Avoid using C++ templates as far as possible.
- Avoid multiple-inheritence and `friend` as far as possible.
