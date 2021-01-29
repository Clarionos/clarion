#include <clcrypto/base58.hpp>
#include <array>
#include <cstring>
#include <vector>

namespace clcrypto {


constexpr char base58_chars[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

constexpr auto create_base58_map() {
    std::array<int8_t, 256> base58_map{{0}};
    for (unsigned i = 0; i < base58_map.size(); ++i)
        base58_map[i] = -1;
    for (unsigned i = 0; i < sizeof(base58_chars); ++i)
        base58_map[base58_chars[i]] = i;
    return base58_map;
}

constexpr auto base58_map = create_base58_map();

template <typename Container>
void base58_to_binary(Container& result, const std::string_view& s) {
    std::size_t offset = result.size();
    for (auto& src_digit : s) {
        int carry = base58_map[static_cast<uint8_t>(src_digit)];
        if (carry < 0)
            throw std::runtime_error( "error parsing base58" );

        for (std::size_t i = offset; i < result.size(); ++i) {
            auto& result_byte = result[i];
            int x = static_cast<uint8_t>(result_byte) * 58 + carry;
            result_byte = x;
            carry = x >> 8;
        }
        if (carry)
            result.push_back(static_cast<uint8_t>(carry));
    }
    for (auto& src_digit : s)
        if (src_digit == '1')
            result.push_back(0);
        else
            break;
    std::reverse(result.begin() + offset, result.end());
}

template <typename Container>
std::string binary_to_base58(const Container& bin) {
    std::string result("");
    for (auto byte : bin) {
        static_assert(sizeof(byte) == 1);
        int carry = static_cast<uint8_t>(byte);
        for (auto& result_digit : result) {
            int x = (base58_map[result_digit] << 8) + carry;
            result_digit = base58_chars[x % 58];
            carry = x / 58;
        }
        while (carry) {
            result.push_back(base58_chars[carry % 58]);
            carry = carry / 58;
        }
    }
    for (auto byte : bin)
        if (byte)
            break;
        else
            result.push_back('1');
    std::reverse(result.begin(), result.end());
    return result;
}


std::string to_base58( const std::string_view& data ) {
    return binary_to_base58( data );
}

void from_base58( const std::string_view& b58, char* data, uint32_t data_size ) {
    std::vector<char> buf; 
    buf.reserve(data_size);
    base58_to_binary( buf, b58 );
    memcpy( data, buf.data(), std::min( data_size, uint32_t(buf.size()) ) );
}

} /// namespace clcrypto
