#ifndef CUSTOM_H
#define CUSTOM_H

#include "QString"
#include "QList"

// страницы
enum class Pages
{
    HOME, REGISTER, ACCOUNT, LOGIN
};

// названия запросов
enum class Requests
{
    REGISTER, LOGIN, POSTS
};

// класс, отвечающий за аунтификацию юзера
class Auth
{
    public:
    QString id;
    QString name;
    QString password;
    QString hash;

    bool register_user(QByteArray answer);
    bool login_user(QByteArray answer);
    bool check();
    void logout();
};

// класс-модель для хранения инфы о публикации
class Post
{
    public:
    QString id;
    QString title;
    QString description;
    Post(QString id, QString title, QString description);
};

// класс-менеджер для управления публикациями
class Posts
{
    public:
    QList<Post> * parse_posts(QByteArray posts);
    Posts();
};

#endif // CUSTOM_H
