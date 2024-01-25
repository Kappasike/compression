#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <bitset>
#include <chrono>
#include <sys/stat.h>

uint64_t get_file_size(std::string filename)
{
    struct stat stat_buf;
    uint64_t rc = stat(filename.c_str(), &stat_buf);

    return rc == 0 ? stat_buf.st_size : -1;
}

struct char_pair {
    int count;
    std::unordered_set<char> s;
};

struct char_key {
    char character;
    std::bitset<256> binary_key;
    int length = 0;
};

struct bitset_length {
    std::bitset<256> binary_key;
    int length = 0;
};

bool compare_by_count(const char_pair& a, const char_pair& b)
{
    return a.count < b.count;
}

void decrypt_file() {
    std::string input_file_name;
    std::string key_file_name;

    std::cout << "Enter a file to decompress: ";
    std::cin >> input_file_name;
    std::cout << "Enter the skykey file: ";
    std::cin >> key_file_name;

    std::ifstream key_file(key_file_name, std::ios::binary);

    std::unordered_map<std::string, char> compressed_chars;

    if (key_file.is_open()) {
        key_file.seekg(0, std::ios::end);
        std::streampos fileSize = key_file.tellg();
        key_file.seekg(0, std::ios::beg);
        size_t numElements = fileSize / sizeof(char_key);
        std::vector<char_key> char_with_key_read(numElements);

        key_file.read(reinterpret_cast<char*>(char_with_key_read.data()), fileSize);
        key_file.close();

        for (const auto& i : char_with_key_read) {
            //compressed_chars[i.binary_key] = i.character;
        }

        // zip it up when you're done
        /*for (auto i = compressed_chars.begin(); i != compressed_chars.end(); i++) {
            std::cout << i->first << ": " << i->second << "\n";
        }*/
    }
    else {
        std::cout << "Can't read shit man\n";
    }

    std::ifstream input_file(input_file_name, std::ios::binary);

    std::string decompressed_output = "";
    std::string output_file_name = input_file_name.substr(0, input_file_name.find_last_of('.'));
    std::ofstream output_file(output_file_name, std::ios::binary);

    if (input_file.is_open())
    {
        input_file.seekg(0, std::ios::end);
        uint64_t file_size = get_file_size(input_file_name);
        input_file.seekg(file_size - 1);
        char ignore_byte;
        input_file.get(ignore_byte);

        int bits_to_ignore = static_cast<unsigned char>(ignore_byte) + 8;

        input_file.seekg(0);

        char byte;
        std::string current_pattern = "";
        int bits_remaining = (file_size * 8) - bits_to_ignore;
        while (input_file.get(byte) && bits_remaining > 0)
        {
            for (int i = 7; i >= 0; i--)
            {

                bool bit = (byte >> i) & 1;
                char bit_char = bit ? '1' : '0';
                current_pattern += bit_char;
                if (compressed_chars.count(current_pattern) && bits_remaining > 0)
                {
                    //std::cout << compressed_chars[current_pattern];
                    output_file.put(compressed_chars[current_pattern]);
                    current_pattern = "";
                }
                bits_remaining--;
            }
        }
        input_file.close();
    }
    else {
        std::cout << "Can't read the damn input file\n";
    }
    output_file.close();

    std::cout << "Finished!\n";
    std::cin;
}

void encrypt_file() {
    auto start = std::chrono::high_resolution_clock::now();
    std::string input_name;
    std::cout << "Enter file: ";
    std::cin >> input_name;
    std::ifstream input_file(input_name, std::ios::binary);

    if (!input_file.is_open())
    {
        std::cout << input_name;
        std::cout << " failed to open :(";
        return;
    }

    std::streampos original_file_size = get_file_size(input_name);

    std::unordered_map<char, bitset_length> compressed_chars;
    std::vector<char_pair> char_pairs;
    std::unordered_set<char> chars;

    char curr_char;
    while (input_file.get(curr_char)) {
        if (chars.count(curr_char))
        {
            for (int i = 0; i < char_pairs.size(); i++)
            {
                if (char_pairs[i].s.count(curr_char))
                {
                    char_pairs[i].count = 1 + char_pairs[i].count;
                }
            }
        }
        else
        {
            chars.insert(curr_char);
            char_pair c;
            c.count = 1;
            c.s.insert(curr_char);
            char_pairs.push_back(c);
        }
    }

    std::sort(char_pairs.begin(), char_pairs.end(), compare_by_count);

    while (char_pairs.size() >= 2)
    {
        char_pair cp;

        cp.count = char_pairs[0].count + char_pairs[1].count;
        for (const auto& i : char_pairs[0].s) {
            compressed_chars[i].length++;
            cp.s.insert(i);
        }
        for (const auto& i : char_pairs[1].s) {
            compressed_chars[i].binary_key.set(compressed_chars[i].length);
            compressed_chars[i].length++;
            cp.s.insert(i);
        }
        char_pairs.erase(char_pairs.begin());
        char_pairs.erase(char_pairs.begin());
        char_pairs.push_back(cp);
        std::sort(char_pairs.begin(), char_pairs.end(), compare_by_count);
    }

    std::vector<char_key> char_with_key;

    for (auto i = compressed_chars.begin(); i != compressed_chars.end(); i++) {
        char_key ck;
        ck.character = i->first;
        ck.binary_key = i->second.binary_key;
        ck.length = i->second.length;
        char_with_key.push_back(ck);
    }

    // zip it up when you're done
    /*for (auto i = compressed_chars.begin(); i != compressed_chars.end(); i++) {
        std::cout << i->first << ": ";
        for (int j = i->second.length-1; j >= 0; j--)
        {
            std::cout << i->second.binary_key[j];
        }
        std::cout << "\n";
    }*/

    std::vector<char> bitset_buffer;

    std::cout << "output file bytes: ";

    input_file.clear();
    input_file.seekg(0);
    std::bitset<8> key_to_byte;
    char it_count = 8;
    bool bContinue = true;
    while (input_file.get(curr_char))
    {
        for (int i = compressed_chars[curr_char].length - 1; i >= 0; i--)
        {
            //std::cout << compressed_chars[curr_char].binary_key[i];
            key_to_byte.set(it_count - 1, compressed_chars[curr_char].binary_key[i]);
            it_count--;
            if (!it_count || (i==0 && input_file.peek() == EOF))
            {
                it_count = 8;

                bitset_buffer.push_back(static_cast<char>(key_to_byte.to_ulong()));
                key_to_byte.reset();
            }
        }
    }

    /*std::cout << "\nvector file bytes: ";
    for (const auto& element : bitset_buffer) {
        std::cout << element;
    }

    std::cout << "\nMade it this far!\n";*/

    input_file.close();

    /*
    * Iterate through every 8 characters and write that as one byte to output and I'm not sure what to do for last character
    */

    //std::cout << output_bitset << " - " << output_bitset.size() << " bits\n";
    //std::cout << output << " - " << output_byte_size << " bytes\n";

    /*for (const char& c : output_byte_array) {
        std::bitset<8> binary_val(c);
        std::cout << binary_val;
    }*/

    

    std::cout << "\nFinished. Old file size: " << original_file_size << " bytes\nNew file size : " << bitset_buffer.size() + 1 << " bytes\n";
    std::string output_file_name = input_name + ".sky";
    std::ofstream output_file(output_file_name, std::ios::binary);

    output_file.write(&bitset_buffer[0], bitset_buffer.size());
    output_file.close();

    std::string key_output_file_name = input_name + ".skykey";
    std::ofstream key_output_file(key_output_file_name, std::ios::binary);

    key_output_file.write(reinterpret_cast<const char*>(char_with_key.data()), char_with_key.size() * sizeof(char_key));
    key_output_file.close();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time Taken: " << duration.count() <<" milliseconds\n";
    std::cin;
}

int main()
{
    int n;
    std::cout << "Kappa's Grand File Compressor\nCome one, come all!\n1. Encrypt a file!\n2. Decrypt a file!\n";
    std::cin >> n;
    if (n == 1)
    {
        encrypt_file();
    }
    else if (n == 2)
    {
        decrypt_file();
    }
    else {
        return 0;
    }
}
