#pragma once

#include <string>
#include <cstdint>
#include <sys/types.h>

// TCP
int tcp_server_listen(uint16_t port, int backlog = 16);
int tcp_accept(int listen_fd, std::string& out_ip, uint16_t& out_port);
int tcp_connect(const std::string& ip, uint16_t port);

ssize_t tcp_send_all(int fd, const void* data, size_t len);
ssize_t tcp_recv_some(int fd, void* buf, size_t maxlen);

//UDP
int udp_bind(uint16_t port);
int udp_socket();

ssize_t udp_send_to(int fd, const std::string& ip, uint16_t port, const void* data, size_t len);
ssize_t udp_recv_from(int fd, void* buf, size_t maxlen, std::string& out_ip, uint16_t& out_port);

void close_fd(int fd);

