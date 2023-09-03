#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include<iostream>
#include <windows.h>
#include<winsock.h>
#include <string.h>
#include <vector>
using namespace std;
#pragma comment(lib, "ws2_32.lib")
using std::cout;
using std::endl;
using std::cin;
using std::string;

#define MAX_CLIENT 200
int client_count = 0;
int client_sockets_arr[MAX_CLIENT];
pthread_mutex_t mutx;
int S = 6000;  // VECTOR LENGTH
int W = 2;  //NO OF CLIENTS
int WL = S / W;
vector<int> vector1(S);
vector<int> vector2(S);
int pointer = 0;
int pointer2 = 0;
int sum;

void* handle_client(void* arg)
{
    int client_socket = *((int*)arg);
    int data;
    int data2;


    pthread_mutex_lock(&mutx);

    for (int i = 0; i < WL; i++)
    {
        data = vector1[pointer];
        const char* data_ptr = reinterpret_cast<const char*>(&data);
        send(client_socket, data_ptr, sizeof(data), 0);
        pointer++;
    }

    // sending -1 to stop receving ata client side
    data = -1;
    const char* data_ptr0 = reinterpret_cast<const char*>(&data);
    send(client_socket, data_ptr0, sizeof(data), 0);

    for (int i = 0; i < WL; i++)
    {
        data = vector2[pointer2];
        const char* data_ptr1 = reinterpret_cast<const char*>(&data);
        send(client_socket, data_ptr1, sizeof(data), 0);
        pointer2++;
    }

    // sending -1 to stop receving ata client side
    data = -1;
    const char* data_ptr2 = reinterpret_cast<const char*>(&data);
    send(client_socket, data_ptr2, sizeof(data), 0);
    pthread_mutex_unlock(&mutx);

    pthread_mutex_lock(&mutx);

    char* data_ptr3 = reinterpret_cast<char*>(&data2);
    recv(client_socket, data_ptr3, sizeof(data2), 0);
    int* int_ptr = reinterpret_cast<int*>(data_ptr3);
    data2 = *int_ptr;

    cout << endl << "Partial_SUM:  " << data2;

    sum += data2;
    pthread_mutex_unlock(&mutx);

    //remove disconnected client
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < client_count; i++) {
        if (client_socket == client_sockets_arr[i]) {
            while (i < client_count - 1) {
                client_sockets_arr[i] = client_sockets_arr[i + 1];
            }
            break;
        }
    }
    client_count--;
    pthread_mutex_unlock(&mutx);

    closesocket(client_socket);
    return NULL;
}


int main() {

    struct sockaddr_in server_addr, client_addr;
    int server_socket, client_socket, client_addr_len;
    char buffer[1024];
    pthread_t tid;
    void* thread_return;

    for (int i = 0; i < S; i++) {
        vector1[i]= 1 + (rand() % 5);
        vector2[i]= 1 + (rand() % 5);
    }

    //to provide env for socket programming in windows
    WSADATA ws;

    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        cout << endl << "WSA failed!";
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&mutx, NULL);
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // 0 USES UNDERLINK PROTOCOL (MEANS WHAT MY SYSTEM'S UNDERLINK FRAMEWORK IS USING)

    //binding
    memset(&server_addr, 0, sizeof(server_addr)); //to set all the socket structures with null values
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // my system is server so it will pick my system's ip address 

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cout << endl << "Binding to local port failed!";
        exit(EXIT_FAILURE);
    }

    //listening
    int listening = listen(server_socket, 5); //more than 5 clients will go to waiting queue and thes 5 clients will be in active queue
    if (listening < 0) {
        cout << endl << "listening to local port failed!";
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < W; i++) {

        cout << endl << "Waiting for client..." << endl;
        //accepting
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        //entering critical section here
        pthread_mutex_lock(&mutx);
        client_sockets_arr[client_count++] = client_socket;
        pthread_mutex_unlock(&mutx);

        pthread_create(&tid, NULL, handle_client, (void*)&client_socket);
        //pthread_detach(tid);
        pthread_join(tid,&thread_return);
    }

    if (S % W != 0) {
        sum += vector1[vector1.size()-1] * vector2[vector2.size() - 1];
        //sum += vector1[pointer] * vector2[pointer];
    }

    cout << endl << "Dot Product:  " << sum;

    closesocket(server_socket);
    return 0;
}
