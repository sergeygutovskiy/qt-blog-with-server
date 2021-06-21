#include <iostream>
#include "server.hpp"

using namespace std;

// generate certificate
// sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout selfsigned.key -out selfsigned.crt



int main(int argc, char **argv)
{
    int port{3000};
    if (argc >= 2)
        port = std::stoi(argv[1]);

    server serv(port);

    serv.setCertPath("../data/certificates/selfsigned.crt");
    serv.setKeyPath("../data/certificates/selfsigned.key");

    if (!serv.start())
        std::cerr << "server can not be start, see above" << std::endl;

    return 0;
}
