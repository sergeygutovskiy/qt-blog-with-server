#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <QString>
#include <QVariant>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlError>

class server
{
public:
    server(int aPort = 443);
    ~server();

    bool start();

    void setCertPath(std::string);
    void setKeyPath(std::string);
    void setPathForCA(std::string);

private:
    bool _db_init();
    bool _ssl_init();
    bool _socket_init();
    void _client_processing(int, std::string);

    QString db_register(QString name, QString password);
    QString db_login(QString name, QString password);
    QString db_auth(QString name, QString hash);
    QString db_posts(QString id);
    QString create_hash(QString password);

private:
    int m_socket{-1};
    int m_port{443};

    QSqlDatabase db;

    std::string m_certificate_path;
    std::string m_key_path;
    std::string m_ca_path;

    QString hash_key = "gorynych##sniper";

    SSL_METHOD *mp_ssl_method{nullptr};
    SSL_CTX *mp_ssl_context{nullptr};

};

#endif // SERVER_HPP
