#include "custom.h"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QList>

Post::Post(QString id, QString title, QString description)
{
    this->id = id;
    this->title = title;
    this->description = description;
}

bool Auth::register_user(QByteArray answer)
{
    QJsonParseError * error = new QJsonParseError;
    QJsonDocument doc = QJsonDocument::fromJson(answer, error);
    QJsonObject json = doc.object();

    if (error->error == QJsonParseError::NoError)
    {
        if (json.value("status").toDouble() == 1)
        {
            QJsonObject::Iterator data = json.find("data");
            QString id = data->toObject().find("id")->toString();
            QString hash = data->toObject().find("hash")->toString();

            QFile file("C:/Users/serega/Documents/blog/auth.txt");
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                qDebug() << "problem with auth file";
                return false;
            }

            QTextStream out(&file);
            out << id << "\n"
                << name << "\n"
                << password << "\n"
                << hash << "\n";
            file.close();

            this->id = id;
            this->hash = hash;

            qDebug() << "User log in: [" << id << ", " << name << ", " << password << ", " << hash << "]";

            return true;
        }
    } else {
        qDebug() << error->errorString();
    }

    return false;
}

bool Auth::login_user(QByteArray answer)
{
    QJsonParseError * error = new QJsonParseError;
    QJsonDocument doc = QJsonDocument::fromJson(answer, error);
    QJsonObject json = doc.object();

    if (error->error == QJsonParseError::NoError)
    {
        if (json.value("status").toDouble() == 1)
        {
            QJsonObject::Iterator data = json.find("data");
            QString id = data->toObject().find("id")->toString();
            QString hash = data->toObject().find("hash")->toString();

            QFile file("C:/Users/serega/Documents/blog/auth.txt");
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                qDebug() << "problem with auth file";
                return false;
            }

            QTextStream out(&file);
            out << id << "\n"
                << name << "\n"
                << password << "\n"
                << hash << "\n";
            file.close();

            this->id = id;
            this->hash = hash;

            qDebug() << "User log in: [" << id << ", " << name << ", " << password << ", " << hash << "]";

            return true;
        }
    } else {
        qDebug() << error->errorString();
    }

    return false;
}

bool Auth::check()
{
    QFile file("C:/Users/serega/Documents/blog/auth.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    if (in.atEnd())
    {
        file.close();
        return false;
    }

    id = in.readLine();
    name = in.readLine();
    password = in.readLine();
    hash = in.readLine();

    file.close();
    return true;
}

void Auth::logout()
{
    id = nullptr;
    name = nullptr;
    password = nullptr;
    hash = nullptr;

    QFile file("C:/Users/serega/Documents/blog/auth.txt");
    file.resize(0);
    file.close();
}

QList<Post> * Posts::parse_posts(QByteArray posts)
{
    QList<Post> * response = new QList<Post>();

    QJsonParseError * error = new QJsonParseError;
    QJsonDocument doc = QJsonDocument::fromJson(posts, error);
    QJsonArray json = doc.array();

    qDebug() << error->errorString();

    for (auto it = json.begin(); it < json.end(); it++)
    {
        QString id = it->toObject().value("id").toString();
        QString title = it->toObject().value("title").toString();
        QString description = it->toObject().value("description").toString();

        Post * post = new Post(id, title, description);
        response->append(*post);
    }

    return response;
}
