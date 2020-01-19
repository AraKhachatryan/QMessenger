/******************************************************************************
*  @file  encode_decode.cpp
*  @brief functions for encodeing and decoding messages for sending over socket
*
*  @brief first 100 byte are destination names part
*  @brief from 100 to 128 byte are message info part
*  @brief from 128th byte starts message
*  @brief io_buffer_length is package size, at least must be 512, max stable
*         size for single package transmission over tcp socket is 2048
*
*  @brief if package size is over 2048, transmission over tcp socket is not
*         reliable, TCP protocol supports package size (maximum window size)
*         up to 65536, but TCP protocol usually segments the package into 32kb,
*         16kb and to smaller packages. TCP retranssmission may occur,
*         connection can closed and data may lost. Tested in wireshark.
*
*  @brief message structure (dots are NULL or 0x00)
*
*  @brief Tom,Sophia...........................................................
*         ...............................Private.....................Hello!!! W
*         hat's up?............................................................
*         .....................................................................
*         .....................................................................
*         .....................................................................
*         .....................................................................
*         .............................
*
*  @brief Anna.................................................................
*         ...............................Server:online............... is online
*         .....................................................................
*         .....................................................................
*         .....................................................................
*         .....................................................................
*         .....................................................................
*         .............................
*
*  @brief decode_message() decodes from message structure and sets variables
*  @brief make_encoded_message() makes message structure from variables
*
*  @version 2.0.1
*  @author Ara Khachatryan
******************************************************************************/

#include <vector>
#include <string>
#include <iostream>



void make_encoded_message( const char * dest_name, const char * message_info,
                          const std::string& message, size_t io_buffer_length, char * output )
{
    size_t j = 0;

    for( size_t i = 0; dest_name[i] != 0x00; ++i )
    {
        output[j] = dest_name[i];
        ++j;
    }

    for( ; j < 100; )
    {
        output[j] = 0x00;
        ++j;
    }

    for(  size_t i = 0; message_info[i] != 0; ++i  )
    {
        output[j] = message_info[i];
        ++j;
    }

    for( ; j < 128; )
    {
        output[j] = 0x00;
        ++j;
    }

    for(auto& c: message)
    {
        output[j] = c;
        ++j;
    }

        for( ; j < io_buffer_length; )
    {
        output[j] = 0x00;
        ++j;
    }

}


void decode_message( const char * encoded_message, size_t io_buffer_length,
                    std::vector<std::string>& dest_names, std::string& message_info, std::string&  message )
{
    size_t name_char_count = 0;
    size_t name_char_firs_pos = 0;
    bool new_name_flag = 0;


    for( size_t i = 0; i < 100; ++i )
    {
        if ( encoded_message[0] == 0x00 )
        {
            break;
        }

        if ( (encoded_message[i] == ',') || (encoded_message[i] == 0x00) )
        {
            if ( new_name_flag )
            {
                name_char_firs_pos += name_char_count + new_name_flag;
            }
            name_char_count = i - name_char_firs_pos;

            dest_names.push_back(std::string(encoded_message + name_char_firs_pos, name_char_count));

            if ( encoded_message[i] == 0x00 )
            {
                break;
            }

            if ( encoded_message[i] == ',' )
            {
                new_name_flag = 1;
                continue;
            }
        }
    }

    // copy message info part to message_info until reaching 0x00
    name_char_count = 0;
    for( size_t i = 100; i < 128; ++i )
    {
        if ( encoded_message[100] == 0x00 )
        {
            break;
        }

        if ( encoded_message[i] == 0x00 )
        {
            name_char_count = i - 100;

            message_info = std::string(encoded_message + 100, name_char_count);

            if ( encoded_message[i] == 0x00 )
            {
                break;
            }
        }

    }

    // copy message part to message until reaching 0x00
    name_char_count = 0;
    for( size_t i = 128; i < io_buffer_length; ++i )
    {
        if ( encoded_message[128] == 0x00 )
        {
            break;
        }

        if ( encoded_message[i] == 0x00 )
        {
            name_char_count = i - 128;

            message = std::string(encoded_message + 128, name_char_count);

            if ( encoded_message[i] == 0x00 )
            {
                break;
            }
        }

    }

}


// Helper function
void print_encoded_message( const char * encoded_message, size_t io_buffer_length )
{
    std::cout << std::endl;
    for( size_t i = 0; i < io_buffer_length; ++i )
    {
        if(encoded_message[i] == 0x00){
            std::cout << ".";
        }else{
            std::cout << encoded_message[i];
        }
    }
    std::cout << std::endl;
}
