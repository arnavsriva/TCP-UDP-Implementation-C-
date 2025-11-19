#include "network.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

static int make_sockaddr_ipv4(const std::string& ip, uint16_t port, sockaddr_in& addr)
{
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (ip.empty()) {
        addr.sin_addr.s_addr = INADDR_ANY; 
        return 0;
    }

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        return -1;
    }

    return 0;
}

void close_fd(int fd) {
    if (fd >= 0) {
        ::close(fd);
    }
}

//TCP

int tcp_server_listen(uint16_t port, int backlog) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close_fd(fd);
        return -1;
    }

    sockaddr_in addr;
    if (make_sockaddr_ipv4("", port, addr) == -1) {
        std::cerr << "Invalid server address\n";
        close_fd(fd);
        return -1;
    }

    if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close_fd(fd);
        return -1;
    }

    if (::listen(fd, backlog) == -1) {
        perror("listen");
        close_fd(fd);
        return -1;
    }

    return fd;
}

int tcp_accept(int listen_fd, std::string& out_ip, uint16_t& out_port) {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_fd = ::accept(listen_fd, (sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("accept");
        return -1;
    }

    char ipbuf[INET_ADDRSTRLEN];
    if (::inet_ntop(AF_INET, &client_addr.sin_addr, ipbuf, sizeof(ipbuf)) != nullptr) {
        out_ip = ipbuf;
    } else {
        out_ip = "";
    }
    out_port = ntohs(client_addr.sin_port);

    return client_fd;
}

int tcp_connect(const std::string& ip, uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    sockaddr_in addr;
    if (make_sockaddr_ipv4(ip, port, addr) == -1) {
        std::cerr << "Invalid IP address\n";
        close_fd(fd);
        return -1;
    }

    if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close_fd(fd);
        return -1;
    }

    return fd;
}

ssize_t tcp_send_all(int fd, const void* data, size_t len) {
    const char* p = static_cast<const char*>(data);
    size_t total = 0;

    while (total < len) {
        ssize_t n = ::send(fd, p + total, len - total, 0);
        if (n == -1) {
            if (errno == EINTR) continue; 
            perror("send");
            return -1;
        }
        if (n == 0) break; 
        total += static_cast<size_t>(n);
    }

    return static_cast<ssize_t>(total);
}

ssize_t tcp_recv_some(int fd, void* buf, size_t maxlen) {
    while (true) {
        ssize_t n = ::recv(fd, buf, maxlen, 0);
        if (n == -1 && errno == EINTR) {
            continue; 
        }
        return n; 
    }
}

//UDP

int udp_socket() {
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
    }
    return fd;
}

int udp_bind(uint16_t port) {
    int fd = udp_socket();
    if (fd == -1) return -1;

    sockaddr_in addr;
    if (make_sockaddr_ipv4("", port, addr) == -1) {
        std::cerr << "Invalid UDP bind address\n";
        close_fd(fd);
        return -1;
    }

    if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close_fd(fd);
        return -1;
    }

    return fd;
}

ssize_t udp_send_to(int fd, const std::string& ip, uint16_t port, const void* data, size_t len)
{
    sockaddr_in addr;
    if (make_sockaddr_ipv4(ip, port, addr) == -1) {
        std::cerr << "Invalid destination IP\n";
        return -1;
    }

    while (true) {
        ssize_t n = ::sendto(fd, data, len, 0, (sockaddr*)&addr, sizeof(addr));
        if (n == -1 && errno == EINTR) continue;
        return n;
    }
}

ssize_t udp_recv_from(int fd, void* buf, size_t maxlen, std::string& out_ip, uint16_t& out_port)
{
    sockaddr_in from{};
    socklen_t from_len = sizeof(from);

    while (true) {
        ssize_t n = ::recvfrom(fd, buf, maxlen, 0,
                               (sockaddr*)&from, &from_len);
        if (n == -1 && errno == EINTR) continue;

        if (n >= 0) {
            char ipbuf[INET_ADDRSTRLEN];
            if (::inet_ntop(AF_INET, &from.sin_addr, ipbuf, sizeof(ipbuf)) != nullptr) {
                out_ip = ipbuf;
            } else {
                out_ip.clear();
            }
            out_port = ntohs(from.sin_port);
        }
        return n;
    }
}
