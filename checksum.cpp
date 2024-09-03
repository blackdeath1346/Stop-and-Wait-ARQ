#include "checksum.h"
namespace Checksum {
string generate_checksum(const vector<string>& chunks) {
    string res = "";
    int size = chunks[0].length();
    
    for (int i = 0; i < chunks.size(); ++i) {
        if (res != "") {
            res = add_binary_strings(res, chunks[i]);
        } else {
            res = chunks[i];
        }
    }
    
    if (res.length() > size) {
        string overflow = res.substr(0, res.length() - size);
        res = res.substr(res.length() - size);
        res = add_binary_strings(res, overflow);
    }
    
    for (int i = 0; i < res.length(); ++i) {
        res[i] = (res[i] == '0') ? '1' : '0';
    }
    
    return res;
}

bool check_checksum(const vector<string>& chunks, const string& checksum) {
    string res = "";
    int size = chunks[0].length();
    
    for (int i = 0; i < chunks.size(); ++i) {
        if (!chunks[i].empty()) {
            if (res != "") {
                res = add_binary_strings(res, chunks[i]);
            } else {
                res = chunks[i];
            }
        }
    }
    
    // Remove any overflow bits
    if (res.length() > size) {
        string overflow = res.substr(0, res.length() - size);
        res = res.substr(res.length() - size);
        res = add_binary_strings(res, overflow);
    }
    
    res = add_binary_strings(res, checksum);
    
    for (int i = 0; i < res.length(); ++i) {
        if (res[i] != '1') {
            return false;
        }
    }
    
    return true;
}

string add_binary_strings(const string& a, const string& b) {
    string result = "";
    int carry = 0;
    int len_a = a.length();
    int len_b = b.length();
    int max_len = max(len_a, len_b);
    
    for (int i = 0; i < max_len; ++i) {
        int bit_a = (i < len_a) ? a[len_a - 1 - i] - '0' : 0;
        int bit_b = (i < len_b) ? b[len_b - 1 - i] - '0' : 0;
        int sum = bit_a + bit_b + carry;
        result = char((sum % 2) + '0') + result;
        carry = sum / 2;
    }
    
    if (carry) {
        result = '1' + result;
    }
    
    return result;
}
}
