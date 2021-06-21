#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QNetworkAccessManager>
#include "custom.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Auth * auth;
    Posts * posts;

private slots:
    void on_register_link_clicked();

    void on_register_button_clicked();

    void on_name_input_textChanged(const QString &arg1);

    void on_password_input_textChanged(const QString &arg1);

    void request_completed(QNetworkReply * reply);

    void on_account_logout_button_clicked();

    void on_login_button_clicked();

    void on_login_link_clicked();

    void on_name_input_login_textChanged(const QString &arg1);

    void on_password_input_login_textChanged(const QString &arg1);

    void post_clicked();

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager * manager;
    QNetworkRequest request;

    Requests current_request;

    void open_home_page();
    void open_register_page();
    void open_account_page();
    void open_login_page();
};
#endif // MAINWINDOW_H
