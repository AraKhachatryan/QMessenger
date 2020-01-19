
#include <sstream>  // for std::ostringstream

#include "client.hpp"

#include "encode_decode.hpp"

#include "namespace_terminal.hpp"
bool terminal_color = true;


Client::Client( const std::string ip, const int port )
    : ip_addres(ip), port_number(port)
{
    socket_desc = 0;
    std::memset(&server_addr, 0x00, sizeof(server_addr));

    // Set remote server information
    server_addr.sin_addr.s_addr = inet_addr( ip_addres.c_str() );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( port_number );

    // if internal login_success and register_success signals are thrown recv_thread() is starting immediately
    QObject::connect(this, &Client::login_success, this, &Client::on_login_register_success);
    QObject::connect(this, &Client::register_success, this, &Client::on_login_register_success);
}


Client::~Client()
{
    // forcely make further recv()-s disallowed
    shutdown(socket_desc, SHUT_RD);
    // it will cause in recv_thread() thread to chatch an error at recv(),
    // which breaks the thread cicle, finishes the recv_thread() thread
    // allowing to completly destroy Client object and his child threads

    close(socket_desc);
}


bool Client::create_socket()
{
    socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_desc == -1)
    {
        std::cerr << "Cannot create socket" << std::endl;
        close(socket_desc);
        return false;
    }

    // Change the socket into non-blocking state
	//fcntl(socket_desc, F_SETFL, O_NONBLOCK);

    return true;
}


bool Client::connect_to_server()
{
    if ( ::connect(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1 )
    {
        std::cerr << "Cannot connect to the server " << ip_addres << ":" << port_number << " " << std::strerror(errno) << std::endl;
        close(socket_desc);
        return false;
    }
    return true;
}


bool Client::execute_register( std::string full_name, std::string login, std::string password )
{
    username = login;

    char buff[100];

    std::ostringstream stringStream;
    stringStream << "Sign_Up;Full_Name:" << full_name << ";Login:" << login << ";Password:" << password;
    std::strcpy( buff, stringStream.str().c_str() );

    send(socket_desc, (char*)&buff, sizeof(buff), 0);


    std::memset(buff, 0x00, sizeof(buff));

    int rv = recv(socket_desc, buff, sizeof(buff), 0);
    if ( rv == -1 ){
        std::cerr << "Error reading registration response from server: " << std::strerror(errno) << std::endl;
        return false;
        close(socket_desc);
    } else if ( rv  == 0 ) {
        close(socket_desc);
        return false;
    }

    if ( std::strcmp(buff, "Register:success") == 0 )
    {
        emit register_success();
        return true;
    } else {
        return false;
    }

}


bool Client::execute_login( std::string login, std::string password )
{
    username = login;

    char buff[100];

    std::ostringstream stringStream;
    stringStream << "Sign_In;Login:" << login << ";Password:" << password;
    std::strcpy( buff, stringStream.str().c_str() );

    send(socket_desc, (char*)&buff, sizeof(buff), 0);


    std::memset(buff, 0x00, sizeof(buff));

    int rv = recv(socket_desc, buff, sizeof(buff), 0);
    if ( rv == -1 ){
        std::cerr << "Error reading login response from server: " << std::strerror(errno) << std::endl;
        close(socket_desc);
        return false;
    } else if ( rv  == 0 ) {
        close(socket_desc);
        return false;
    }

    if ( std::strcmp(buff, "Login:success") == 0 )
    {
        emit login_success();
        return true;
    } else {
        return false;
    }

}


void Client::on_login_register_success()
{
    QtConcurrent::run(this, &Client::recv_thread);
}


void Client::send_global_message( std::string message )
{
    char buffer[2048];
    std::memset(buffer, 0x00, sizeof(buffer));

    make_encoded_message("", "Public", message, sizeof(buffer), buffer);
    //print_encoded_message(buffer, sizeof(buffer));

    send(socket_desc,(char*)&buffer, sizeof(buffer), 0);


    // Formatting and printing sent message on linux terminal   ///////////////
    // Public message is send
    std::cout << terminal::TEXT_BOLD << terminal::TEXTCOLOR_WHITE << "Public: "
                << terminal::TEXTCOLOR_YELLOW << message
                << terminal::RESET_ALL << std::endl;
    ///////////////////////////////////////////////////////////////////////////

}


void Client::send_private_message( std::vector<std::string> dest_names, std::string message )
{
    char buffer[2048];
    char dest_names_serialized[100];
    std::memset(buffer, 0x00, sizeof(buffer));
    std::memset(dest_names_serialized, 0x00, sizeof(dest_names_serialized));

    // serializing dest_names and copying into dest_names_serialized[100]
    std::ostringstream stringStream;
    for( auto it = dest_names.begin(); it != dest_names.end(); ++it )
    {
        stringStream << *it;
        if ( *it != dest_names.back() )
        {
            stringStream << ",";
        }
    }
    std::strcpy( dest_names_serialized, stringStream.str().c_str() );


    make_encoded_message( dest_names_serialized, "Private", message, sizeof(buffer), buffer );

    send(socket_desc,(char*)&buffer, sizeof(buffer), 0);


    // Formatting and printing sent message on linux terminal   ///////////////
    // Private message is send
    if ( !dest_names.empty() )
    {
        std::cout << terminal::TEXT_BOLD << terminal::TEXTCOLOR_BLUE;

        for( auto it = dest_names.begin(); it != dest_names.end(); ++it )
        {
            std::cout << *it;
            if ( *it != dest_names.back() )
            {
                std::cout << ", ";
            }
        }

        std::cout << terminal::RESET_ALL << "<- " << terminal::TEXT_BOLD << terminal::TEXTCOLOR_YELLOW
                    << message << terminal::RESET_ALL << std::endl;

    }
    ///////////////////////////////////////////////////////////////////////////

}


void Client::recv_thread()
{
    char input[2048];

	std::vector<std::string> names_from;
	std::string message_info;
	std::string message_from;

	while(1)
    {
        std::memset(input, 0x00, sizeof(input));
        names_from.clear();
        message_info.clear();
        message_from.clear();

        int rv = recv(socket_desc, input, sizeof(input), 0);
		if ( rv == -1 ){
            std::cerr << "Error reading from server: " << std::strerror(errno) << std::endl;
            break;
        } else if ( rv  == 0 ) {
            break;
        }

        decode_message(input, sizeof(input), names_from, message_info, message_from);

        // Private incoming messages
        if( !message_info.compare("Private") )
        {
            std::cout << terminal::TEXT_BOLD << terminal::TEXTCOLOR_RED << names_from.front()
                        << terminal::RESET_ALL <<  "-> " << terminal::TEXT_BOLD << terminal::TEXTCOLOR_GREEN
                        << message_from << terminal::RESET_ALL << std::endl;

            emit private_message_received( QString::fromStdString(names_from.front()), QString::fromStdString(message_from) );

        // Public incoming messages
        } else if ( !message_info.compare("Public") ) {
            std::cout << terminal::TEXT_BOLD << terminal::TEXTCOLOR_WHITE << names_from.front()
                        << terminal::RESET_ALL <<  "-> " << terminal::TEXT_BOLD << terminal::TEXTCOLOR_GREEN
                        << message_from << terminal::RESET_ALL << std::endl;

            emit global_message_received( QString::fromStdString(names_from.front()), QString::fromStdString(message_from) );

        // New online clients notifications from server
        } else if ( !message_info.compare("Server:online") ) {
            std::cout << terminal::TEXT_BOLD << terminal::TEXTCOLOR_CYAN;
            for( auto it = names_from.begin(); it != names_from.end(); ++it )
            {
                std::cout << *it;
                if( *it != names_from.back() )
                {
                    std:: cout << ", ";
                }

                emit online_user( QString::fromStdString(*it) );
            }
            std::cout << terminal::RESET_ALL << terminal::TEXT_BOLD << terminal::TEXTCOLOR_WHITE
                        << message_from << terminal::RESET_ALL << std::endl;

        // Clients gone offline notifications from server
        } else if ( !message_info.compare("Server:offline") ) {
            std::cout << terminal::TEXT_BOLD << terminal::TEXTCOLOR_CYAN;
            for( auto it = names_from.begin(); it != names_from.end(); ++it )
            {
                std::cout << *it;
                if( *it != names_from.back() )
                {
                    std:: cout << ", ";
                }

                emit offline_user( QString::fromStdString(*it) );
            }
            std::cout << terminal::RESET_ALL << terminal::TEXT_BOLD << terminal::TEXTCOLOR_WHITE
                        << message_from << terminal::RESET_ALL << std::endl;
        }


	} // End while()


}

