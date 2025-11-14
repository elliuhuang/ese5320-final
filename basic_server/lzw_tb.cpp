#include <iostream>
#include <string>
#include <vector>
#include <cstring>   // For memcmp and memcpy
#include <iomanip>   // For printing hex

// Golden encoding function
std::vector<int> encoding(std::string s1);

// Golden decoding function (not used by this testbench, but good to have)
void decoding(std::vector<int> op);

void lzw_fpga(unsigned char *chunk, int chunk_len, unsigned char *compressed, int *compressed_length);

#include "lzw_playground.cpp"

// These MUST match the defines in lzw_hw.cpp
#define MAX_CHUNK_SIZE 8192
#define CODE_LENGTH 13

#define MAX_OUTPUT_SIZE 16384

/**
 * @brief Prints a byte buffer in hexadecimal for debugging.
 */
void print_hex(const char* title, unsigned char* data, int len) {
    std::cout << title << " (" << len << " bytes):\n";
    if (len == 0) {
        std::cout << "(empty)\n\n";
        return;
    }
    std::cout << std::hex << std::setfill('0');
    for (int i = 0; i < len; ++i) {
        std::cout << std::setw(2) << (int)data[i] << " ";
        if ((i + 1) % 16 == 0 && (i + 1) != len) std::cout << "\n";
    }
    std::cout << std::dec << "\n\n"; // Reset to decimal
}

/**
 * @brief "Golden" bit-packer.
 * This function mimics the exact bit-packing logic from lzw_fpga.cpp
 * to create an identical "golden" output stream for comparison.
 */
void pack_codes_golden(const std::vector<int>& codes, unsigned char* compressed, int* compressed_length) {
    int bit_pos = 0;
    int byte_pos = 0;
    memset(compressed, 0, MAX_OUTPUT_SIZE); // Clear buffer

    for (int code_int : codes) {
        uint16_t code = (uint16_t)code_int; // Cast to uint16_t

        // Write CODE_LENGTH bits, MSB first
        for (int bit = CODE_LENGTH - 1; bit >= 0; bit--) {
            int bit_value = (code >> bit) & 1;
            if (bit_value) {
                compressed[byte_pos] |= (1 << (7 - bit_pos));
            }
            bit_pos++;
            if (bit_pos == 8) {
                bit_pos = 0;
                byte_pos++;
                if (byte_pos >= MAX_OUTPUT_SIZE) {
                    std::cerr << "Golden pack buffer overflow!" << std::endl;
                    *compressed_length = byte_pos;
                    return;
                }
            }
        }
    }

    // If we're not on a byte boundary, move to next byte
    if (bit_pos != 0) {
        byte_pos++;
    }
    *compressed_length = byte_pos;
}

/**
 * @brief Generates a string of purely random bytes.
 * LZW works on bytes (char), so we generate values 0-255.
 */
std::string generate_random_string(int length) {
    if (length > MAX_CHUNK_SIZE) {
        length = MAX_CHUNK_SIZE;
    }
    std::string s;
    s.reserve(length);
    for (int i = 0; i < length; ++i) {
        // Generate a random byte
        s += static_cast<char>(rand() % 256);
    }
    return s;
}

/**
 * @brief Generates a string with high compressibility.
 * It does this by creating a few small patterns and repeating them.
 */
std::string generate_compressible_string(int length) {
    if (length > MAX_CHUNK_SIZE) {
        length = MAX_CHUNK_SIZE;
    }
    std::string s;
    s.reserve(length);

    // Create a few short patterns
    std::vector<std::string> patterns;
    patterns.push_back("WYS*WYG");
    patterns.push_back(generate_random_string(5));
    patterns.push_back("This is a test. ");
    patterns.push_back(std::string(10, 'A') + std::string(10, 'B'));
    
    int pattern_count = patterns.size();
    while (s.length() < length) {
        s += patterns[rand() % pattern_count];
    }
    
    // Trim to exact length
    return s.substr(0, length);
}


/**
 * @brief Runs a single LZW compression test.
 * @return true if test passed, false otherwise.
 */
bool run_test(const std::string& test_name, const std::string& input_str) {
    std::cout << "--- Running Test: " << test_name << " ---\n";
    // Don't print the whole string if it's long
    if (input_str.length() < 100) {
        std::cout << "Input: \"" << input_str << "\"\n";
    } else {
        std::cout << "Input: (string of length " << input_str.length() << ")\n";
    }
    std::cout << "  Input Length: " << input_str.length() << " bytes\n";

    // --- 1. Golden Path ---
    std::vector<int> golden_codes = encoding(input_str);
    unsigned char golden_compressed[MAX_OUTPUT_SIZE];
    int golden_len = 0;
    pack_codes_golden(golden_codes, golden_compressed, &golden_len);

    // --- 2. Hardware Path ---
    unsigned char hw_compressed[MAX_OUTPUT_SIZE];
    int hw_len = 0;

    // We must copy the const string data into a mutable buffer
    // for the lzw_fpga function.
    int input_len = input_str.length();
    if (input_len > MAX_CHUNK_SIZE) {
        std::cout << "TEST FAILED: Input string too large (" << input_len << " bytes). Max is " << MAX_CHUNK_SIZE << "\n";
        return false;
    }
    
    unsigned char input_chunk[MAX_CHUNK_SIZE];
    // Use input_str.data() to handle potential embedded nulls
    memcpy(input_chunk, input_str.data(), input_len);

    // Call the function under test
    lzw_fpga(input_chunk, input_len, hw_compressed, &hw_len);

    // --- 3. Comparison ---
    bool pass = true;
    if (golden_len != hw_len) {
        std::cout << "****************************************\n";
        std::cout << "TEST FAILED: Length mismatch!\n";
        std::cout << "  Golden Length: " << golden_len << "\n";
        std::cout << "  FPGA Length:   " << hw_len << "\n";
        std::cout << "****************************************\n";
        pass = false;
    } else if (input_len > 0 && memcmp(golden_compressed, hw_compressed, golden_len) != 0) {
        // Only check memcmp if length > 0
        std::cout << "****************************************\n";
        std::cout << "TEST FAILED: Content mismatch!\n";
        std::cout << "****************************************\n";
        pass = false;
    } else if (input_len == 0 && golden_len == 0 && hw_len == 0) {
        // Special case for empty string, this is a PASS
        pass = true;
    }

    if (pass) {
        std::cout << "TEST PASSED!\n";
        double ratio = (input_len > 0) ? (double)golden_len / input_len : 0;
        std::cout << "  Output Length: " << golden_len << " bytes\n";
        std::cout << "  Compression Ratio: " << std::fixed << std::setprecision(2) << ratio << "\n";
    } else {
        // On failure, print both buffers for debugging (only if small)
        if (golden_len < 256 && hw_len < 256) {
             print_hex("Golden Compressed", golden_compressed, golden_len);
             print_hex("FPGA Compressed", hw_compressed, hw_len);
        } else {
            std::cout << "  (Output buffers are too large to print)\n";
        }
    }
    
    std::cout << "-------------------------------------------\n\n";
    return pass;
}


int main() {
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(NULL)));

    int passed_count = 0;
    int total_count = 0;

    std::vector<std::pair<std::string, std::string>> tests;
    
    // --- Standard Tests ---
    tests.push_back({"User's String", "WYS*WYGWYS*WYSWYSG"});
    tests.push_back({"Empty String", ""});
    tests.push_back({"Single Char", "A"});
    tests.push_back({"Repeated Chars", "AAAAAAAAAA"});
    tests.push_back({"No Repetition", "ABCDEFGHIJK"});
    tests.push_back({"Classic Example", "TOBEORNOTTOBEORTOBEORNOT"});
    tests.push_back({"Long Repetition", "ABABABABABABABABABABABABABABABABABAB"});

    // --- New Large Tests ---
    const int SIZE_2K = 2048;
    const int SIZE_4K = 4096;
    const int SIZE_8K = 8192;

    tests.push_back({"2K Random (Worst Case)", generate_random_string(SIZE_2K)});
    tests.push_back({"2K Compressible (Best Case)", generate_compressible_string(SIZE_2K)});
    
    tests.push_back({"4K Random (Worst Case)", generate_random_string(SIZE_4K)});
    tests.push_back({"4K Compressible (Best Case)", generate_compressible_string(SIZE_4K)});
    
    // Test exact max size
    tests.push_back({"8K Random (Worst Case)", generate_random_string(SIZE_8K)});
    tests.push_back({"8K Compressible (Best Case)", generate_compressible_string(SIZE_8K)});

    for (const auto& test : tests) {
        total_count++;
        if (run_test(test.first, test.second)) {
            passed_count++;
        }
    }

    std::cout << "\n=====================\n";
    std::cout << "   Test Summary\n";
    std::cout << "=====================\n";
    std::cout << passed_count << " / " << total_count << " tests passed.\n\n";

    // Return 0 on success (all tests passed), 1 on failure
    return (passed_count == total_count) ? 0 : 1;
}