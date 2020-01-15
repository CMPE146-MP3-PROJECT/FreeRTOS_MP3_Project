#pragma once

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

/// @note: C++ code should use built-in 'static_assert()'
#ifndef COMPILE_TIME_ASSERT
#define COMPILE_TIME_ASSERT(cond) ((void)sizeof(char[1 - (2 * (int)!(cond))]))
#endif

/// If you get a warning about an unused variable or function (i.e.: foo)
/// you can explicitly disable the warning by using 'UNUSED(foo)'
#ifndef UNUSED
#define UNUSED (void)
#endif

#define MAX_OF(a, b) (((a) > (b)) ? (a) : (b))
#define MIN_OF(a, b) (((a) < (b)) ? (a) : (b))
