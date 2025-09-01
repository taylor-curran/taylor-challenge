#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include <string>
#include <vector>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>

class UDPReceiver {
public:
    // Constructor - creates socket and binds to port
    explicit UDPReceiver(uint16_t port);
    
    // Destructor - closes socket
    ~UDPReceiver();
    
    // Disable copy constructor and assignment
    UDPReceiver(const UDPReceiver&) = delete;
    UDPReceiver& operator=(const UDPReceiver&) = delete;
    
    // Set socket to non-blocking mode
    void setNonBlocking(bool nonBlocking);
    
    // Receive data from socket
    // Returns number of bytes received, or -1 if no data available (non-blocking)
    // or 0 on error
    ssize_t receive(void* buffer, size_t bufferSize);
    
    // Get last sender's address info
    std::string getLastSenderAddress() const;
    uint16_t getLastSenderPort() const;
    
    // Check if socket is valid
    bool isValid() const { return socketFd_ >= 0; }
    
    // Get the port this receiver is bound to
    uint16_t getPort() const { return port_; }
    
private:
    int socketFd_;                     // Socket file descriptor
    uint16_t port_;                    // Port we're listening on
    struct sockaddr_in serverAddr_;    // Our address
    struct sockaddr_in senderAddr_;    // Last sender's address
    socklen_t senderAddrLen_;          // Size of sender address structure
};

#endif // UDP_RECEIVER_H
