#pragma once
#include <version>

/*!
 * @file Requirements.h
 *
 * @brief ISO C++ requirements for dots-cpp.
 *
 * This header safeguards all C++20 features or later that are required
 * to build and use this library.
 *
 * If you are an author and want to use a C++ language feature in the
 * library that is not already listed here, ensure first that it is
 * provided (see [1]) by the desired set of officially supported
 * compilers specified in the README. Then add a preprocessor directive
 * based on the corresponding feature test macro (see [2]) according to
 * the example below.
 *
 * @code{.cpp}
 * #if not defined(__cpp_concepts)
 * #error "compiler is missing required feature: __cpp_concepts"
 * #endif
 * @endcode
 *
 * References:
 *
 *  - [1] https://en.cppreference.com/w/cpp/compiler_support
 *
 *  - [2] https://en.cppreference.com/w/cpp/feature_test
 */

#if not defined(__cpp_designated_initializers)
#error "compiler is missing required feature: __cpp_designated_initializers"
#endif

#if not defined(__cpp_lib_span)
#error "compiler is missing required feature: __cpp_lib_span"
#endif
