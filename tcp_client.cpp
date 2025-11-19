#include "network.hpp"
#include <iostream>
#include <string>

int main() {
    std::string ip = "127.0.0.1";
    uint16_t port = 8080;

    int fd = tcp_connect(ip, port);
    if (fd == -1) {
        std::cerr << "Failed to connect\n";
        return 1;
    }

    std::cout << "Connected to " << ip << ":" << port << "\n";

    std::string line;
    char buf[1024];

    while (true) {
        std::cout << "Enter message (or 'quit'): ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit") break;

        if (tcp_send_all(fd, line.c_str(), line.size()) == -1) {
            std::cerr << "send error\n";
            break;
        }

        ssize_t n = tcp_recv_some(fd, buf, sizeof(buf) - 1);
        if (n <= 0) {
            std::cerr << "server closed / error\n";
            break;
        }
        buf[n] = '\0';
        std::cout << "Echo: " << buf << "\n";
    }

    close_fd(fd);
    return 0;
}
