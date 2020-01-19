#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFile>
#include <QMessageBox>
#include <QRegExpValidator>


#include "connect_login_register.hpp"


connect_to_server::connect_to_server(QWidget *parent)
    : QWidget(parent)
{
    ip_addres_LineEdit = new QLineEdit();
    ip_addres_LineEdit->setText("127.0.0.1");
    ip_addres_LineEdit->setAlignment(Qt::AlignCenter);

    port_number_LineEdit = new QLineEdit();
    port_number_LineEdit->setText("2025");
    port_number_LineEdit->setAlignment(Qt::AlignCenter);

    connect_PushButton = new QPushButton("Connect");
    connect_PushButton->setDefault(true);
    connect_PushButton->setAutoDefault(true);
    mainLayout = new QVBoxLayout;

    QString ip_addres;
    QString port_number;

    mainLayout->addWidget(ip_addres_LineEdit);
    mainLayout->addWidget(port_number_LineEdit);
    mainLayout->addWidget(connect_PushButton);

    mainLayout->addStretch(1);
    setLayout(mainLayout);

    // server ip address validation
    QString ip_range = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";

    QRegExp ip_regex ("^" + ip_range   + "\\." + ip_range   + "\\." + ip_range  + "\\." + ip_range + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ip_regex, this);
    ip_addres_LineEdit->setValidator(ipValidator);


    // port number validation
    QString PortRange = "([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])(?::([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]))?";

    QRegExp PortRegex ("^" + PortRange  +"$");
    QRegExpValidator *PortValidator = new QRegExpValidator(PortRegex, this);
    port_number_LineEdit->setValidator(PortValidator);

    QObject::connect(connect_PushButton, &QPushButton::clicked, this, &connect_to_server::on_connect_button_clicked);

    setWindowTitle("Connect");

}


void connect_to_server::on_connect_button_clicked()
{
    bool flag = true;

    QString addressText = ip_addres_LineEdit->text();
    QStringList list = addressText.split(".");

    if( ip_addres_LineEdit->text().isEmpty() )
    {
        ip_addres_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }else if( list.size() != 4 )
    {
        QMessageBox::information(this, "Error", "Invalid Ip Address");
        ip_addres_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }

    if( port_number_LineEdit->text().isEmpty() ){
        port_number_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }else if( port_number_LineEdit->text().toInt()<=1024 ){
         port_number_LineEdit->setStyleSheet("border: 1px solid red;");
         QMessageBox::information(this,"Error","Ports under 1024 are reserved for system services http, ftp, ssl... Valid range is (1025-65535)");
         flag = false;
    }

    if( flag )
    {
        ip_addres = ip_addres_LineEdit->text();
        port_number = port_number_LineEdit->text();

        emit connect_signal(ip_addres, port_number);
    }

}



login_register::login_register(QWidget *parent)
    : QWidget(parent)
{
    tabWidget = QSharedPointer<QTabWidget>::create();
    sign_in_QWidget = QSharedPointer<sign_in>::create();
    sign_up_QWidget = QSharedPointer<sign_up>::create();

    tabWidget->addTab( sign_in_QWidget.data(), "Sign In" );
    tabWidget->addTab( sign_up_QWidget.data(), "Sign Up" );

    mainLayout = QSharedPointer<QVBoxLayout>::create();
    mainLayout->addWidget(tabWidget.data());
    setLayout(mainLayout.data());

    setWindowTitle("QMessenger");


    this->setMinimumSize(300, 260);
    this->setMaximumSize(300, 360);
    this->resize(300, 260);
    QObject::connect(tabWidget.data(), &QTabWidget::currentChanged, this, &login_register::on_current_changed);

    QObject::connect(sign_in_QWidget.data(), &sign_in::sign_in_signal, this, &login_register::re_emit_sign_in_signal);
    QObject::connect(sign_up_QWidget.data(), &sign_up::sign_up_signal, this, &login_register::re_emit_sign_up_signal);


}


void login_register::on_current_changed( int index )
{
    if( index == 0 )
    {
        this->resize(300, 260);
    }

    if( index == 1 )
    {
        this->resize(300, 360);
    }
}


void login_register::re_emit_sign_in_signal(QString login, QString password)
{
    emit re_sign_in_signal(login, password);
}


void login_register::re_emit_sign_up_signal(QString full_name, QString login, QString password)
{
    emit re_sign_up_signal(full_name, login, password);
}


sign_in::sign_in(QWidget *parent)
    : QWidget(parent)
{

    login_LineEdit = new QLineEdit();
    login_LineEdit->setPlaceholderText("Login");
    login_LineEdit->setAlignment(Qt::AlignCenter);

    password_LineEdit = new QLineEdit();
    password_LineEdit->setPlaceholderText("Password");
    password_LineEdit->setEchoMode(QLineEdit::Password);
    password_LineEdit->setAlignment(Qt::AlignCenter);

    login_PushButton = new QPushButton("Login");
    login_PushButton->setDefault(true);
    login_PushButton->setCheckable(true);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(login_LineEdit);
    mainLayout->addWidget(password_LineEdit);
    mainLayout->addWidget(login_PushButton);

    mainLayout->addStretch(1);
    setLayout(mainLayout);

    QObject::connect(login_PushButton, &QPushButton::clicked, this, &sign_in::on_login_button_clicked);

}


void sign_in::on_login_button_clicked()
{
    bool flag = true;

    if(login_LineEdit->text().isEmpty()){
        login_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }


    if(password_LineEdit->text().isEmpty() || password_LineEdit->text().length()<3){
        password_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }

    if( flag ){
        login = login_LineEdit->text();
        password = password_LineEdit->text();

        emit sign_in_signal(login, password);
    }

}



sign_up::sign_up(QWidget *parent)
    : QWidget(parent)
{
    full_name_LineEdit = new QLineEdit();
    full_name_LineEdit->setPlaceholderText("Full Name");
    full_name_LineEdit->setAlignment(Qt::AlignCenter);

    login_LineEdit = new QLineEdit();
    login_LineEdit->setPlaceholderText("Login");
    login_LineEdit->setAlignment(Qt::AlignCenter);

    password_1_LineEdit = new QLineEdit();
    password_1_LineEdit->setPlaceholderText("Password");
    password_1_LineEdit->setEchoMode(QLineEdit::Password);
    password_1_LineEdit->setAlignment(Qt::AlignCenter);

    password_2_LineEdit = new QLineEdit();
    password_2_LineEdit->setPlaceholderText("Confirm Password");
    password_2_LineEdit->setEchoMode(QLineEdit::Password);
    password_2_LineEdit->setAlignment(Qt::AlignCenter);

    register_PushButton = new QPushButton("Register");
    register_PushButton->setDefault(true);
    register_PushButton->setCheckable(true);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(full_name_LineEdit);
    mainLayout->addWidget(login_LineEdit);
    mainLayout->addWidget(password_1_LineEdit);
    mainLayout->addWidget(password_2_LineEdit);
    mainLayout->addWidget(register_PushButton);

    mainLayout->addStretch(1);
    setLayout(mainLayout);


    QObject::connect(register_PushButton, &QPushButton::clicked, this, &sign_up::on_register_button_clicked);

}



void sign_up::on_register_button_clicked()
{
    bool flag = true;

    if(full_name_LineEdit->text().isEmpty()){
        full_name_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }

    if(login_LineEdit->text().isEmpty()){
        login_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }else if(login_LineEdit->text().isEmpty()){
        login_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }


    if(password_1_LineEdit->text().isEmpty()){
        password_1_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }else if(password_1_LineEdit->text().length() < 5){
        QMessageBox::information(this, "Error", "Password length at least must be 5 simbols!!! \n");
        password_1_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }

    if(password_2_LineEdit->text().isEmpty()){
        password_2_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }else if(password_2_LineEdit->text() != password_1_LineEdit->text()){
        QMessageBox::information(this, "Error", "Passwords don't match.");
         password_1_LineEdit->setStyleSheet("border: 1px solid red;");
         password_2_LineEdit->setStyleSheet("border: 1px solid red;");
         flag = false;
    }else if(password_2_LineEdit->text().length()<5){
        password_2_LineEdit->setStyleSheet("border: 1px solid red;");
        flag = false;
    }

    if( flag ){
        full_name = full_name_LineEdit->text();
        login = login_LineEdit->text();
        password = password_1_LineEdit->text();

        emit sign_up_signal(full_name, login, password);
    }

}

