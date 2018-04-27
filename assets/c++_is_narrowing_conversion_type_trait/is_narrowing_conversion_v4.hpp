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
/* the identity<decltype(...)>::type::value is needed because msvc do not
 * like the form decltype(...)::value. gcc and clang are fine though. */
    std::enable_if_t<   identity<decltype(
                            is_narrowing_convertion_aux<To>( {std::declval<From>()} )
                        )>::type::value >
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
    std::enable_if_t<   identity<decltype(
                            is_narrowing_convertion_aux<To>( {std::declval<std::underlying_type_t<From> >()} )
                        )>::type::value >
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

/* remove inline for MSVC */
template<typename From, typename To>
inline constexpr bool is_narrowing_conversion_v =
    is_narrowing_conversion<From, To>::value;
