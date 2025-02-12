#include <iostream>
#include <cstring>      // memset
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <netinet/in.h> // sockaddr_in
#include <unistd.h> 

int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // 1️ Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed!\n";
        return -1;
    }

    // 2️ Allow the socket to be reused (optional)
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3️ Bind socket to an IP and port
    address.sin_family = AF_INET;          // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Listen on all network interfaces
    address.sin_port = htons(port);        // Convert to network byte order

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Binding failed!\n";
        close(server_fd);
        return -1;
    }

    // 4️ Start listening for connections
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listening failed!\n";
        close(server_fd);
        return -1;
    }

    std::cout << "Server is running on port " << port << "...\n";
    return server_fd;
}


//-------------------------------------------------------------------




void handle_client(int client_socket) {
    char buffer[1024] = {0};
    
    // 1 Read the request
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        std::cerr << "Failed to receive request.\n";
        close(client_socket);
        return;
    }

    std::cout << "Received request:\n" << buffer << "\n";

    // 2️ Respond with a basic HTTP response
    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: 20\r\n"
                           "\r\n"
                           "<h1>Hello, World!</h1>";

    send(client_socket, response.c_str(), response.size(), 0);

    // 3️ Close the connection
    close(client_socket);
}





//--------------------------------------------------------------------

int main() {
    int port = 8080;  // Choose your port
    int server_fd = create_server_socket(port);

    if (server_fd == -1)
        return 1;

    while (true) 
    {
        // 1️ Accept a client connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            std::cerr << "Failed to accept connection!\n";
            continue;
        }

        std::cout << "Client connected!\n";
        
        // 2️ Handle the client's request
        handle_client(client_socket);
    }

    // 3️ Close the server socket (this is never reached in this loop)
    close(server_fd);
    return 0;
}