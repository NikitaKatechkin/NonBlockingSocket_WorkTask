#include "Socket.h"
#include <iostream>

namespace CustomSocket
{
    /**
    Socket& Socket::operator=(const Socket& other)
    {
        Socket copy(other);

        std::swap(m_handle, copy.m_handle);
        m_IPVersion = other.m_IPVersion;

        return *this;
    }
    **/

    Socket& Socket::operator=(Socket&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_IPVersion, other.m_IPVersion);
        //other.m_handle = INVALID_SOCKET;
        
        return *this;
    }

    Socket::Socket(SocketHandle handle, IPVersion IPVersion):
        m_handle(handle), m_IPVersion(IPVersion)
    {
        if (m_IPVersion != IPVersion::IPv4)
        {
            throw std::exception("Using unknown socket IP version.");
        }
    }
    
    /**
    Socket::Socket(const Socket& socket)
    {
        m_handle = socket.m_handle;
        m_IPVersion = socket.m_IPVersion;
    }
    **/

    Socket::Socket(Socket&& socket) noexcept
    {
        std::swap(m_handle, socket.m_handle);
        std::swap(m_IPVersion, socket.m_IPVersion);
        //socket.m_handle = INVALID_SOCKET;
    }

    Socket::~Socket()
    {
        if (m_handle != INVALID_SOCKET)
        {
            Close();
        }
    }

    Result Socket::Create()
    {
        if (m_IPVersion != IPVersion::IPv4)
        {
            throw std::exception("Using unknown socket IP version.");
        }

        Result result = Result::Fail;

        if (m_handle == INVALID_SOCKET)
        {
            m_handle = socket(AF_INET,
                              SOCK_STREAM,
                              IPPROTO_TCP);

            if (m_handle != INVALID_SOCKET)
            {
                result = (SetSocketOption(Option::TCP_NoDelay, TRUE) != Result::Success) 
                    ? Result::Fail : Result::Success;
            }
        }
        else
        {
            std::cerr << "[Socket::create()] " << "ERROR: ";
            std::cerr << "Trying to create already created socket." << std::endl;
        }

        return result;
    }

    Result Socket::Close()
    {
        Result result = Result::Fail;

        if (m_handle != INVALID_SOCKET)
        {
            result = (closesocket(m_handle) != 0) ? Result::Fail : Result::Success;

            if (result != Result::Success)
            {
                //WSAGetLastError();

                std::cerr << "[Socket::close()] " << "ERROR: ";
                std::cerr << "Failed to close a socket with GLE = ";
                std::cerr << WSAGetLastError() << std::endl;
            }
            else
            {
                m_handle = INVALID_SOCKET;
            }
        }
        else
        {
            std::cerr << "[Socket::close()] " << "ERROR: ";
            std::cerr << "Trying to close already closed socket." << std::endl;
        }

        return result;
    }

    Result Socket::Bind(IPEndpoint endpoint)
    {
        sockaddr_in addr = endpoint.GetSockaddrIPv4();
        Result result = bind(m_handle, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in)) == 0 ?
                                                                    Result::Success : Result::Fail;

        if (result == Result::Fail)
        {
            //WSAGetLastError();

            std::cerr << "[Socket::Bind()] " << "ERROR: ";
            std::cerr << "Failed to bind a socket to port " << static_cast<int>(endpoint.GetPort()) << " with GLE = ";
            std::cerr << WSAGetLastError() << std::endl;
        }

        return result;
    }

    Result Socket::Listen(IPEndpoint endpoint, int backlog)
    {
        Result result = Bind(endpoint);

        if (result == Result::Success)
        {
            if (listen(m_handle, backlog) != 0)
            {
                //WSAGetLastError();
                std::cerr << "[Socket::Listen()] " << "ERROR: ";
                std::cerr << "Failed to listen a socket on port " << static_cast<int>(endpoint.GetPort());
                std::cerr << " with GLE = " << WSAGetLastError() << std::endl;

                result = Result::Fail;
            }
        }

        return result;
    }

    Result Socket::Accept(Socket& outSocket, IPEndpoint* outEndpoint)
    {
        sockaddr_in addr = {};
        int addr_len = sizeof(sockaddr_in);
        //sizeof
        SocketHandle acceptedConnectionHandle = accept(m_handle,
            reinterpret_cast<sockaddr*>(&addr),
            &addr_len); //Blocking Func

        Result result = (acceptedConnectionHandle == INVALID_SOCKET) ? Result::Fail : Result::Success;

        if (result == Result::Fail)
        {
            std::cerr << "[Socket::Accept()] " << "ERROR: ";
            std::cerr << "Failed to accept new connection on a socket ";
            std::cerr << "with GLE = " << WSAGetLastError() << std::endl;
        }
        else
        {
            //outSocket = Socket(acceptedConnectionHandle, IPVersion::IPv4); // Replace to setters
            outSocket.SetHandle(acceptedConnectionHandle);
            outSocket.SetIPVersion(IPVersion::IPv4);

            if (outEndpoint != nullptr)
            {
                *outEndpoint = IPEndpoint(reinterpret_cast<sockaddr*>(&addr));

            }

            /**
            IPEndpoint newConnectionEndpoint(reinterpret_cast<sockaddr*>(&addr));
            std::cout << "New connection accepted." << std::endl;
            std::cout << newConnectionEndpoint;
            **/
        }


        return result;
    }

    Result Socket::Connect(IPEndpoint endpoint)
    {
        sockaddr_in addr = endpoint.GetSockaddrIPv4();
        Result result = (connect(m_handle, 
                                 reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in)) == 0) ? 
                                                                    Result::Success : Result::Fail;

        if (result != Result::Success)
        {
            std::cerr << "[Socket::Connect()] " << "ERROR: ";
            std::cerr << "Failed to connect a socket to " << endpoint.GetIPString();
            std::cerr << " on port " << static_cast<int>(endpoint.GetPort());
            std::cerr << " with GLE = " << WSAGetLastError() << std::endl;
        }

        return result;
    }

    Result Socket::Send(const void* data, int numberOfBytes, int& bytesSent)
    {
        bytesSent = send(m_handle, reinterpret_cast<const char*>(data), numberOfBytes, NULL);

        Result result = (bytesSent == SOCKET_ERROR) ? Result::Fail : Result::Success;
        if (result != Result::Success)
        {
            std::cerr << "[Socket::Send()] " << "ERROR: ";
            std::cerr << "Failed to send data from a socket ";
            std::cerr << "with GLE = " << WSAGetLastError() << std::endl;
        }

        /**
        if (numberOfBytes != bytesSent)
        {
            std::cout << "[SERVICE INFO]:";
            std::cout << "Data was corrupted or partly sent." << std::endl;
        }
        **/

        return result;
    }

    Result Socket::Recieve(void* destination, int numberOfBytes, int& bytesRecieved)
    {
        bytesRecieved = recv(m_handle, reinterpret_cast<char*>(destination), numberOfBytes, NULL);

        Result result = Result::Fail;
        if (bytesRecieved == SOCKET_ERROR)
        {
            std::cerr << "[Socket::Send()] " << "ERROR: ";
            std::cerr << "Failed to recieve data on a socket ";
            std::cerr << "with GLE = " << WSAGetLastError() << std::endl;

            result = Result::Fail;
        }
        else if (bytesRecieved == 0)
        {
            result = Result::Fail;
        }
        /**
        else if (numberOfBytes != bytesRecieved)
        {
            std::cout << "[SERVICE INFO]:";
            std::cout << "Data was corrupted or partly recieved." << std::endl;
        }
        **/
        else
        {
            result = Result::Success;
        }

        return result;
    }

    Result Socket::SendAll(void* data, int& numberOfBytes)
    {
        int totalBytesSent = 0;
        Result result = Result::Success;

        while (totalBytesSent < numberOfBytes)
        {
            int bytesRemaining = numberOfBytes - totalBytesSent;
            int bytesSent = 0;
            
            result = Send(reinterpret_cast<char*>(data) + totalBytesSent, 
                          bytesRemaining, 
                          bytesSent);
            if (result == Result::Fail)
            {
                break;
            }

            totalBytesSent += bytesSent;
        }

        numberOfBytes = totalBytesSent;

        return result;
    }

    Result Socket::RecieveAll(void* data, int& numberOfBytes)
    {
        int totalBytesRecieved = 0;
        Result result = Result::Success;

        while (totalBytesRecieved < numberOfBytes)
        {
            int bytesRemaining = numberOfBytes - totalBytesRecieved;
            int bytesRecieved = 0;

            result = Recieve(reinterpret_cast<char*>(data) + totalBytesRecieved,
                bytesRemaining,
                bytesRecieved);
            if (result == Result::Fail)
            {
                break;
            }

            totalBytesRecieved += bytesRecieved;
        }

        numberOfBytes = totalBytesRecieved;

        return result;
    }

    SocketHandle Socket::GetHandle()
    {
        return m_handle;
    }

    IPVersion Socket::GetIPVersion()
    {
        return m_IPVersion;
    }

    void Socket::SetHandle(SocketHandle handle)
    {
        m_handle = handle;
    }

    void Socket::SetIPVersion(IPVersion ipVersion)
    {
        m_IPVersion = ipVersion;
    }
    
    Result Socket::SetSocketOption(Option option, BOOL value)
    {

        Result result = Result::Fail;

        switch (option)
        {
        case Option::IO_NonBlocking:
        {
            u_long blockValue = (value == TRUE) ? 1 : 0;
            result = (ioctlsocket(m_handle, FIONBIO, &blockValue) == SOCKET_ERROR) ? Result::Fail :
                                                                                     Result::Success;

            break;
        }
        case Option::TCP_NoDelay:
        {
            result = (setsockopt(m_handle, IPPROTO_TCP, TCP_NODELAY,
                                 (const char*)&value, sizeof(BOOL)) == 0) ? Result::Success :
                                                                            Result::Fail;
            break;
        }
        default:
            break;
        }

        if (result != Result::Success)
        {
            WSAGetLastError();
        }

        return result;
    }
}

