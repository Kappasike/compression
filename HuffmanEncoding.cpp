#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <map>
#include <bitset>

struct char_pair {
    int count;
    std::set<char> s;
};

bool compare_by_count(const char_pair& a, const char_pair& b)
{
    return a.count < b.count;
}

int main()
{
    std::string input;
    std::cout << "Enter file: ";
    std::cin >> input;
    std::ifstream input_file(input);

    if (!input_file.is_open())
    {
        std::cout << input;
        std::cout << " failed to open :(";
        return 0;
    }

    std::map<char, std::string> compressed_chars;
    std::vector<char_pair> char_pairs;
    std::set<char> chars;

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
            compressed_chars[i] = "0" + compressed_chars[i];
            cp.s.insert(i);
        }
        for (const auto& i : char_pairs[1].s) {
            compressed_chars[i] = "1" + compressed_chars[i];
            cp.s.insert(i);
        }
        char_pairs.erase(char_pairs.begin());
        char_pairs.erase(char_pairs.begin());
        char_pairs.push_back(cp);
        std::sort(char_pairs.begin(), char_pairs.end(), compare_by_count);
    }
    
    for (auto i = compressed_chars.begin(); i != compressed_chars.end(); i++) {
        std::cout << i->first << ": " << i->second << "\n";
    }

    std::string output = "";

    input_file.clear();
    input_file.seekg(0);
    while (input_file.get(curr_char)) {
        output = output + compressed_chars[curr_char];
    }

    input_file.close();

    //boost::dynamic_bitset<> output_bitset(output);

    /*
    * Iterate through every 8 characters and write that as one byte to output and I'm not sure what to do for last character
    */

    int output_byte_size = output.length()/8;
    char remainder = 8 - (output.length() % 8);
    if (remainder)
    {
        output_byte_size += 1;
    }

    //std::cout << output_bitset << " - " << output_bitset.size() << " bits\n";
    //std::cout << output << " - " << output_byte_size << " bytes\n";

    std::vector<char> output_byte_array;
    for (int i = 0; i < remainder; i++)
    {
        output += "0";
    }

    for (int i = 0; i < output_byte_size; i++) {
        char byte = 0;
        std::string byte_string = output.substr(i * 8, (i+1)*8);
        for (int i = 0; i < 8; ++i){
            if (byte_string[i] == '1') byte |= 1 << (7 - i);
        }
        output_byte_array.push_back(byte);
    }
    output_byte_array.push_back(remainder);

    /*for (const char& c : output_byte_array) {
        std::bitset<8> binary_val(c);
        std::cout << binary_val;
    }*/
    size_t dot_index = input.find('.');
    input = input.substr(0, dot_index);

    std::cout << "\nFinished. New file size: " << output_byte_size + 1 << " bytes";
    std::string output_file_name = input + ".monka";
    std::ofstream output_file(output_file_name, std::ios::binary);

    output_file.write(&output_byte_array[0], output_byte_array.size());
    output_file.close();
}