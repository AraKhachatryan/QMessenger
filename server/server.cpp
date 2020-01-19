/******************************************************************************
*  @file  server.cpp
*  @brief Multi client and multithreaded server with POSIX sockets for
*         Linux/Unix terminal command line messenger
*
*  @brief main() function is responsible for creating master socket, binding
*         with specified port, listening, accepting incoming connections
*         from clients and creating new socket for communication with this
*         clinet. Used POSIX sockets and threads
*
*  @brief sha256() function for computing sha256 hash (used openssl library)
*
*  @brief send_message_to_client() function designed to send state information 
*         messages to client
*
*  @brief login_register() function is responsible for reading login/register
*         info from client. If recived message contains Sign_In data, login
*         name and password compared in local registered_users.db database.
*         If recived message contains Sign_Up data, full name, login name and
*         password inserts into registered_users.db. Login names in database
*         are unic and password is saved with sha256 hashing algoritm
*
*  @brief client_thread() is core thread function of messenger's server,
*         for each new client connection in main() is created new thread
*
*  @brief client_thread() is responsible for receiving encoded message from
*         client, decoding the message, determining destination client names of
*         this message, private and public state of message and sending back to
*         recipients of this message with appropriate way
*
*  @brief message decoding and encoding is described in encode_decode.cpp and
*         .hpp files
*
*  @brief client_thread() also is responsible for notifying to all clients that
*         this connection is online and notifying to this connection for
*         available online clients. Also notifies to all about leaving the chat
*..............................................................................
*  @version 2.0.0
*  @author Ara Khachatryan
******************************************************************************/

#include <sys/socket.h> // Core BSD socket functions and data structures
#include <sys/fcntl.h>  // for the non-blocking socket
#include <arpa/inet.h>  // for manipulating IP addresses, for inet_addr()
#include <unistd.h>     // for close()
#include <strings.h>    // for bzero()
#include <pthread.h>    // POSIX threads
#include <openssl/sha.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cerrno>       // for errno
#include <cstring>      // for std::strerror()
//#include <filesystem>   // for std::filesystem::exists(), nedded C++17 standart support
#include <regex>
#include <fstream>
#include <sstream>
#include <iomanip>


#include "encode_decode.hpp"
#include "namespace_terminal.hpp"
bool terminal_color = true;


std::map <std::string, int> clients_list;

std::string sha256( const std::string str );
void send_message_to_client( const int client_socket_fd, const char * const message );
bool login_register( const int fd, char * const client_name );
void * client_thread( void * );


int main( int argc, char* argv[] )
{
    const int port_number = 2025;
    const int max_clients = 1000;

    int master_socket_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr[max_clients];
    const int opt = 1;

    int client_socket_fd;
    pthread_t thread[max_clients];

    // create socket
    master_socket_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( master_socket_fd == -1)
    {
        close(master_socket_fd);
        std::cerr << "Cannot open tcp socket" << std::endl;
        return 0;
    }

    // Forcefully attaching socket to the same port again after restart
    if( setsockopt(master_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (const void *)&opt, (socklen_t)sizeof(opt)) == -1 )
    {
        std::cerr << "setsockopt" << std::endl;
        return 0;
    }

    bzero((void *)&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    // bind socket
    if( bind(master_socket_fd, (struct sockaddr *)&server_addr, (socklen_t)sizeof(server_addr)) == -1 )
    {
        std::cerr << "Cannot bind to port " << port_number << std::endl;
        return 0;
    }

    if ( listen(master_socket_fd, max_clients) == -1 )
    {
        std::cerr << "Cannot listen" << std::endl;
        return 0;
    }

    std::cout << "Listening socket_fd " << master_socket_fd << " on port " << port_number << "..." << std::endl;

    for ( int i = 0; i < max_clients; ++i )
    {
        socklen_t client_addr_len = sizeof(client_addr[i]);
        bzero((void *)&client_addr[i], sizeof(client_addr[i]));

        // Accept incoming connections
        client_socket_fd = accept(master_socket_fd, (struct sockaddr *)&client_addr[i], &client_addr_len);
        if ( client_socket_fd == -1 )
        {
            std::cerr << "Cannot accept connection" << std::endl;
        }
        else
        {
            std::cout << "Incoming connection from: " << inet_ntoa(client_addr[i].sin_addr) << " on socket_fd: " << client_socket_fd << std::endl;
        }

        // Change the socket into non-blocking state
        //fcntl(socket_desc, F_SETFL, O_NONBLOCK);

        pthread_create( &thread[i], NULL, client_thread, (void*)&client_socket_fd );
    }

    return 0;

}


std::string sha256( const std::string str )
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::ostringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}


void send_message_to_client( const int client_socket_fd, const char * const message )
{
    char buff[100];
    std::memset(buff, 0x00, sizeof(buff));
    std::strcpy( buff, message );
    send(client_socket_fd, (char*)&buff, sizeof(buff), 0);
}


bool login_register( const int client_socket_fd, char * const client_name )
{
    char buff[100];
    std::memset(buff, 0x00, sizeof(buff));

    int rv = recv(client_socket_fd, buff, sizeof(buff), 0);
    if ( rv == -1 ){
        std::cerr << "Error reading login/register info from client: " << std::strerror(errno) << std::endl;
        return false;
    } else if ( rv  == 0 ) {
        return false;
    }

    // std::regex::ECMAScript for enabling (?:...) atomic non capturing group and ^ $ anchors
    // if flag not specified, default is std::regex::ECMAScript
    const std::regex recv_regex("(Sign_Up|Sign_In);(?:Full_Name:([^;]+);)?Login:([^;]+);Password:([^;]+)", std::regex::ECMAScript);
    const std::regex database_regex("^Full_Name:([^;]+);Login:([^;]+);Password:([^;]+)$", std::regex::ECMAScript);

    // here will be stored matched groups data
    std::smatch recv_matches;
    std::smatch database_matches;

    std::string recv_string(buff);
    // match recv_regex pattern in recv_string
    if ( !std::regex_match(recv_string, recv_matches, recv_regex) )
    {
        std::cerr << "Error not regocnized login/register info from client: " << std::endl;
        return false;
    }
    std::string login_register_request = recv_matches[1].str();
    std::string full_name =  recv_matches[2].str();
    std::string login = recv_matches[3].str();
    std::string password = recv_matches[4].str();

    // Login case /////////////////////////////////////////////////////////////
    if ( std::strcmp(login_register_request.c_str(), "Sign_In") == 0 )
    {
        std::fstream database("registered_users.db", std::ios::in);
        if ( !database.is_open() )
        {
            std::cerr << "Can not open database." << std::endl;
            return false;
        }

        bool login_exist = 0;

        std::string line;
        while (getline(database, line))
        {
            if ( std::regex_match(line, database_matches, database_regex) )
            {
                // if recived Login name found in database
                if ( std::strcmp( login.c_str(), database_matches[2].str().c_str() ) == 0 )
                {
                    login_exist = 1;
                    // compare recived password sha256 hash with database's record
                    if ( std::strcmp( sha256(password).c_str(), database_matches[3].str().c_str() ) == 0 )
                    {
                        send_message_to_client( client_socket_fd, "Login:success" );
                        std::strcpy( client_name, login.c_str() );
                        database.close();
                        return true;
                    // if hashes not equal send Login:wrong_password message to client
                    } else {
                        send_message_to_client( client_socket_fd, "Login:wrong_password" );
                        database.close();
                        return false;
                    }
                }
            }
        }

        // if login not found in database send Login:login_not_exist message to client
        if ( !login_exist )
        {
            send_message_to_client( client_socket_fd, "Login:login_not_exist" );
            database.close();
            return false;
        }
    }
    // END Login case /////////////////////////////////////////////////////////

    // Register case //////////////////////////////////////////////////////////
    if ( std::strcmp(login_register_request.c_str(), "Sign_Up") == 0 )
    {
        std::fstream database_read("registered_users.db", std::ios::in);
        if ( database_read.is_open() )
        {
            std::string line;
            while (getline(database_read, line))
            {
                if ( std::regex_match(line, database_matches, database_regex) )
                {
                    // if Login name found in database_read
                    if ( std::strcmp(database_matches[2].str().c_str(), login.c_str()) == 0 )
                    {
                        send_message_to_client( client_socket_fd, "Register:username_exist" );
                        database_read.close();
                        return false;
                    }
                }
            }
            database_read.close();

        }

        std::fstream database_write("registered_users.db", std::ios::out | std::ios::app);
        if ( database_write.is_open() )
        {
            // if Login name not found in database write new entry
            database_write << "Full_Name:" << full_name << ";Login:" << login << ";Password:" << sha256(password) << std::endl << std::flush;
            database_write.close();
            send_message_to_client( client_socket_fd, "Register:success" );
            std::strcpy( client_name, login.c_str() );
            return true;
        }

    }
    // END Register case //////////////////////////////////////////////////////

    return false;

}


void * client_thread ( void * client_socket_fd )
{
    char client_name[50];
    std::memset(client_name, 0x00, sizeof(client_name));

    char encoded_message[2048];
    std::vector<std::string> dest_names;
    std::string message_info;
    std::string message;
    char output[2048];

    int fd = *(int *)client_socket_fd;


    bool result = false;
    for( int i = 0; i < 10; ++i )
    {
        // login_register() function must set the client_name if succeeded
        result = login_register(fd, client_name);
        if ( result == true )
        {
            break;
        }
    }
    // if after 10 attempts login/register not succeeded, connection and thread closed, bye!
    if ( result == false )
    {
        close(fd);
        pthread_exit(NULL);
        return nullptr;
    }


    // Notify to all clients that this connection is online
    std::string active_clients;
    size_t clinets_count = 0;
    for( const auto& client: clients_list )
    {
        active_clients += client.first;
        ++clinets_count;
        if ( clinets_count < clients_list.size() ) {
            active_clients += ",";
        }

        make_encoded_message(client_name, "Server:online", std::string(" is online"), sizeof(output), output);
        //print_encoded_message(output, sizeof(output));

        send(client.second, output, sizeof(output), 0);
    }

    // Notify to this connection for available online clients
    if ( !clients_list.empty() )
    {
        std::string is_are;
        if( clinets_count > 1 )
        {
            is_are = std::string(" are online");
        } else {
            is_are = std::string(" is online");
        }
        make_encoded_message(active_clients.c_str(), "Server:online", is_are, sizeof(output), output);
        //print_encoded_message(output, sizeof(output));

        send(fd, output, sizeof(output), 0);
    }


    clients_list.insert(std::pair<std::string, int>(std::string(client_name), fd));


    std::cout << "Thread ID: " << pthread_self() << " socket_fd: " << fd << " Name: " << client_name << std::endl;

    while(1)
    {
        std::memset(encoded_message, 0x00, sizeof(encoded_message));
        dest_names.clear();
        message_info.clear();
        message.clear();
        std::memset(output, 0x00, sizeof(output));


        int rv = recv(fd, encoded_message, sizeof(encoded_message), 0);
        if ( rv  == -1 )
        {
            std::cerr << "Error reading from client: " << std::strerror(errno) << std::endl;
            break;
        } else if ( rv  == 0 ) {
            break;
        }

        //print_encoded_message(encoded_message, sizeof(encoded_message));

        decode_message(encoded_message, sizeof(encoded_message), dest_names, message_info, message);


        // Private messages
        if ( !dest_names.empty() )
        {
            // for each destination names in encoded_message do
            for( const auto& d_name: dest_names )
            {
                int dest_socket_desc;

                // find dest_socket_desc
                std::map <std::string, int>::iterator it = clients_list.find(d_name);
                if (it != clients_list.end()) {
                    dest_socket_desc = it->second;
                } else {
                    std::cout << "From \"" << client_name << "\": destination name \"" << d_name << "\" not found" << std::endl;
                    continue;
                }

                std::cout << terminal::TEXT_BOLD << terminal::TEXTCOLOR_RED << client_name << terminal::RESET_ALL
                            << " -> " << terminal::TEXT_BOLD << terminal::TEXTCOLOR_GREEN << d_name << terminal::RESET_ALL
                            << ": " << terminal::TEXT_BOLD << terminal::TEXTCOLOR_CYAN << message << terminal::RESET_ALL << std::endl;

                make_encoded_message(client_name, "Private", message, sizeof(output), output);
                //print_encoded_message(output, sizeof(output));

                // and send message to destination
                send(dest_socket_desc, output, sizeof(output), 0);

            }
        // Public messages
        } else {
            for( const auto& client: clients_list )
            {
                // send message to all except sender
                if ( client.first.compare(client_name) == 0 )
                {
                    continue;
                }
                std::cout << terminal::TEXT_BOLD << terminal::TEXTCOLOR_RED << client_name << terminal::RESET_ALL
                            << " -> " << terminal::TEXT_BOLD << terminal::TEXTCOLOR_GREEN << client.first << terminal::RESET_ALL
                            << ": " << terminal::TEXT_BOLD << terminal::TEXTCOLOR_WHITE << message << terminal::RESET_ALL << std::endl;

                make_encoded_message(client_name, "Public", message, sizeof(output), output);
                //print_encoded_message(output, sizeof(output));

                send(client.second, output, sizeof(output), 0);
            }
        }

    }


    // removing connection information from clients_list
    clients_list.erase(client_name);


    // Notify to all clients that this connection is leaving
    make_encoded_message(client_name, "Server:offline", std::string(" left the chat"), sizeof(output), output);
    //print_encoded_message(output, sizeof(output));
    for( const auto& client: clients_list )
    {
        send(client.second, output, sizeof(output), 0);
    }


    std::cout << "Closing connection with socket_fd: " << fd << ".  Name: " << client_name
                << ".  Exiting the thread." << std::endl;

    close(fd);
    pthread_exit(NULL);

}
