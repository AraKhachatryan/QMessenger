#ifndef CONNECT_LOGIN_REGISTER_HPP
#define CONNECT_LOGIN_REGISTER_HPP

#include <QTabWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>


class connect_to_server : public QWidget
{
    Q_OBJECT

public:
    explicit connect_to_server(QWidget *parent = 0);

private:
    QLineEdit * ip_addres_LineEdit;
    QLineEdit * port_number_LineEdit;
    QPushButton * connect_PushButton;
    QVBoxLayout * mainLayout;

    QString ip_addres;
    QString port_number;

signals:
    void connect_signal(QString ip_addres, QString port_number);

public slots:
    void on_connect_button_clicked();

};


class sign_in : public QWidget
{
    Q_OBJECT

public:
    explicit sign_in(QWidget *parent = 0);

private:
    QLineEdit * login_LineEdit;
    QLineEdit * password_LineEdit;
    QPushButton * login_PushButton;
    QVBoxLayout * mainLayout;

    QString login;
    QString password;

signals:
    void sign_in_signal(QString login, QString password);

public slots:
    void on_login_button_clicked();

};


class sign_up : public QWidget
{
    Q_OBJECT

public:
    explicit sign_up(QWidget *parent = 0);

private:
    QLineEdit * full_name_LineEdit;
    QLineEdit * login_LineEdit;
    QLineEdit * password_1_LineEdit;
    QLineEdit * password_2_LineEdit;
    QPushButton * register_PushButton;
    QVBoxLayout * mainLayout;

    QString full_name;
    QString login;
    QString password;

signals:
    void sign_up_signal(QString full_name, QString login, QString password);

public slots:
    void on_register_button_clicked();

};


class login_register : public QWidget
{
    Q_OBJECT

public:
    explicit login_register(QWidget *parent = 0);

private:
    QSharedPointer<QTabWidget> tabWidget;
    QSharedPointer<QVBoxLayout> mainLayout;
    QSharedPointer<sign_in> sign_in_QWidget;
    QSharedPointer<sign_up> sign_up_QWidget;

signals:
    void re_sign_in_signal(QString login, QString password);
    void re_sign_up_signal(QString full_name, QString login, QString password);

public slots:
    void re_emit_sign_in_signal(QString login, QString password);
    void re_emit_sign_up_signal(QString full_name, QString login, QString password);

private slots:
    void on_current_changed( int index );

};


#endif // CONNECT_LOGIN_REGISTER_HPP
