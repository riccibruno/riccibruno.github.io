#include <cstdint>
#include "is_narrowing_conversion_v1.hpp"

enum uint8_t_enum : std::uint8_t {};
enum uint16_t_enum : std::uint16_t {};

int main() {
    /* bool */
    static_assert(!is_narrowing_conversion_v<bool, int>);
    static_assert(!is_narrowing_conversion_v<bool, short>);
    static_assert(!is_narrowing_conversion_v<bool, bool>);
    static_assert(is_narrowing_conversion_v<int, bool>);
    static_assert(is_narrowing_conversion_v<std::uint8_t, bool>);
    static_assert(is_narrowing_conversion_v<std::uint16_t, bool>);
    static_assert(is_narrowing_conversion_v<char, bool>);
    static_assert(is_narrowing_conversion_v<float, bool>);
    static_assert(is_narrowing_conversion_v<double, bool>);
    static_assert(is_narrowing_conversion_v<uint8_t_enum, bool>);
    static_assert(is_narrowing_conversion_v<uint16_t_enum, bool>);
    /* floating point -> integer */
    static_assert(is_narrowing_conversion_v<float, int>);
    static_assert(is_narrowing_conversion_v<double, int>);
    static_assert(is_narrowing_conversion_v<long double, int>);
    static_assert(is_narrowing_conversion_v<float, char>);
    static_assert(is_narrowing_conversion_v<double, char>);
    static_assert(is_narrowing_conversion_v<long double, char>);
    /* floating point -> narrower floating point */
    static_assert(is_narrowing_conversion_v<double, float>);
    static_assert(is_narrowing_conversion_v<long double, float>);
    static_assert(is_narrowing_conversion_v<long double, double>);
    static_assert(!is_narrowing_conversion_v<float, float>);
    static_assert(!is_narrowing_conversion_v<double, double>);
    static_assert(!is_narrowing_conversion_v<long double, long double>);
    /* integer -> floating point */
    static_assert(is_narrowing_conversion_v<int, float>);
    static_assert(is_narrowing_conversion_v<int, double>);
    static_assert(is_narrowing_conversion_v<char, float>);
    static_assert(is_narrowing_conversion_v<char, double>);
    static_assert(is_narrowing_conversion_v<std::uint8_t, float>);
    static_assert(is_narrowing_conversion_v<std::uint8_t, double>);
    static_assert(is_narrowing_conversion_v<std::uint16_t, float>);
    static_assert(is_narrowing_conversion_v<std::uint16_t, double>);
    /* integer -> narrower integer */
    static_assert(!is_narrowing_conversion_v<std::uint8_t, std::uint8_t>);
    static_assert(!is_narrowing_conversion_v<std::uint8_t, std::uint16_t>);
    static_assert(!is_narrowing_conversion_v<std::uint8_t, std::uint32_t>);
    static_assert(!is_narrowing_conversion_v<std::uint8_t, std::uint64_t>);
    static_assert(!is_narrowing_conversion_v<std::uint8_t, std::int16_t>);
    static_assert(!is_narrowing_conversion_v<std::uint8_t, std::int32_t>);
    static_assert(!is_narrowing_conversion_v<std::uint8_t, std::int64_t>);
    static_assert(is_narrowing_conversion_v<std::uint8_t, std::int8_t>);
    static_assert(is_narrowing_conversion_v<std::uint16_t, std::int16_t>);
    static_assert(is_narrowing_conversion_v<std::uint32_t, std::int32_t>);
    static_assert(is_narrowing_conversion_v<std::uint64_t, std::int64_t>);
    /* unscoped enumeration -> narrower integer */
    static_assert(is_narrowing_conversion_v<uint16_t_enum, std::uint8_t>);
    static_assert(is_narrowing_conversion_v<uint16_t_enum, char>);
    static_assert(is_narrowing_conversion_v<uint16_t_enum, unsigned char>);
    static_assert(is_narrowing_conversion_v<uint16_t_enum, signed char>);
    static_assert(!is_narrowing_conversion_v<uint16_t_enum, std::uint16_t>);
    static_assert(!is_narrowing_conversion_v<uint16_t_enum, std::uint32_t>);
    static_assert(!is_narrowing_conversion_v<uint16_t_enum, std::int32_t>);
    /* unscoped enumeration -> floating point */
    static_assert(is_narrowing_conversion_v<uint16_t_enum, float>);
    static_assert(is_narrowing_conversion_v<uint16_t_enum, double>);
    static_assert(is_narrowing_conversion_v<uint16_t_enum, long double>);
    static_assert(is_narrowing_conversion_v<uint8_t_enum, float>);
    static_assert(is_narrowing_conversion_v<uint8_t_enum, double>);
    static_assert(is_narrowing_conversion_v<uint8_t_enum, long double>);

    return 0;
}
