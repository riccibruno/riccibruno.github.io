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
