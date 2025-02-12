#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <vector>

#include "GETRequestHandler.hpp"


#define BUFFER_SIZE
class HTTPServer {
private:
    int server_fd;
    struct sockaddr_in address;
    std::vector<int> client_sockets;
    char buffer[1024];
    fd_set read_fds;
    int max_fd;

public:
    HTTPServer(int port) {
        // Create socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        // Set socket options
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            throw std::runtime_error("Setsockopt failed");
        }

        // Configure address
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Bind socket
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            throw std::runtime_error("Bind failed");
        }

        // Listen
        if (listen(server_fd, 3) < 0) {
            throw std::runtime_error("Listen failed");
        }

        // Set socket to non-blocking
        fcntl(server_fd, F_SETFL, O_NONBLOCK);
        max_fd = server_fd;
    }

    void run() 
    {
        while (true) {
            // std::cout<<"her\n";
            // Clear and set file descriptors
            FD_ZERO(&read_fds);
            FD_SET(server_fd, &read_fds);
            
            // Add client sockets to fd_set
            for (size_t i = 0; i < client_sockets.size(); i++) {
                FD_SET(client_sockets[i], &read_fds);
                if (client_sockets[i] > max_fd)
                    max_fd = client_sockets[i];
            }

            // Wait for activity
            int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
            if (activity < 0) {
                std::cout << "Select error" << std::endl;
                continue;
            }

            // Check for new connections
            if (FD_ISSET(server_fd, &read_fds)) {
                handleNewConnection();
            }

            // Check existing connections for data
            for (size_t i = 0; i < client_sockets.size(); i++) {
                if (FD_ISSET(client_sockets[i], &read_fds)) {
                    handleClientRequest(i);
                }
            }
        }
    }

private:
    void handleNewConnection() {
        socklen_t addrlen = sizeof(address);
        int new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        
        if (new_socket < 0) {
            // std::cout << "Accept failed" << std::endl;
            return;
        }

        // Set new socket to non-blocking
        fcntl(new_socket, F_SETFL, O_NONBLOCK);
        
        // Add to client sockets vector
        client_sockets.push_back(new_socket);
        // std::cout << "New connection established" << std::endl;
    }

    void handleClientRequest(size_t index) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(client_sockets[index], buffer, sizeof(buffer));
        
        if (valread <= 0) {
            // Connection closed or error
            close(client_sockets[index]);
            client_sockets.erase(client_sockets.begin() + index);
            return;
        }

        // Print the received request
        // std::cout << "Received Request:\n" << buffer << std::endl;
        std::cout << "Received Request:\n" << std::endl;
      
//********************************************
//********************************************
        GETRequestHandler GETrequest;
        GETrequest.handleRequest(buffer);
//********************************************
//********************************************

        // Send a basic response
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        send(client_sockets[index], response.c_str(), response.length(), 0);
    }
};

int main() {
    try {
        HTTPServer server(8080);
        std::cout << "Server started on port 8080" << std::endl;
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}


    // std::string trim(const std::string& str) {
    //     size_t first = str.find_first_not_of(" \t\n\r");
    //     size_t last = str.find_last_not_of(" \t\n\r");
    //     if (first == std::string::npos || last == std::string::npos)
    //         return "";
    //     return str.substr(first, last - first + 1);
    // }