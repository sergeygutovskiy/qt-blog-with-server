#include "main_window.h"
#include "ui_main_window.h"
#include "QDebug"
#include "custom.cpp"
#include "QtNetwork/QNetworkReply"
#include <QSslConfiguration>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

QString HOST                = "https://qt";

QString REGISTER_URL        = "/users/register";
QString LOGIN_URL           = "/users/login";
QString POSTS_URL           = "/posts";

bool    SLICE_JSON_RESPONSE = true;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{   
    ui->setupUi(this);

    // init https connection to host
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(request_completed(QNetworkReply*)));

    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    // manager->connectToHostEncrypted("qt", 443, conf);

    // init auth
    auth = new Auth();
    if (auth->check())
    {
        qDebug() << "User is auth";
        open_account_page();
    }
    else
    {
        qDebug() << "User is not auth";
        open_home_page();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete auth;
    delete manager;
}

// обработка https ответов
void MainWindow::request_completed(QNetworkReply * reply)
{
    if (current_request == Requests::REGISTER)
    {
        if (reply->error()) {
            qDebug() << reply->errorString();
            return;
        }

        QByteArray answer = reply->readAll();
        if (SLICE_JSON_RESPONSE)
        {
            answer = answer.remove(0, 2);
            answer.chop(2);
            answer.insert(0, '{');
            answer.append('}');
        }

        if (auth->register_user(answer))
        {
            QString old_name = auth->name;
            QString old_password = auth->password;

            ui->register_2->findChild<QLineEdit*>("name_input")->clear();
            ui->register_2->findChild<QLineEdit*>("password_input")->clear();

            auth->name = old_name;
            auth->password = old_password;

            open_account_page();
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Error !!!");
            msgBox.exec();
        }
    }
    else if (current_request == Requests::LOGIN)
    {
        QByteArray answer = reply->readAll();
        if (SLICE_JSON_RESPONSE)
        {
            answer = answer.remove(0, 2);
            answer.chop(2);
            answer.insert(0, '{');
            answer.append('}');
        }

        if (auth->login_user(answer))
        {
            QString old_name = auth->name;
            QString old_password = auth->password;

            ui->login->findChild<QLineEdit*>("name_input_login")->clear();
            ui->login->findChild<QLineEdit*>("password_input_login")->clear();

            auth->name = old_name;
            auth->password = old_password;

            open_account_page();
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Error !!!");
            msgBox.exec();
        }
    }
    else if (current_request == Requests::POSTS)
    {
        QByteArray answer = reply->readAll();
        QList<Post> * response = posts->parse_posts(answer);

        for (auto it = response->begin(); it < response->end(); it++)
        {
            QPushButton * btn = new  QPushButton(it->title);
            btn->setProperty("id", it->id);
            btn->setProperty("title", it->title);
            btn->setProperty("description", it->description);

            connect(btn, SIGNAL(clicked()), this, SLOT(post_clicked()));

            ui->account->findChild<QHBoxLayout *>()
                    ->findChild<QVBoxLayout *>("posts")
                    ->insertWidget(2, btn);
        }
    }
}

// открытие страниц88
void MainWindow::open_home_page() { ui->router->setCurrentIndex(static_cast<int>(Pages::HOME)); }
void MainWindow::open_register_page() { ui->router->setCurrentIndex(static_cast<int>(Pages::REGISTER)); }
void MainWindow::open_account_page()
{
    QLabel* account_name = ui->account->findChild<QLabel*>("account_name");
    account_name->setText(auth->name);

    current_request = Requests::POSTS;

    request.setUrl(QUrl(HOST + POSTS_URL + QString("?user_id=") + QString(auth->id)));
    manager->get(request);

    ui->router->setCurrentIndex(static_cast<int>(Pages::ACCOUNT));
}
void MainWindow::open_login_page() { ui->router->setCurrentIndex(static_cast<int>(Pages::LOGIN)); }

// обработка нажатий на кнопки адресации: входа в акаунт и регистрации
void MainWindow::on_register_link_clicked() { open_register_page(); }
void MainWindow::on_login_link_clicked() { open_login_page(); }

// отправка форм регистрации и входа
void MainWindow::on_register_button_clicked()
{
    current_request = Requests::REGISTER;

    request.setUrl(
        QUrl(HOST + REGISTER_URL + "?name=" + auth->name + "&password=" + auth->password)
    );
    manager->get(request);
}
void MainWindow::on_login_button_clicked()
{
    qDebug() << "login";

    current_request = Requests::LOGIN;

    request.setUrl(
        QUrl(HOST + LOGIN_URL + "?name=" + auth->name + "&password=" + auth->password)
    );
    manager->get(request);
}


void MainWindow::on_name_input_textChanged(const QString &arg1) { auth->name = arg1; }
void MainWindow::on_name_input_login_textChanged(const QString &arg1) { auth->name = arg1; }
void MainWindow::on_password_input_textChanged(const QString &arg1) { auth->password = arg1; }
void MainWindow::on_password_input_login_textChanged(const QString &arg1) { auth->password = arg1; }

// обработка нажатия на кнопку выхода из аккаунта
void MainWindow::on_account_logout_button_clicked()
{
    auth->logout();
    open_home_page();
}

// обработка нажатия на публикацию
void MainWindow::post_clicked()
{
    QObject* senderObj = sender();
    if (senderObj->isWidgetType())
    {
        QPushButton* button = qobject_cast<QPushButton*>(senderObj);
        if (button)
        {
            QString id = button->property("id").toString();
            QString title = button->property("title").toString();
            QString description = button->property("description").toString();

            QMessageBox message;
            message.setWindowTitle("POST");
            message.setText(QString("ID: " + id + "\nTitle: " + title + "\nDescription: " + description));
            message.exec();
        }
    }
}
