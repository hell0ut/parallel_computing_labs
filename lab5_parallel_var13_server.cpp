#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <map>
#include <fstream>
#include <thread>
#include <mutex>


#define prot IPPROTO_TCP;

std::mutex console;
std::mutex clients_vector;


/*
1.GET MENU (GET,no args)
2.ADD POSITION TO ORDER (POST,arg1 id,arg2 quantity)
3.EDIT POSITION (PUT,arg1 id,arg2 new_quantity)
4.REMOVE POSITION (DELETE,arg1 id)
5.CONFIRM ORDER AND CLOSE CONNECTION (CONFIRM,no args)
*/


const int GET = 0;
const int POST = 1;
const int PUT = 2;
const int DEL = 3;
const int CONFIRM = 4;

std::map <std::string, int> mapping;




struct order_row {
    int id;
    int quantity;
};

struct client {
    int id;
    SOCKET socket = INVALID_SOCKET;
    std::vector<order_row> order;
    client( SOCKET sock, std::vector<order_row> ord, int id_) :id(id_), socket(sock), order(ord) {};

};

struct iasa_product {
    int id;
    std::string name;
};

struct iasa_request {
    std::string method;
    int arg1;
    int arg2;

};

class IASA_REQUEST_DECODER {
public:
    static iasa_request char_to_request(char* request_text) {
        std::string arr[3];
        std::string request_str(request_text);
        std::stringstream ssin(request_str);
        int i = 0;
        while (ssin.good() && i < 3) {
            ssin >> arr[i];
            ++i;
        }
        iasa_request req;
        req.method = arr[0];
        if (req.method != "CONFIRM") {
            req.arg1 = std::stoi(arr[1]);
            if (req.method != "GET" && req.method != "DEL")
                req.arg2 = std::stoi(arr[2]);
        }
        return req;
    }

    static std::string request_to_char(iasa_request req) {
        std::string request_text = req.method + " " + std::to_string(req.arg1) + " " + std::to_string(req.arg2);
        return request_text;
    }

private:
    IASA_REQUEST_DECODER() {}
};



int shutdown_services(ADDRINFO* addrResult, SOCKET* ConnectSocket, std::string message, int result) {
    console.lock();
    std::cout << message << " " << result << std::endl;
    console.unlock();
    if (ConnectSocket != NULL) {
        closesocket(*ConnectSocket);
        *ConnectSocket = INVALID_SOCKET;
    }
    freeaddrinfo(addrResult);
    WSACleanup();
    return 1;
}



class FoodServer {
    SOCKET ListenSocket = INVALID_SOCKET;
    int result;
    ADDRINFO hints;
    ADDRINFO* addrResult = NULL;
    std::vector<client> clients;

    std::vector<iasa_product> menu{
        {1,"Pizza"},
        {2,"Tofu"},
        {3,"Gorchitsa"}
    };

    int get_client_index_by_id(int id) {
        int i = 0;
        clients_vector.lock();
        while (i < clients.size()) {
            if (clients[i].id == id) break;
            i++;
        }
        clients_vector.unlock();
        return i;
    }

    int speak_client(int client_id) {
        char recvBuffer[512];
        std::vector<order_row> order;
        bool close_connection = false;
        while (!close_connection) {
            ZeroMemory(recvBuffer, 512);
            int client_index = get_client_index_by_id(client_id);
            result = recv(clients[client_index].socket, recvBuffer, 512, 0);

            if (result > 0) {
                console.lock();
                std::cout << "Receieved data from client " << client_id << ":\n" << recvBuffer << std::endl;
                console.unlock();
                iasa_request cur_req = IASA_REQUEST_DECODER::char_to_request(recvBuffer);
                auto response = process_request(cur_req,client_index, close_connection);

                result = send(clients[client_index].socket, response.c_str(), (int)strlen(response.c_str()), 0);
                console.lock();
                std::cout << "Send data back to client" << client_id<<":\n" << response << std::endl;
                console.unlock();
                if (result == SOCKET_ERROR) return shutdown_services(addrResult, &clients[client_index].socket, "Sending data back failed, result = ", result);
                if (close_connection) {
                    break;
                }
                
            }
        }
        int client_index = get_client_index_by_id(client_id);
        result = shutdown(clients[client_index].socket, SD_SEND);
        if (result == SOCKET_ERROR) return shutdown_services(addrResult, &clients[client_index].socket, "Shutdown failed, result = ", result);
        //shutdown_services(addrResult, &ListenSocket, "Returned ", result);
        clients_vector.lock();
        clients.erase(clients.begin() + client_index);
        clients_vector.unlock();
        return 0;

    }

    std::string process_request(iasa_request request,int client_index,bool &close_connection) {
        std::string response;
        switch (mapping[request.method]) {
        case GET:
            response = get_response(request.arg1,client_index);
            break;
        case POST:
            response = post_response(request.arg1, request.arg2,client_index);
            break;
        case PUT:
            response = put_response(request.arg1, request.arg2, client_index);
            break;
        case DEL:
            response = del_response(request.arg1,client_index);
            break;
        case CONFIRM:
            response = confirm_response(close_connection, client_index);
            break;
        default:
            break;
        }
        return response;
    }
    
    std::string get_response(int arg1,int client_index) {
        std::string resp = "";
        if (arg1 == 0){
            resp += "Our menu is:\n";
            for (int i = 0; i < menu.size(); i++) {
                resp += std::to_string(menu[i].id) + " " + menu[i].name + "\n";
            }
        }
        else {

            resp += "Order is:\n";
            for (int i = 0; i < clients[client_index].order.size(); i++) {
                for (int j = 0; j < menu.size(); j++) {
                    if (clients[client_index].order[i].id == menu[j].id) {
                        resp+= menu[i].name + " with quantity of " +std::to_string(clients[client_index].order[i].quantity) + "\n";
                        break;
                    }
                }

            }

        }
        return resp;
    }

    std::string post_response(int id,int quantity,int client_index) {
        clients[client_index].order.push_back({ id,quantity });
        std::string response = " Successfully added";
        return response;
    }

    std::string put_response(int id, int quantity, int client_index) {
        for (int i = 0; i < clients[client_index].order.size(); i++) {
            if (clients[client_index].order[i].id == id) clients[client_index].order[i].quantity = quantity;
        }
        std::string response = " Succseffuly changed quantity";
        return response;
    }

    std::string del_response(int id, int client_index) {
        int del_ind;
        for (int i = 0; i < clients[client_index].order.size(); i++) {
            if (clients[client_index].order[i].id == id) { del_ind = i; break; }
        }
        clients[client_index].order.erase(clients[client_index].order.begin()+del_ind);
        std::string response = "Succesfully deleted";
        return response;
    }

    std::string confirm_response(bool &close_connection, int client_index) {
        
        std::string response = "OK. Your order is saved,thank you! Goodbye!";
        std::ofstream myfile;
        std::string filename = std::to_string(client_index) +"_" + std::to_string(clock()) + ".txt";
        myfile.open(filename);
        myfile << "Order is:\n";
        for (int i = 0; i < clients[client_index].order.size(); i++) {
            for (int j = 0; j < menu.size(); j++) {
                if (clients[client_index].order[i].id == menu[j].id) {
                    myfile << menu[i].name << " with quantity of " << clients[client_index].order[i].quantity;
                    break;
                }
            }

        }
        myfile.close();
        close_connection = true;
        return response;
    }

    int listen_for_clients() {
        int i = 0;

        while (true) {
            console.lock();
            std::cout << "Server is waiting for connection" << std::endl;
            console.unlock();
            SOCKET ClientSocket = SOCKET_ERROR;
            while (ClientSocket == SOCKET_ERROR)
            {
                ClientSocket = accept(ListenSocket, NULL, NULL);
                if (ClientSocket == INVALID_SOCKET) return shutdown_services(addrResult, &ListenSocket, "Accepting socket failed, result = ", result);
            }
            std::thread th;
            std::vector<order_row> cur_order;
            client cur_client(ClientSocket,cur_order,i);
            clients.push_back(cur_client);
            std::thread thread(&FoodServer::speak_client, this, i);
            thread.detach();
            console.lock();
            std::cout << "Client connected" << std::endl;
            console.unlock();
            i++;
            
        }
    }

public:
    int start() {
        WSADATA wsaData;
        result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) return shutdown_services(addrResult, NULL, "WSAStartup failed, result = ", result);

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = prot;
        hints.ai_flags = AI_PASSIVE;

        result = getaddrinfo(NULL, "666", &hints, &addrResult);
        if (result != 0) return shutdown_services(addrResult, NULL, "getaddrinfo failed, result = ", result);

        ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
        if (ListenSocket == INVALID_SOCKET) return shutdown_services(addrResult, &ListenSocket, "Socket creation failed, result = ", result);

        result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
        if (result == SOCKET_ERROR) return shutdown_services(addrResult, &ListenSocket, "Binding socket failed, result = ", result);

        result = listen(ListenSocket, SOMAXCONN);
        if (result == SOCKET_ERROR) return shutdown_services(addrResult, &ListenSocket, "Listening socket failed, result = ", result);
        

        listen_for_clients();

    }

};


int main()
{
    mapping["GET"] = GET;
    mapping["DEL"] = DEL;
    mapping["PUT"] = PUT;
    mapping["POST"] = POST;
    mapping["CONFIRM"] = CONFIRM;
    FoodServer server;
    server.start();
    return 0;
}


