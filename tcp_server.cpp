#include "network.hpp"
#include <iostream>
#include <string>

int main() {
    uint16_t port = 8080;

    int listen_fd = tcp_server_listen(port);
    if (listen_fd == -1) {
        std::cerr << "Failed to start TCP server\n";
        return 1;
    }
    std::cout << "TCP server listening on port " << port << "...\n";

    std::string client_ip;
    uint16_t client_port;

    int client_fd = tcp_accept(listen_fd, client_ip, client_port);
    if (client_fd == -1) {
        close_fd(listen_fd);
        return 1;
    }

    std::cout << "Client connected from " << client_ip
              << ":" << client_port << "\n";

    char buf[1024];
    while (true) {
        ssize_t n = tcp_recv_some(client_fd, buf, sizeof(buf) - 1);
        if (n == -1) {
            std::cerr << "recv error\n";
            break;
        } else if (n == 0) {
            std::cout << "Client disconnected\n";
            break;
        }

        buf[n] = '\0';
        std::cout << "Received: " << buf;

        if (tcp_send_all(client_fd, buf, static_cast<size_t>(n)) == -1) {
            std::cerr << "send error\n";
            break;
        }
    }

    close_fd(client_fd);
    close_fd(listen_fd);
    return 0;
}
