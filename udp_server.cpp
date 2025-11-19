#include "network.hpp"
#include <iostream>

int main() {
    uint16_t port = 9090;

    int fd = udp_bind(port);
    if (fd == -1) {
        std::cerr << "Failed to bind UDP\n";
        return 1;
    }

    std::cout << "UDP server listening on port " << port << "...\n";

    char buf[1024];
    std::string ip;
    uint16_t client_port;

    while (true) {
        ssize_t n = udp_recv_from(fd, buf, sizeof(buf) - 1, ip, client_port);
        if (n == -1) {
            std::cerr << "recvfrom error\n";
            break;
        }
        buf[n] = '\0';

        std::cout << "Received from " << ip << ":" << client_port
                  << " -> " << buf;

        if (udp_send_to(fd, ip, client_port, buf, (size_t)n) == -1) {
            std::cerr << "sendto error\n";
            break;
        }
    }

    close_fd(fd);
    return 0;
}
