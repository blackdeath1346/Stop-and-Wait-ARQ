#ifndef CRC_H
#define CRC_H

#include <string>
#include <vector>

using namespace std;
namespace crc{
// Function prototypes
string xorOperation(const string& a, const string& b);
string mod2div(const string& dividend, const string& divisor);
vector<string> encodeData(const vector<string>& chunks, const string& key);
bool checkRemainder(const vector<string>& chunks, const string& key);
}
#endif // CRC_H
