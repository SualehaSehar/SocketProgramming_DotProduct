#include <stdio.h>
#include <stdlib.h>
#include <String>
#include <pthread.h>
#include<iostream>
#include <windows.h>
#include<winsock.h>
#include <string.h>
#include <thread>
#include <vector>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
using namespace std;
using std::cout;
using std::endl;
using std::cin;
using std::string;
#define buf_size 1024
#define name_size 20
bool exit_flag = false;
string name = "";
vector<int> vector1;
vector<int> vector2;

void* compute(void* arg) {
    int client_socket = *((int*)arg);
    int i = 0;
    int data;
    int data2 = 0;
    cout << endl << "processing";


    while (true) {

        char* data_ptr2 = reinterpret_cast<char*>(&data2);
        recv(client_socket, data_ptr2, sizeof(data2), 0);
        int* int_ptr = reinterpret_cast<int*>(data_ptr2);
        data2 = *int_ptr;

        if (data2 == -1) { break; }
        vector1.insert(vector1.begin() + i, data2);
        i++;

        cout << endl << "vector1 receivng " << data2;
    }

    cout << endl << "vector1 received";


    i = 0;
    while (true) {

        char* data_ptr3 = reinterpret_cast<char*>(&data2);
        recv(client_socket, data_ptr3, sizeof(data2), 0);
        int* int_ptr1 = reinterpret_cast<int*>(data_ptr3);
        data2 = *int_ptr1;

        if (data2 == -1) { break; }
        vector2.insert(vector2.begin() + i, data2);
        i++;

        cout << endl << "vector2 receivng " << data2;
    }

    cout << endl << "vector2 received";


    int dot_product = 0;
    for (int i = 0; i < vector1.size(); i++) {
        dot_product += vector1[i] * vector2[i];
    }


    data = dot_product;
    const char* data_ptr = reinterpret_cast<const char*>(&data);
    send(client_socket, data_ptr, sizeof(data), 0);

    return NULL;
}

int main() {

    struct sockaddr_in server_addr;  // the address we want to connect to
    int client_socket;
    pthread_t rcv_thread;
    void* thread_return;

    //to provide env for socket programming in windows
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        cout << endl << "WSA failed!";
        exit(EXIT_FAILURE);
    }

    //create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    //Connect
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cout << endl << "Connection failed!";
        exit(EXIT_FAILURE);
    }

    cout << endl << "Connection Established." << endl;

    pthread_create(&rcv_thread, NULL, compute, (void*)&client_socket);
    pthread_join(rcv_thread, &thread_return);

    //close
    closesocket(client_socket);
    return 0;
}
