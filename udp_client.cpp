#include "network.hpp"
#include <iostream>
#include <string>

int main() {
    std::string ip = "127.0.0.1";
    uint16_t port = 9090;

    int fd = udp_socket();
    if (fd == -1) {
        std::cerr << "Failed to create UDP socket\n";
        return 1;
    }

    std::string line;
    char buf[1024];
    std::string from_ip;
    uint16_t from_port;

    while (true) {
        std::cout << "Enter message (or 'quit'): ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit") break;

        if (udp_send_to(fd, ip, port, line.c_str(), line.size()) == -1) {
            std::cerr << "sendto error\n";
            break;
        }

        ssize_t n = udp_recv_from(fd, buf, sizeof(buf) - 1, from_ip, from_port);
        if (n == -1) {
            std::cerr << "recvfrom error\n";
            break;
        }
        buf[n] = '\0';

        std::cout << "Reply from " << from_ip << ":" << from_port
                  << " -> " << buf << "\n";
    }

    close_fd(fd);
    return 0;
}
