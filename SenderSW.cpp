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
#include <fstream>
#include <sstream>
#include <fcntl.h>
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
        } else 
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
void inject_error(string &text, int number) 
{
    if (number == 0) 
    {
        return;
    }
    for (int i = 0; i < number; i++) 
    {
        int x = rand() % text.length();
        text[x] = (text[x] == '0') ? '1' : '0'; 
    }
    return;
}
void injectBurstError(string &text, int x) {
    if (x <= 0 || text.empty()) {
        return;
    }

    
    srand(time(0));

    
    int start_index = rand() % (text.length() - x + 1);

    
    for (int i = 0; i < x; ++i) 
    {
        if(start_index + i==text.length())
        return;
        if (text[start_index + i] == '0') 
        {
            text[start_index + i] = '1';
        } else if (text[start_index + i] == '1') 
        {
            text[start_index + i] = '0';
        }
    }
}
void send_data(int sock, const string& data) 
{
    if (send(sock, data.c_str(), data.size(), 0) < 0) 
    {
        cerr << "Error sending data!" << endl;
    }
}

string receive_ack(int sock) 
{
    char buffer[1024] = {0}; // Adjust buffer size as needed
    int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) 
    {
        buffer[bytes_received] = '\0';
        string ack(buffer);
        cout << "Received ACK: " << ack << endl;
        return ack;
    } 
    else if (bytes_received == 0) 
    {
        cerr << "Connection closed by the server!" << endl;
        return "";
    } 
    else if (errno == EWOULDBLOCK || errno == EAGAIN) 
    {
        // No data available yet, continue with the loop
        return "";
    } 
    else 
    {
        cerr << "Error receiving ACK!" << endl;
        return "";
    }
}
int main() 
{
    const char* HOST = "127.0.0.1"; 
    const int PORT = 9998;
    const int PKT_SIZE = 376;
    string sAddr="101101011001110010100110101010110100010010111100";
    string dAddr="010010111100101011001110010010101111000010100110";

   
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) 
    {
        cerr << "Error creating socket!" << endl;
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, HOST, &server_addr.sin_addr) <= 0) 
    {
        cerr << "Invalid address/ Address not supported!" << endl;
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        cerr << "Error connecting to server!" << endl;
        close(sock);
        return -1;
    }
    string filename;
    string enc_text;

    // Ask for the filename
    cout << "Enter the filename: ";
    cin >> filename;

    // Open the file in read mode
    ifstream file(filename);

    // Check if the file is open
    if (file.is_open()) 
    {
        stringstream buffer;
        buffer << file.rdbuf(); // Read the file contents into the stringstream
        enc_text = buffer.str(); // Convert the stringstream into a string
        file.close(); // Close the file
    } 
    else 
    {
        cerr << "Unable to open file: " << filename << endl;
        return 1;
    }
   

    int actual_len = enc_text.length();
    if (actual_len % PKT_SIZE != 0) 
    {
        enc_text += string(PKT_SIZE - (actual_len % PKT_SIZE), '0'); 
    }

    //YOUR MAIN LOGIC STARTS HERE
    vector<string> chunks;//breaks entire data in 48 bytes payload
    for (size_t i = 0; i < enc_text.length(); i += PKT_SIZE) 
    {
        chunks.push_back(enc_text.substr(i, PKT_SIZE)); 
    }
    int S=0;//frame number
    string num="00000000";//frame number in binary
    bool canSend=true;
    string currframe;
    float prob;
    cout<<"enter probability for delay/error"<<endl;
    cin>>prob;
    //cout<<"Do you want error (y/n)"<<endl;
    //char ch;
    //cin>>ch;
    time_t start;
    srand(static_cast<unsigned>(time(0)));
   /* for(int i=0;i<chunks.size();i++)
    {
        cout<<chunks[i]<<endl;
        cout<<i<<endl;
    }*/
    //cout<<chunks.size();
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    while(true)
    {
        if(S==chunks.size()&&canSend==true)
        break;
       // cout << "Loop iteration: " << S << endl;
        string header=sAddr+dAddr+num;
        string data=header+chunks[S];
        if(canSend)
        {
            vector<string> check;
            for (size_t i = 0; i < data.size(); i += 32) 
            {
                check.push_back(data.substr(i, 32)); 
            }
            /*for(int i=0;i<check.size();i++)
            {
                cout<<check[i]<<endl;
            }*/
            string checksum= Checksum::generate_checksum(check);
            currframe=data+checksum;//ENTIRE FRAME IS MADE
            string toSend=currframe;//frame to send where error will be introduced
            //SIMULATING CHANNEL CONDITIONS
            //start
            /*WILL SIMULATE ERROR INJECTION HERE*/
            float x = static_cast<float>(rand()) / RAND_MAX;

            if (x < prob) 
            {
                int random_error_code = rand() % 384 + 1;
                inject_error(toSend,random_error_code);
            }
            //srand(static_cast<unsigned>(time(0)));
            x = static_cast<float>(rand()) / RAND_MAX;
            start=time(0);
            if (x < prob) 
            {
                sleep(5);
            }
            usleep(5000);
            send_data(sock, toSend);
            usleep(5000);
            cout<<"Sent Frame no. :"<<S<<endl;
            //end
            S++;
            num=addBinary(num);
            canSend=false;
        }
        //acknowldegment
        //cout<<"before ack"<<endl;
        string ack = receive_ack(sock);
        //cout<<"after ack"<<endl;
        if (!ack.empty()) 
        {
            if (addBinary(ack) == num) 
            {
                canSend = true; // ACK received correctly, ready to send the next frame
                continue;
            } 
            else if (ack == "11111111") 
            {
                //cout << "Negative ACK received. Resending the frame..." << endl;
                canSend = false; // NAK received, do not increment frame number
            }
        }
        time_t now=time(0);
        //cout<<now-start<<endl;
        if(now-start>=1.0)
        {
            string toSend=currframe;
            //srand(static_cast<unsigned>(time(0)));
            float x = static_cast<float>(rand()) / RAND_MAX;

            if (x < prob) 
            {
                int random_error_code = rand() % 384 + 1;
                inject_error(toSend,random_error_code);
            }
            //srand(static_cast<unsigned>(time(0)));
            x = static_cast<float>(rand()) / RAND_MAX;
            start=time(0);
            if (x < prob) 
            {
                sleep(5);
            }
            //usleep(5000);
            send_data(sock, toSend);
            //usleep(5000);
            cout<<"Sent fame no :"<<S-1<<" on timeout"<<endl;
            //end
        }
    }

    close(sock);
    return 0;
}