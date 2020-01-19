#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/socket.h> // Core BSD socket functions and data structures
#include <sys/fcntl.h>  // for the non-blocking socket
#include <arpa/inet.h>  // for manipulating IP addresses, for inet_addr()
#include <unistd.h>     // for close()

#include <iostream>
#include <vector>
#include <string>
#include <cerrno>       // for errno
#include <cstring>      // for std::memset() and std::strerror()


#include <QtConcurrent>
#include <QObject>
#include <QMetaType>




class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject* parent = nullptr);
    Client( const std::string ip, const int port = 2025 );
    ~Client();

    std::string getUsername() { return username; }

    bool create_socket();
    bool connect_to_server();

    bool execute_register( std::string full_name, std::string login, std::string password );
    bool execute_login( std::string login, std::string password );

public:
    void send_global_message( std::string message );
    void send_private_message( std::vector<std::string> dest_names, std::string message );

private:
    void recv_thread();

private:
    const std::string ip_addres;
    const int port_number;

    std::string username;

    int socket_desc;
    struct sockaddr_in server_addr;

signals:
    void login_success();
    void register_success();

    void online_user( QString new_online_user );
    void offline_user( QString gone_offline_user );

    void global_message_received( QString sender, QString message );
    void private_message_received( QString sender, QString message );

// capturing an internal signals, here recv_thread() is starting immediately
private slots:
    void on_login_register_success();

};

#endif // CLIENT_HPP
