#include "crc.h"

using namespace std;

// Function definitions
namespace crc
{
string xorOperation(const string& a, const string& b) {
    string result;
    for (size_t i = 1; i < b.length(); ++i) {
        result += (a[i] == b[i]) ? '0' : '1';
    }
    return result;
}

string mod2div(const string& dividend, const string& divisor) {
    size_t pick = divisor.length();
    string tmp = dividend.substr(0, pick);
    
    for (size_t i = pick; i < dividend.length(); ++i) {
        if (tmp[0] == '1') {
            tmp = xorOperation(divisor, tmp) + dividend[i];
        } else {
            tmp = xorOperation(string(pick, '0'), tmp) + dividend[i];
        }
    }

    if (tmp[0] == '1') {
        tmp = xorOperation(divisor, tmp);
    } else {
        tmp = xorOperation(string(pick, '0'), tmp);
    }

    return tmp;
}

vector<string> encodeData(const vector<string>& chunks, const string& key) {
    size_t l_key = key.length();
    vector<string> enc_chunks;
    
    for (const auto& data : chunks) {
        string appended_data = data + string(l_key - 1, '0');
        string remainder = mod2div(appended_data, key);
        string codeword = data + remainder;
        enc_chunks.push_back(codeword);
    }
    return enc_chunks;
}

bool checkRemainder(const vector<string>& chunks, const string& key) {
    size_t l_key = key.length();
    
    for (const auto& data : chunks) {
        string appended_data = data + string(l_key - 1, '0');
        string remainder = mod2div(appended_data, key);
        if (remainder.find('1') != string::npos) 
        {
            return false;
        }
    }

    return true;
}

bool checkCRC(string &data, string key)
{
    size_t l_key = key.length();
    string appended_data = data + string(l_key - 1, '0');
    string remainder = mod2div(appended_data, key);
    if (remainder.find('1') != string::npos) 
    {
            return false;
    }
    return true;
}
}