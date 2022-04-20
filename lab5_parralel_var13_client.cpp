#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <chrono>
#include <thread>
#include <string>

#define prot IPPROTO_TCP; 



int shutdown_services(ADDRINFO* addrResult, SOCKET* ConnectSocket, std::string message, int result) {
    std::cout << message << " " << result << std::endl;
    if (ConnectSocket != NULL) {
        closesocket(*ConnectSocket);
        *ConnectSocket = INVALID_SOCKET;
    }
    freeaddrinfo(addrResult);
    WSACleanup();
    return 1;
}


int main()
{
    WSADATA wsaData;
    int result;
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    ADDRINFO hints;
    ADDRINFO* addrResult = NULL;

    if (result != 0) return shutdown_services(addrResult, NULL, "WSAStartup failed, result = ", result);

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = prot;

    result = getaddrinfo("localhost", "666", &hints, &addrResult);
    if (result != 0) return shutdown_services(addrResult, NULL, "getaddrinfo failed, result = ", result);

    SOCKET ConnectSocket = INVALID_SOCKET;

    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) return shutdown_services(addrResult, &ConnectSocket, "Socket creation failed, result = ", result);

    result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) return shutdown_services(addrResult, &ConnectSocket, "Unable to connect to server, result = ", result);

    std::cout << "Input your request:\n"<<
        "1.GET MENU (GET,arg 1 (type) 0=MENU 1=ORDER)\n"<<
        "2.ADD POSITION TO ORDER(POST, arg1 id, arg2 quantity)\n"<<
        "3.EDIT POSITION(PUT, arg1 id, arg2 new_quantity)\n"<<
        "4.REMOVE POSITION(DEL, arg1 id)\n"<<
        "5.CONFIRM ORDER AND CLOSE CONNECTION(CONFIRM, no args)\n";

    while (true) {
        std::string request;
        std::getline(std::cin, request);

        result = send(ConnectSocket, request.c_str(), (int)strlen(request.c_str()), 0);
        if (result == SOCKET_ERROR) return shutdown_services(addrResult, &ConnectSocket, "Send failed, result = ", result);

        std::cout << "Bytes sent: " << result << " bytes" << std::endl;
        char recvBuffer[512];
        ZeroMemory(recvBuffer, 512);
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            std::cout << "Receieved data:\n" << recvBuffer << std::endl;
        }
        else if (result == 0) {
            std::cout << "Connection closed" << std::endl;
            break;
        }
        else {
            std::cout << "Recv failed with error: " << result << std::endl;
            break;
        }
    }

    shutdown_services(addrResult, &ConnectSocket, "Returned ", result);
    return 0;


}


