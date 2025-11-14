#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdlib.h>

//****************************************************************************************************************
// "Golden" functions to check correctness
std::vector<int> encoding(std::string s1)
{
    std::unordered_map<std::string, int> table;
    
    // initialize the hash table with each ASCII character 
    for (int i = 0; i <= 255; i++) {
        std::string ch = "";
        ch += char(i);
        table[ch] = i;
    }
 
    std::string p = "", c = ""; // p: prefix; c: current character
    
    // Fix for empty string
    if (s1.length() == 0) {
        return std::vector<int>(); // Return empty vector
    }

    p += s1[0]; 
    int code = 256; 
    std::vector<int> output_code;
    for (int i = 0; i < s1.length(); i++) {
        // set c as the next character
        // operator += is used because c is of string type, even though it actually denotes a character
        if (i != s1.length() - 1)
            c += s1[i + 1];
        // update the prefix as the combination of p and c if p + c exists in the hash table 
        if (table.find(p + c) != table.end()) {
            p = p + c;
        }
        // output p and encode p + c into the hash table if no matching exists 
        else {
            output_code.push_back(table[p]);
            table[p + c] = code;
            code++; 
            p = c;
        }
        c = "";
    }
    output_code.push_back(table[p]);
    return output_code;
}

void decoding(std::vector<int> op)
{
    std::cout << "\nDecoding\n";
    std::unordered_map<int, std::string> table;
    for (int i = 0; i <= 255; i++) {
        std::string ch = "";
        ch += char(i);
        table[i] = ch;
    }
    
    // Fix for empty input
    if (op.size() == 0) {
        return;
    }

    int old = op[0], n;
    std::string s = table[old];
    std::string c = "";
    c += s[0];
    std::cout << s;
    int count = 256;
    for (int i = 0; i < op.size() - 1; i++) {
        n = op[i + 1];
        if (table.find(n) == table.end()) {
            s = table[old];
            s = s + c;
        }
        else {
            s = table[n];
        }
        std::cout << s;
        c = "";
        c += s[0];
        table[count] = table[old] + c;
        count++;
        old = n;
    }
}
