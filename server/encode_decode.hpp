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


void decode_message( const char * encoded_message, size_t io_buffer_length,
        std::vector<std::string>& dest_names, std::string& message_info, std::string&  message );

void make_encoded_message( const char * dest_name, const char * message_info,
        const std::string& message, size_t io_buffer_length, char * output );

void print_encoded_message( const char * encoded_message, size_t io_buffer_length );

