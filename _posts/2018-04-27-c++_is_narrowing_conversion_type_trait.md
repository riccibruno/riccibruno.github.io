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
narrowing conversions. We obtain the following [code][impl_v1]:
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

### Second implementation : making clang happy

This first implementation works quite well. However a few [tests][test_is_narrowing_conversion.cpp] fails.
First, we are not considering the case when `From` is an enumeration type. However changing
`std::is_arithmetic_v<From>` to `(std::is_arithmetic_v<From> || std::is_enum_v<From>)` still leaves clang (5.0) unhappy.
Therefore we add a special case for when `From` is an enumeration type.
This gives us the [second version][impl_v2]:

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
    std::enable_if_t<   (std::is_integral_v<From> || std::is_floating_point_v<From>) &&
                        (std::is_integral_v<To>   || std::is_floating_point_v<To>) >,
    std::enable_if_t<   decltype(
                            is_narrowing_convertion_aux<To>( {std::declval<From>()} )
                        )::value >
> : std::false_type {};
/* clang (tested on clang 5.0 and below) misdetect the
 * narrowing conversion when From is an unscoped enumeration type
 * in some cases. We decay it to the underlying type to
 * avoid this problem. */
template<typename From, typename To>
struct is_narrowing_conversion<From, To,
    std::enable_if_t<   (std::is_enum_v<From>) &&
                        (std::is_integral_v<To>   || std::is_floating_point_v<To>) >,
    std::enable_if_t<   decltype(
                            is_narrowing_convertion_aux<To>( {std::declval<std::underlying_type_t<From> >()} )
                        )::value >
> : std::false_type {};

template<typename From, typename To>
inline constexpr bool is_narrowing_conversion_v =
    is_narrowing_conversion<From, To>::value;
~~~

### Third implementation : making GCC happy

Clang 5.0 compiles correctly all the tests with the
second implementation but GCC for some reason fails to
detect some narrowing conversions to bool:

~~~
test_is_narrowing_conversion.cpp: In function ‘int main()’:
test_is_narrowing_conversion.cpp:17:5: error: static assertion failed
     static_assert(is_narrowing_conversion_v<float, bool>);
     ^~~~~~~~~~~~~
test_is_narrowing_conversion.cpp:18:5: error: static assertion failed
     static_assert(is_narrowing_conversion_v<double, bool>);
     ^~~~~~~~~~~~~
~~~

To fix this problem we add two special cases:
   * Anything but `bool` to `bool` => narrowing
   * `bool` to `bool` => non-narrowing
and obtain the following [third implementation][impl_v3]:

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
    std::enable_if_t<   (std::is_integral_v<From> || std::is_floating_point_v<From>) &&
                        (std::is_integral_v<To>   || std::is_floating_point_v<To>) &&
                        (!std::is_same_v<std::remove_cv_t<std::remove_reference_t<To>>, bool>) >,
    std::enable_if_t<   decltype(
                            is_narrowing_convertion_aux<To>( {std::declval<From>()} )
                        )::value >
> : std::false_type {};
/* clang (tested on clang 5.0 and below) misdetect the
 * narrowing conversion when From is an unscoped enumeration type
 * in some cases. We decay it to the underlying type to
 * avoid this problem. */
template<typename From, typename To>
struct is_narrowing_conversion<From, To,
    std::enable_if_t<   (std::is_enum_v<From>) &&
                        (std::is_integral_v<To>   || std::is_floating_point_v<To>) &&
                        (!std::is_same_v<std::remove_cv_t<std::remove_reference_t<To>>, bool>) >,
    std::enable_if_t<   decltype(
                            is_narrowing_convertion_aux<To>( {std::declval<std::underlying_type_t<From> >()} )
                        )::value >
> : std::false_type {};
/* gcc (as of gcc 7.3 and below) do not detect the narrowing
 * conversion when To is bool in some cases. In this case we return true_type
 * when From is not bool and false_type when From is bool. Note that gcc
 * do not like when the above two partial template specializations are enabled,
 * even though the two specializations below are more specialized (clang is fine).
 * They are therefore disabled with a check for To == bool up to cv-qual and ref. */
template<typename From>
struct is_narrowing_conversion<From, bool,
    std::enable_if_t<   (std::is_integral_v<From> || std::is_floating_point_v<From> ||
                        std::is_enum_v<From>) >,
    void > : std::true_type {};
template<>
struct is_narrowing_conversion<bool, bool, void, void> : std::false_type {};

template<typename From, typename To>
inline constexpr bool is_narrowing_conversion_v =
    is_narrowing_conversion<From, To>::value;
~~~

### Fourth implementation : Making MSVC happy

With the third implementation shown above both GCC and Clang compile correctly all the tests.
Can MSVC compile it ? Unfortunately for lacks of a Windows machine I can not test it but
we can use the [Godbolt compiler explorer][godbolt] to test it.
Specifically we will use `x86-64 MSVC 19 2017 RTW` with `/std:c++latest /O2 /permissive-`.
The use of `decltype(...)::value` causes an error but using the identity idiom we can replace
this with `identity<decltype(...)>::type::value`. The last error is from the `inline` variable
which MSVC do not support yet. Remove it allows MSVC to compile all the tests successfully.
Here is the [fourth and final implementation][impl_v4] which [you can see on Godbolt][godbolt_impl_v4].

[impl_v1]: https://github.com/riccibruno/riccibruno.github.io/blob/master/assets/c%2B%2B_is_narrowing_conversion_type_trait/is_narrowing_conversion_v1.hpp
[test_is_narrowing_conversion.cpp]: https://github.com/riccibruno/riccibruno.github.io/blob/master/assets/c%2B%2B_is_narrowing_conversion_type_trait/test_is_narrowing_conversion.cpp
[impl_v2]: https://github.com/riccibruno/riccibruno.github.io/blob/master/assets/c%2B%2B_is_narrowing_conversion_type_trait/is_narrowing_conversion_v2.hpp
[impl_v3]: https://github.com/riccibruno/riccibruno.github.io/blob/master/assets/c%2B%2B_is_narrowing_conversion_type_trait/is_narrowing_conversion_v3.hpp
[impl_v4]: https://github.com/riccibruno/riccibruno.github.io/blob/master/assets/c%2B%2B_is_narrowing_conversion_type_trait/is_narrowing_conversion_v4.hpp
[godbolt]: https://godbolt.org/
[godbolt_impl_v4]: https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAKxAEZSAbAQwDtRkBSAJgCFufSAZ1QBXYskwgA5NwDMeAGYssCgNQBhAAqaA%2BgEkAKtgBKAQQMB5YwGUdAEWwHTegDL7bAOVPHjFgOp6HgDiOuoWHgBqJtZ64RwADKZyKngsmBra%2BkZmljb2js5uep7evgHBoeFRNrEeCUmJcqnIDCJYqhyy6gQAngAOmDoExEyEgp3Y9U0sLW3pneoiBHgMhD0TU4ksTAC2mIJ9TBKqyH19OoSYI0TEgiAgWASjDB0A7HyNiQSYO33M3wtegNtntVAYNolBMMRMgCKo8FgWMtem8eKoRIJUsBVED5rI7GDOmiOK87ETNqZvr9/phAf1MCD0gZUBDTGgWFDMAAPPrEVRQ9D3aGDXHwwQ6bbEYioADuWJ07IAbldlqgWDomCIuRBcYz4YjkesusyJkL6QBKcmJdmcnl8gX3BRMBiCEX0sUSphS2XypUqvBqjVaiAAOjDltkHwalJ%2BfyYAK6ut26QAYtKdqQcfS9czM0nQZ0CYrUAi89nkx18apiwjWVDiDC4XhxZLpXK2Aq1crbgGWKoQPyCIKQMKhu6SXxSVaY9T47TE%2BXQWnUBms8CKybZJNIdDYR7Wz6O36e2qFsvV7n6qpr4PhwymAAjBiDRRDBY3iAOkDNi5IzDAEYGB0RUz3TCY3nUEl1Fve4fwUBhUHjeU%2BhLJFgNAlcJnNDouAANm4fDEhvYiSNI0jPyHWDxVSb4AOddDjRZLcbygqCYO/cV4MQ5YOxQmiGPUTdsGwgjRKIsiJIkiAwDAL8f0EZMBK/YgflQZUFUVN8umU1T1JUhQrgZCQtMEpjJi3TMH1QVAGCwytsFIeoAHoACocQQdIEQZQ0FiwFpcVDcNTRHel7kVZ0RE8wRVDSTAsHQVQH0wZBNVdVQdkERVkFUdBUBi1ACCvNzVgAa3SAgPNUBQSB2HLkoYAKwxDc0woizAQ1UYBkGy1gEpaVhsS9dIFFScqEFEYAEA6lynKvG8v3vJ8XwUEyby8pE1l8%2BqArmyS9rIn8D3bYBOxYbtVXVTUuQWISIFRL8/IYcLbK6c8JggS1SVUS1xP2/aI2wM0Bla1o8W3JJmIHL8nRdN0BlREkyUjZy3P6thVB1fZvgStUTmYdGAFYQ3iVResSzAEJlbDcvyuFHmSuEKtpIi3KOrETi7K5MVxmUPL7c8xVJvsRA5NABgShkRD2a5ezXdqitUPQ%2Bwq5sTiYNKZXSPymB6eFGbypn0WUK4GB6dnRSIBWmBrBKVei3lUCWnZptmr5YxpOl1yXdMyy9pkzPqetG33L0219TmTxYDCL1QRzfvYxbnwuFbCq6D85PFSWdgEt6txEvCxNMP6/oo4cfxo/9AIEoSWNeSC6/YuCEKQ3jUIIauzPz/CC/j4uyOk2TKI4nQFL2JSh5UnY1MGZBNNT9QdKnvTMAMlSZhFG6A4sxLrJe4T7Ljov5qHxPltW691p8rpHp23u%2B8kw7Q8PE7jwuoNrsY977qHx7noWL8RZYGIKbeUuITK52wPZD6iNvq7XvqRAGQNJAgGepFeyUxIbsRhq6Mc8MJyI2nK5Tq3UMbq1UKgNQXVsqvBDLIIWCUkqU2pnlFgBU6rfD3IbNmbAFbHm5n2XmDIwR5VVlZGy8I%2BzCFBClV0ggOpK3cqrGR6RNaqBUgQMQysGxw2ZkXNygj%2BbpkFqwuEYiXhk2wTo1QBjVAC1EbvDqHgCpjXjMQzgLMcosLYaVFRfN3LpEfNPHEMo8qHGIMsZ0OJ3Zzn5AMZAeBnR4AAF5ITVNFIaqhT7oEPqoNymBlTK3GiISa/jgl5QOMlBJqwUkXWiow2UpMVLpRIOkCp8TElJLihjNG2JVYjTSOaEMCsDAeV1hkpm%2BkWk5WbI%2BZ8CU5QVVJicDyyASpVRIMIysSMCRmPRH0HEeVZ4AFoACOIhIlk30i7eoVI4wJm6IuVMYEtyB13E2FsT9jqnW7Pw6Olld45OPnebYS1k7nwxhnX8tEq4gVes8yBrEG6Qq4i3E6fE0KwvUBAiCrE77wOvJCrOOd4UiQsnAm29l%2BzsVHKKfBU5kZu1nPcusbyQ7ei%2BXw3sCwzH/JspmG2/KSzoHAlDIeljcHzHeAQhl0ZbkewXH7WxPs5Y5gDokVyqRVhpBmhzDk3w7Q73EY/dl4czpc17MBLZcDjVhyPBHX5cKVyZiEiDSK04phfSck5U45xLjXBIHcB4mAngrApHIBk%2BA1Beq0LoQwJhzBWFsA4Jwrh3A6C8D4fwgQQhhEiNEWoUwuDyBmK0doCxkAChoqyL10xZhlq4FwMMTlmilswN6s4Fxvj%2BtuE5R4zxm0fJNXas1kcQwIDONwLgFIs7ohogADiGDoGdorhwiHnUMBG9KowzrXUiWguFF3LvYrugg%2B6N10qRlGeoNF0qjBYB9BG8ciG7JmnAqESFkAakEK6cJA8fWdquPGAN9w%2B0rCop6IdL97UWsxTyiRqdhLTmIu%2B5Yn71Y/oIH%2BjtfqgO3BA8G544HuFQZHfwgScGzFYSQ8fD9X6MMQH/Thm4gbQMMCI5801PyYMLBoryveEYozIaeKhujKoGPYa7bhljBGwPD2I9881gZMUAPXQQPjVGZVCdo%2BhsTjHJPMfwyGtjcmOPDq40p/%2BQ8T1nrU4a/j1GCXCbwGh79umJOAYM0Goz7HIMKcjgJZACAvTqbzg5wc2nXO/r0x54DXnCMmd85yizXQUW2co6FzTNGRM6ai%2B57t0nvMJdtSR8z6pMW5REEtELiHMuOYi/R6L%2BXDPxZtc/PzZHMUnoXe3LO1WBNvqcy5hreWpPNdk61jl0HkuLBojZpdLApZ9bC0Q1L7N0VwiOeBCuwAri5NdkfOr2XIuYca6NuL43B3Ffa9xlLzdbNVoy4JrLznRO5d9fp2LrGfNXaS2V3yogqvwY009w7L2csnZG55r7RW2u/YEghdGFXAcPZqyD8LR3hvvZi3h87xmJuccU39273FMyBa9MDgb9W3NY6a7j77sOptE/UEj58pOgvEAp/HFDYPjviZp2d6H%2BOzOE/h2qbELPMBs/J49uBK27trbbqoTbzEjq7dW%2Bjdbe3KcY%2BpwB2ngvLsM9Izd5nAPWdVTu5zg76OeeY71wLmTePDeTeN9NhH4uzeS4t9xK3Wmddvft1Dx39OXeldF4jz3mYJe%2B5vEQ7bu3leQPV9iTXr6ueDdexD/nQfCtC5KyLzFvHvfxhj6DobuumOfeDzD0PBeeNIij570vNvy8B8rzjg3EGfuM4C%2BzzMqXm/c9b1nwPVfc/O4J/5zFZOyCeMq8%2BQfGfwd89Hx36vefrvTZU0ibr/fLcy/T1TtvH21/j670bsPymrOqcb/P%2BcqPte24ryfgrLWJ/C6n5Z1ds2D22YHwf63IfTPFfdvV/C7c/WvT/bSa/PdX/W/JaZvOPP8HbPkRPGKT5XbePPkNPQApfXnGSU7HPN/CAyfDrL/e4LrIYTMbfAgbrRfI/TDAgyHMfYg%2BTOHK/b/HfKg49H/LSB/Q/f3RgsAQglg8AtgnvDgigm/HgpEWQLgPg/rAQp/X9Jg7PUQp3Egj/Mg6Azg2g7gmg3CAAFgULCyAOX1UNXzAI0PENdyZxoN30blgJMNqxb2AIsNALG2sNM3zygIXhgL0NszklkPkIQ0UNwIYKwzUNP1YO8M3zsP8IcKCIICMOcLRzMN5xEOiLENiPYPIJAEoMCKHlU3oMEJAJf08JD1IJNxoJs2oKKKcNCNMLwLtw8LpxryqK338LkP0PqIIG6MaJcPSJaPKLaI31yJ0KkKRBSMKLLimOMIGLRyIRFkEDFi6Szg81ljQNVz5CwK1yUOHzKOxysMqK0JN2s1/3m0WxkICJKOUJH1aM7xsMvwWHOMPQW1XBn1uIOMyOOPaNOOm1eJ63eMzGWLwGADSD6nZy%2BOAJ%2BIqL%2BJ8O0JmwaMuNXExHBK6U%2BIAOIiWNFlQHFkyXeI2NxjQOT1UFT32z9zuMOP13X3fwRLON4KBKuP/34PCNKNhNGLpLiIEkBJRPgIXyxOe2%2BOYKyK8MSwkJeMZL5NUHdznwQMFLLxhJFN%2BLGIlK6AKOlJZLCMpOFKiJVK5PGKRICOlOjwVNcOXw5MeJyLVKNO62lNlNNNZLgXUU0VUHiHdSnESCkHNEYGkAJikFIBYGkHiADNQGkEgl4H4H5FEHEHmCLVoADIIGDO9J9JKhAAJlwmJkMIAE5DDXhZBDC8JDD4guB91fSpBDCAygypAQzSAwypAAy7h4hSAkyazvTSA4BYAkA0BfgVgrhyBKAey%2Bg%2BziAUAGBaBsydAjDSARoGouZKAHxkzSAHxUgvQehpAEzSAey9gkQLAWBTYlysAdgBpnxDy8AVJYQ8BlQ7g2zSBuRkolhJApBNyK42NnzEziA8AdhkyfT8Z2B%2BB%2BBGA8AHw7hIAfS8TalpAnIHROBIzeAaQoRVAnILAuAkKBhiAdhmxMRlQjlGyYyJA6Bfy/Sqylz6yuQ51cIjkjDVAABZawCIaCCc1QLgeIWgV4VQYwAwPwDGXAQgDZOQegDQFcYc58PkAS7CCMvgXgRMn81MkAQw2QEMbMgmeIAmPCAmfMgmbM14OdAmcsyswM0i6QRskAZs1skM80IiqQLgEi28%2BsmStsyy0gUreSoAA%3D%3D%3D
