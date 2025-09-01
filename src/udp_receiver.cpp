#include "udp_receiver.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>

UDPReceiver::UDPReceiver(uint16_t port) : port_(port), socketFd_(-1) {
    // Create UDP socket
    socketFd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd_ < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return;
    }
    
    // Allow port reuse
    int optval = 1;
    if (setsockopt(socketFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        std::cerr << "Warning: Could not set SO_REUSEADDR: " << strerror(errno) << std::endl;
    }
    
    // Setup server address
    memset(&serverAddr_, 0, sizeof(serverAddr_));
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    serverAddr_.sin_port = htons(port);
    
    // Bind socket to port
    if (bind(socketFd_, (struct sockaddr*)&serverAddr_, sizeof(serverAddr_)) < 0) {
        std::cerr << "Error binding to port " << port << ": " << strerror(errno) << std::endl;
        close(socketFd_);
        socketFd_ = -1;
        return;
    }
    
    std::cout << "UDP Receiver listening on port " << port << std::endl;
    
    // Initialize sender address structure
    memset(&senderAddr_, 0, sizeof(senderAddr_));
    senderAddrLen_ = sizeof(senderAddr_);
}

UDPReceiver::~UDPReceiver() {
    if (socketFd_ >= 0) {
        close(socketFd_);
        std::cout << "UDP Receiver on port " << port_ << " closed" << std::endl;
    }
}

void UDPReceiver::setNonBlocking(bool nonBlocking) {
    if (socketFd_ < 0) return;
    
    int flags = fcntl(socketFd_, F_GETFL, 0);
    if (flags < 0) {
        std::cerr << "Error getting socket flags: " << strerror(errno) << std::endl;
        return;
    }
    
    if (nonBlocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    if (fcntl(socketFd_, F_SETFL, flags) < 0) {
        std::cerr << "Error setting socket to " 
                  << (nonBlocking ? "non-blocking" : "blocking") 
                  << " mode: " << strerror(errno) << std::endl;
    }
}

ssize_t UDPReceiver::receive(void* buffer, size_t bufferSize) {
    if (socketFd_ < 0) return 0;
    
    senderAddrLen_ = sizeof(senderAddr_);
    ssize_t bytesReceived = recvfrom(socketFd_, buffer, bufferSize, 0,
                                      (struct sockaddr*)&senderAddr_, &senderAddrLen_);
    
    if (bytesReceived < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available (non-blocking mode)
            return -1;
        }
        // Actual error
        std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
        return 0;
    }
    
    return bytesReceived;
}

std::string UDPReceiver::getLastSenderAddress() const {
    char addrStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(senderAddr_.sin_addr), addrStr, INET_ADDRSTRLEN);
    return std::string(addrStr);
}

uint16_t UDPReceiver::getLastSenderPort() const {
    return ntohs(senderAddr_.sin_port);
}
