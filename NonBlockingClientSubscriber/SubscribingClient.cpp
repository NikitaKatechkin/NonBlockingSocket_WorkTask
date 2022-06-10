#include "SubscribingClient.h"

#include <fstream>

void SubscribingLogger::Update()
{
    std::string filename = "./myfile.txt";

    // Checking if file created
    //std::ofstream creationOut(filename, std::ios_base::app);
    //creationOut.close();

    //Writing some useless thing
    std::ofstream out(filename, std::ios_base::app);

    if (out.is_open() == false)
    {
        throw std::exception();
    }

    out << "Something Happenned" << std::endl;
    out.close();
}
