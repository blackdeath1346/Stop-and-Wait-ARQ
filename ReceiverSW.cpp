#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <bitset>
#include "checksum.h"
#include "crc.h"

using namespace std;

string addBinary(string binary) 
{
    int n = binary.length();
    int carry = 1; // We start with a carry of 1, which represents the "+1" operation.

    for (int i = n - 1; i >= 0; i--) 
    {
        if (binary[i] == '1') 
        {
            binary[i] = '0'; // Flip '1' to '0' and keep the carry.
        } 
        else 
        {
            binary[i] = '1'; // Flip '0' to '1', no more carry needed.
            carry = 0;
            break;
        }
    }

    // If carry is still 1, it means we overflowed the most significant bit, so we need to add a new '1' at the beginning.
    if (carry == 1) 
    {
        binary = '1' + binary;
    }

    return binary;
}

bool isValidChecksum(const string& frame, int dataSize) 
{
    vector<string> check;
    for (int i = 0; i < dataSize; i += 32) 
    {
        check.push_back(frame.substr(i, 32)); 
    }

    string receivedChecksum = frame.substr(dataSize, 32);
    //string calculatedChecksum = Checksum::generate_checksum(check);
    return Checksum::check_checksum(check,receivedChecksum);
}

void send_ack(int sock, const string& ack) 
{
    if (send(sock, ack.c_str(), ack.size(), 0) < 0) 
    {
        cerr << "Error sending ACK!" << endl;
    }
}

string receive_data(int sock) 
{
    char buffer[1024] = {0}; // Adjust buffer size as needed
    int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) 
    {
        buffer[bytes_received] = '\0';
        string data(buffer);
        //cout << "Received Data: " << data << endl;
        return data;
    } 
    else if (bytes_received == 0) 
    {
        cerr << "Connection closed by the client!" << endl;
        return "";
    } 
    else 
    {
        cerr << "Error receiving data!" << endl;
        return "";
    }
}

int main() 
{
    const char* HOST = "127.0.0.1"; 
    const int PORT = 9998;
    const int PKT_SIZE = 384;

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) 
    {
        cerr << "Error creating socket!" << endl;
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        cerr << "Bind failed!" << endl;
        close(server_sock);
        return -1;
    }

    if (listen(server_sock, 3) < 0) 
    {
        cerr << "Error in listen!" << endl;
        close(server_sock);
        return -1;
    }

    cout << "Server is listening on port " << PORT << "..." << endl;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);

    if (client_sock < 0) 
    {
        cerr << "Error accepting connection!" << endl;
        close(server_sock);
        return -1;
    }

    cout << "Connection established with client!" << endl;

    int S = 0; // Expected frame number
    string num = "00000000"; // Frame number in binary

    while (true) 
    {
        string received_frame = receive_data(client_sock);

        if (received_frame.empty()) 
        {
            break; // Connection closed or error
        }

        string received_num = received_frame.substr(96, 8); // Extract frame number
        cout<<"receiver frame no:"<<num<<" received frame no: "<<received_num<<endl;

        // Verify if the frame number matches the expected frame and if the checksum is valid
        if (received_num == num && isValidChecksum(received_frame, 480)) 
        {
            cout<<"Received Frame No."<<S<<endl;
            //usleep(5000);
            send_ack(client_sock, num); // Send ACK
            //usleep(5000);
            num = addBinary(num); // Increment expected frame number
            S++; // Move to the next frame
        } 
        else 
        {
            //cout<<received_frame<<endl;
            //usleep(5000);
            send_ack(client_sock, "11111111"); // Send NAK (Negative Acknowledgment)
            //usleep(5000);
        }
    }

    close(client_sock);
    close(server_sock);
    return 0;
}
