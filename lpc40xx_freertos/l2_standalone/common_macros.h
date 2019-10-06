#pragma once

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

/// @note: C++ code should use built-in 'static_assert()'
#ifndef COMPILE_TIME_ASSERT
#define COMPILE_TIME_ASSERT(cond) ((void)sizeof(char[1 - (2 * (int)!(cond))]))
#endif
