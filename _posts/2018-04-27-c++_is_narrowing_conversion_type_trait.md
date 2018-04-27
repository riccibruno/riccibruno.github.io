---
layout:         post
title:          "An is_narrowing_conversion type trait for Clang, GCC and MSVC"
my_category:    "c++"
---
This post present a type trait to detect narrowing conversions as defined in the c++ standard.
The basic idea is explained and then workarounds for bugs in gcc/clang/msvc are presented.
<!--more-->

### Introduction and definition

Since c++11 the concept of a *narrowing conversion* is defined by the c++ standard.
The idea is to prevent loss of information when doing a conversion between arithmetic types.
It might therefore be useful to know when a conversion is a narrowing conversion.
The `<type_traits>` header provides a multitude of type traits but currently contains no type trait
detecting such conversion, although a proposal exists (P0870R0).

The goal of this post is to present an implementation of such a type trait.
First here is the definition of a narrowing conversion (in the c++17 draft n4659)
(in [dcl.init.list]/11.6.4 List-initialization):

> A *narrowing conversion* is an implicit conversion
>    * from a floating-point type to an integer type, or
>    * from `long double` to `double` or `float`, or from
>      `double` to `float`, except where the source is a constant
>      expression and the actual value after conversion is within
>      the range of values that can be represented
>      (even if it cannot be represented exactly), or
>    * from an integer type or unscoped enumeration type to a
>      floating-point type, except where the source is
>      a constant expression and the actual value after conversion
>      will fit into the target type and will produce the original
>      value when converted back to the original type, or
>    * from an integer type or unscoped enumeration type to an
>      integer type that cannot represent all the values of the original type,
>      except where the source is a constant expression whose value
>      after integral promotions will fit into the target type.

Since we are interested only in type-level information
(ie: is a conversion from type T1 to type T2 narrowing?)
we can discard all the text after "except ..."
What is left is the following:

An implicit conversion from type T1 to type T2 is narrowing if:
   * T1 is a floating-point type and T2 an integer type, or
   * T1 is an integer type or unscoped enumeration type
     and T2 a floating-point type, or
   * T1 is a floating-point type and T2 a narrower floating-point type, or
   * T1 is an integer type or unscoped enumeration type
     and T2 a narrower integer type

Although it would be quite possible to write the `is_narrowing_conversion` type trait
just by special-casing the various possibilities obtained from the above observation,
it would be quite annoying to write and redundant since the compiler must be able to
detect such conversions anyways. The idea is therefore to find an expression usable in an
SFINAE context to detect a narrowing conversion and enable/disable a class template specialization.

### First implementation

The basic idea is to have the following function template/function declarations:
~~~c++
template<typename To>
constexpr std::true_type is_narrowing_convertion_aux(To);
constexpr std::false_type is_narrowing_convertion_aux(...);
~~~
and then to examine the result of
~~~c++
decltype(is_narrowing_convertion_aux<To>( {std::declval<From>()} ) )::value
~~~
which will be true or false depending on which overload has been selected.
This idea is based in the fact that copy-list-initializations do not allow
narrowing conversions. We obtain the following code:
~~~c++
#include <type_traits>
#include <utility>

template<typename T>
struct identity { using type = T; };

template<typename To>
constexpr std::true_type is_narrowing_convertion_aux(typename identity<To>::type);
constexpr std::false_type is_narrowing_convertion_aux(...);

template<typename From, typename To, typename = void, typename = void>
struct is_narrowing_conversion : std::true_type {};
template<typename From, typename To>
struct is_narrowing_conversion<From, To,
    std::enable_if_t<   std::is_arithmetic_v<From> &&
                        std::is_arithmetic_v<To> >,
    std::enable_if_t<   decltype(
        is_narrowing_convertion_aux<To>( {std::declval<From>()} )
                        )::value >
> : std::false_type {};

template<typename From, typename To>
inline constexpr bool is_narrowing_conversion_v =
    is_narrowing_conversion<From, To>::value;
~~~

Some notes on the above code:
   * We use the `typename identity<T>::type` idiom to disable function template argument
     deduction. This is not strictly needed but we will need the identity template to work
     around a bug in MSVC later anyways. This force us to specify the `To` template argument
     inside `decltype(...)`.
   * We provide the c++17-style variable template `is_narrowing_conversion_v` and use the
     similar `std::is_arithmetic_v`. This could easily be replaced by `std::is_arithmetic<T>::value`.
   * We use the c++14-style `std::enable_if_t` which is just an alias template for
     `typename std::enable_if<B, T>::type`.

This first implementation works quite well. However a few [tests][test_is_narrowing_conversion.cpp] fails

[test_is_narrowing_conversion.cpp]: https://github.com/riccibruno/riccibruno.github.io/blob/master/assets/c%2B%2B_is_narrowing_conversion_type_trait/test_is_narrowing_conversion.cpp
