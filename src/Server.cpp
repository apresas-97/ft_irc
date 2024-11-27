#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080 // Port number to bind

int main() {
    // 1. Create a socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed." << std::endl;
        return -1;
    }

    // 2. Set socket options (optional, for example to reuse the address)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0) {
        std::cerr << "setsockopt failed." << std::endl;
        close(server_fd);
        return -1;
    }

    // 3. Bind the socket to a specific IP address and port
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        close(server_fd);
        return -1;
    }

    // 4. Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed." << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    // 5. Accept an incoming connection
    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_address, &client_addr_len);
    if (client_fd < 0) {
        std::cerr << "Accept failed." << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Connection established with client: "
              << inet_ntoa(client_address.sin_addr) << ":"
              << ntohs(client_address.sin_port) << std::endl;

    // 6. Receive data from the client
    char buffer[1024] = {0};
    int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        std::cerr << "Recv failed." << std::endl;
        close(client_fd);
        close(server_fd);
        return -1;
    }

    std::cout << "Received: " << buffer << std::endl;

    // 7. Send data back to the client
    const char* response = "Hello from server!";
    send(client_fd, response, strlen(response), 0);

    // 8. Close the client socket
    close(client_fd);

    // 9. Close the server socket
    close(server_fd);

    return 0;
}
