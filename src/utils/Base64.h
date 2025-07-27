#pragma once

#include <string>
#include <vector>

namespace RPGEngine {
namespace Utils {

/**
 * Base64 decoder class
 * Decodes base64-encoded data
 */
class Base64 {
public:
    /**
     * Decode base64-encoded data
     * @param input Base64-encoded string
     * @return Decoded data
     */
    static std::vector<uint8_t> decode(const std::string& input) {
        std::vector<uint8_t> result;
        
        // Initialize lookup table
        static const int lookup[256] = {
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
            -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
            -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        };
        
        // Calculate output size
        size_t inputLength = input.length();
        size_t outputLength = inputLength * 3 / 4;
        
        // Adjust output size for padding
        if (inputLength > 0 && input[inputLength - 1] == '=') {
            outputLength--;
        }
        if (inputLength > 1 && input[inputLength - 2] == '=') {
            outputLength--;
        }
        
        // Resize result vector
        result.resize(outputLength);
        
        // Decode
        size_t i = 0;
        size_t j = 0;
        
        while (i < inputLength) {
            uint32_t a = i < inputLength && input[i] != '=' ? lookup[static_cast<uint8_t>(input[i])] : 0;
            uint32_t b = i + 1 < inputLength && input[i + 1] != '=' ? lookup[static_cast<uint8_t>(input[i + 1])] : 0;
            uint32_t c = i + 2 < inputLength && input[i + 2] != '=' ? lookup[static_cast<uint8_t>(input[i + 2])] : 0;
            uint32_t d = i + 3 < inputLength && input[i + 3] != '=' ? lookup[static_cast<uint8_t>(input[i + 3])] : 0;
            
            uint32_t triple = (a << 18) | (b << 12) | (c << 6) | d;
            
            if (j < outputLength) {
                result[j++] = (triple >> 16) & 0xFF;
            }
            if (j < outputLength) {
                result[j++] = (triple >> 8) & 0xFF;
            }
            if (j < outputLength) {
                result[j++] = triple & 0xFF;
            }
            
            i += 4;
        }
        
        return result;
    }
};

} // namespace Utils
} // namespace RPGEngine