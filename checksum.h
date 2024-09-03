#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;
namespace Checksum {
string generate_checksum(const vector<string>& chunks);
bool check_checksum(const vector<string>& chunks, const string& checksum);
string add_binary_strings(const string& a, const string& b);
}

#endif // CHECKSUM_H