#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>

#include<QString>
#include <QDebug>

#include "server.hpp"
#include "http_request.hpp"


static void *get_in_addr(sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((sockaddr_in *)sa)->sin_addr);

    return &(((sockaddr_in6 *)sa)->sin6_addr);
}
//------------------------------------------------------------------------------
static std::string get_ip_addr(const sockaddr_in &aClientData)
{
    char ip[INET6_ADDRSTRLEN] = {0};
    inet_ntop(aClientData.sin_family, get_in_addr((sockaddr *)&aClientData), ip, sizeof ip);
    return std::string(ip);
}

//------------------------------------------------------------------------------
server::server(int aPort)
    : m_port(aPort)
{
}
//------------------------------------------------------------------------------
server::~server()
{
    if (mp_ssl_context != nullptr)
        SSL_CTX_free(mp_ssl_context);
    mp_ssl_context = nullptr;
}
//------------------------------------------------------------------------------
bool server::start()
{

    if (!_ssl_init())
    {
        std::cerr << "ssl init error" << std::endl;
        return false;
    }

    if (!_socket_init())
    {
        std::cerr << "socket init error" << std::endl;
        return false;
    }

    if (!_db_init())
    {
        std::cerr << "db init error" << std::endl;
        return false;
    }

    std::cout << "server: was started, port:" << m_port << std::endl;

    while (1)
    {
        int sock_cli;
        sockaddr_in sa_cli;
        socklen_t client_len = sizeof(sa_cli);

        sock_cli = accept(m_socket, (sockaddr *)&sa_cli, &client_len);
        if (sock_cli == -1)
        {
            std::cerr << "accept error:" << m_port << std::endl;
            continue;
        }

        std::string ip = get_ip_addr(sa_cli);
        _client_processing(sock_cli, ip);
    }

    return true;
}
//------------------------------------------------------------------------------
void server::setCertPath(std::string aPath)
{
    m_certificate_path = aPath;
}
//------------------------------------------------------------------------------
void server::setKeyPath(std::string aPath)
{
    m_key_path = aPath;
}
//------------------------------------------------------------------------------
void server::setPathForCA(std::string aPath)
{
    m_ca_path = aPath;
}
//------------------------------------------------------------------------------
bool server::_db_init()
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setDatabaseName("qt");
    db.setUserName("gorynych");
    db.setPassword("gorynych");
    if(!db.open()) {
        return false;
    }
    return true;
}
//------------------------------------------------------------------------------
bool server::_ssl_init()
{
    SSL_library_init();

    SSL_load_error_strings();

    mp_ssl_method = const_cast<SSL_METHOD *>(TLSv1_2_server_method());

    mp_ssl_context = SSL_CTX_new(mp_ssl_method);

    if (mp_ssl_context == NULL)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (SSL_CTX_use_certificate_file(mp_ssl_context, m_certificate_path.c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (SSL_CTX_use_PrivateKey_file(mp_ssl_context, m_key_path.c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (!SSL_CTX_check_private_key(mp_ssl_context))
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (m_ca_path.size() > 0)
    {
        if (SSL_CTX_load_verify_locations(mp_ssl_context, m_ca_path.c_str(), NULL) == 0)
        {
            ERR_print_errors_fp(stderr);
            return false;
        }
    }

    return true;
}
//------------------------------------------------------------------------------
bool server::_socket_init()
{
    m_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (m_socket == -1)
    {
        std::cerr << "Can't create socket" << std::endl;
        return false;
    }

    sockaddr_in sa_serv;
    memset(&sa_serv, 0, sizeof(sa_serv));
    sa_serv.sin_family = AF_INET;
    sa_serv.sin_addr.s_addr = INADDR_ANY;
    sa_serv.sin_port = htons(m_port);

    if (bind(m_socket, (sockaddr *)&sa_serv, sizeof(sa_serv)) != 0)
    {
        std::cerr << "can't bind port " << m_port << std::endl;
        return false;
    }

    if (listen(m_socket, 1000) != 0)
    {
        std::cerr << "Can't configure listening port" << std::endl;
        return false;
    }
    return true;
}
//------------------------------------------------------------------------------
QString server::db_register(QString name, QString password) {
    QSqlQuery query = QSqlQuery(db);
    QString hash = create_hash(password.toStdString().data());
    QString data = "insert into users (name, password, hash) values ('" + name + "', '" + password + "', '" + hash + "')";
//    std::cout << data.toStdString() << std::endl;

    if(!query.exec(data)) {
        std::cout << query.lastError().databaseText().toStdString() << std::endl;
        std::cout << query.lastError().driverText().toStdString() << std::endl;
        return NULL;
    }
    QVariant var = query.lastInsertId();
    std::cout << hash.toStdString() << std::endl;
    return "{\"status\": 1,\"data\": {\"hash\":\"" + hash + "\", \"id\":" + var.toString() + "}}";
}
//------------------------------------------------------------------------------
QString server::db_login(QString name, QString password) {

    QSqlQuery query = QSqlQuery(db);
    QString data = "select * from users where name='" + name + "'";
    QString hash = create_hash(password.toStdString().data());
    QVariant var;
    std::cout << data.data() << std::endl;

    if(!query.exec(data)) {
        std::cerr << query.lastError().databaseText().toStdString() << std::endl;
        std::cerr << query.lastError().driverText().toStdString() << std::endl;
        return "{\"status\": 0,\"data\": {\"hash\":\"\", \"id\":\"\"}}";
    }
    while (query.next()) {
        var = query.value(0);
        if(query.value(2).toString() == password) {
            return "{\"status\": 1,\"data\": {\"hash\":\"" + hash + "\", \"id\":" + var.toString() + "}}";
        }
    }
    return "{\"status\": 0,\"data\": {\"hash\":\"\", \"id\":\"\"}}";
}

//------------------------------------------------------------------------------
QString server::db_posts(QString id) {

    QSqlQuery query = QSqlQuery(db);
//    QString data = "select * from users where name='" + name + "'";

}
//------------------------------------------------------------------------------
QString server::create_hash(QString password) {
    QString hash(hash_key);
//    for(int i = 0, j = 0; i < 16; i++) {
//        hash[i] = hash_key[i] ^ password[j++];
//        if(j == (int)password.size()) {
//            j = 0;
//        }
//    }
    return hash_key;
}
//------------------------------------------------------------------------------
void server::_client_processing(int aSocket, std::string aIp)
{
    SSL *ssl;
    ssl = SSL_new(mp_ssl_context);
    SSL_set_fd(ssl, aSocket);

    // service connection
    char buffer[1024 * 16] = {0};
    int read_bytes{0};

    if (SSL_accept(ssl) == -1)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(aSocket);
        std::cout << "ssl_error1";
        return;
    }

    read_bytes = SSL_read(ssl, buffer, sizeof(buffer));

    if (read_bytes <= 0)
    {
        SSL_free(ssl);
        close(aSocket);
        std::cout << "ssl_error1";
        return;
    }

    std::cout << "ssl_error1";

    buffer[read_bytes] = '\0';

    http_request request;
    request.parse(buffer);

    std::cout << "connection from(" << aIp << ")"
              << " request uri \"" << request.uri() << "\"" << std::endl;

    if (request.type() != http_request::eType::GET)
    {
        std::cout << "unknown type of request. Not support." << std::endl;
        return;
    }

    std::string url1 = request.uri().substr(0, request.uri().find('?'));
    std::string url2 = request.uri().substr(request.uri().find('?') + 1);
    std::cout << url1 << std::endl << url2 << std::endl;
//    std::string m_html_data = "{\"status\": 1,\"data\": {\"hash\":\"123456\"}}";

    switch (url1.size())
    {
    case 15:
    {
        if (url1 == "/users/register")
        {
            std::string name = url2.substr(0, url2.find('&'));
            std::string password = url2.substr(url2.find('&') + 1);

            if(name.substr(0, 5) == "name=" && password.substr(0, 9) == "password=") {
                std::stringstream response;
                name = name.substr(5);
                password = password.substr(9);
                std::cout << name << std::endl << password << std::endl;
                QString r = db_register(QString::fromUtf8(name.c_str()), QString::fromUtf8(password.c_str()));
                if(r == NULL) {
                    std::cout << "error register" << std::endl;
                }

                response << "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: application/json\r\nContent-Length: " << r.size() << "\r\n\r\n";

                int sended = 0;

                sended = SSL_write(ssl, response.str().c_str(), response.str().length());

                if (sended > 0)
                {
                    SSL_write(ssl, r.toStdString().data(), r.size());
                }
            }
        }
        break;
    }
    case 12:
    {
        if (url1 == "/users/login")
        {

            std::string name = url2.substr(0, url2.find('&'));
            std::string password = url2.substr(url2.find('&') + 1);

            if(name.substr(0, 5) == "name=" && password.substr(0, 9) == "password=") {
                std::stringstream response;

                name = name.substr(5);
                password = password.substr(9);

                std::cout << name << std::endl << password << std::endl;
                QString r = db_login(QString::fromUtf8(name.c_str()), QString::fromUtf8(password.c_str()));
                std::cout << "foo";
                response << "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: application/json\r\nContent-Length: " << r.size() << "\r\n\r\n";
                int sended = 0;
                sended = SSL_write(ssl, response.str().c_str(), response.str().length());

                if (sended > 0)
                {
                    SSL_write(ssl, r.toStdString().data(), r.size());
                }
            }
            break;
        }
    }
    case 6:
    {
        if (url1 == "/posts")
        {

            std::string id = url2;

            if(id.substr(0, 3) == "id=") {
                std::stringstream response;

                id = id.substr(3);

                QString r = db_login(QString::fromUtf8(name.c_str()), QString::fromUtf8(password.c_str()));
                std::cout << "foo";
                response << "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: application/json\r\nContent-Length: " << r.size() << "\r\n\r\n";
                int sended = 0;
                sended = SSL_write(ssl, response.str().c_str(), response.str().length());

                if (sended > 0)
                {
                    SSL_write(ssl, r.data(), r.size());
                }
            }
            break;
        }
    }

    }
}
