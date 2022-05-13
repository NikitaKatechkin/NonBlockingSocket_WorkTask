#include "NetworkInitializer.h"
#include <iostream>

namespace CustomSocket
{
    namespace NetworkAPIInitializer
    {
        bool Initialize()
        {
            WSADATA data;

            bool result = !(WSAStartup(MAKEWORD(2, 2), &data));

            if (result == false)
            {
                std::cerr << "Failed to start up the winsock API." << std::endl;
            }
            else
            {
                if ((LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2))
                {
                    std::cerr << "Could not find a correct version of the winsock API." << std::endl;

                    result = false;
                }
            }

            return result;
        }

        void Shutdown()
        {
            WSACleanup();
        }
    }
}


