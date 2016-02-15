#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <fstream>
#include "client.h"


Client::Client() {
        data_len = 0;
        bzero(receive_buffer, sizeof (receive_buffer));
        bzero(send_buffer, sizeof (send_buffer));
    }


void Client::Connect() {

    struct sockaddr_un* socket_address;
    /*
     * Create the address we will be connecting to.
     */
    socket_address = (struct sockaddr_un*)malloc( sizeof(struct sockaddr_un) );

    socket_address->sun_family = AF_UNIX;
    strcpy(socket_address->sun_path, address);

    /*
     * Get a socket to work with.  This socket will
     * be in the UNIX domain, and will be a
     * stream socket.
     */
    if ((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("client: socket");
        exit(1);
    }
    
    if (connect(server_socket, (struct sockaddr*)socket_address, SUN_LEN(socket_address)) < 0) {
        perror("client: connect");
        exit(1);
    }

    free(socket_address);

}

void Client::Message_Loop() {

    while( printf(">"), fgets(send_buffer, sizeof(send_buffer), stdin), !feof(stdin))
    {
        if( write(server_socket, send_buffer, sizeof(send_buffer)) == -1 )
        {
            perror("Client: Error on send() call \n");
        }
        bzero(send_buffer, sizeof (send_buffer));
        bzero(receive_buffer, sizeof (receive_buffer));

        if( (data_len = read(server_socket, receive_buffer, 1) ) > 0 )
        {
            operation = (operation_type)receive_buffer[0];

            switch( operation ) {
                case LS:
                    LS_Operation();
                    break;
                case CD:
                    CD_Operation();
                    break;
                case CP:
                    CP_Operation();
                    break;
                case MV:
                    MV_Operation();
                    break;
                case GET:
                    GET_Operation();
                    break;
                case PUT:
                    PUT_Operation();
                    break;
            }
        }
        else
        {
            if(data_len < 0)
            {
                perror("Client: Error on recv() call \n");
            }
            else
            {
                std::cout << "Client: Server socket closed " << std::endl;
                close(server_socket);
                break;
            }
        }
    }
    close(server_socket);
}

void Client::LS_Operation() {

    int length;
    char status;

    read(server_socket, &length, 4);
    read(server_socket, &status, 1);

    if(status == LS_STATUS_OK ) {

        Receive_Message(server_socket, receive_buffer, length);

        std::cout << receive_buffer << std::endl;

    }else {
        std::cout << "Error on ls command. Check --help" << std::endl;
    }
}

void Client::CD_Operation() {

    char status;
    read(server_socket, &status, 1);
}

void Client::CP_Operation() {

    int length;
    char status;

    read(server_socket, &length, 4);
    read(server_socket, &status, 1);

    if(status == CP_STATUS_OK) {
        std::cout << "Done." << std::endl;

    }else {
        read(server_socket, receive_buffer, length);
        std::cout << receive_buffer << std::endl;
    }
}

void Client::MV_Operation() {

    int length;
    char status;

    read(server_socket, &length, 4);
    read(server_socket, &status, 1);

    if(status == MV_STATUS_OK) {
        std::cout << "Done." << std::endl;

    }else {
        Receive_Message(server_socket, receive_buffer, length);
        std::cout << receive_buffer << std::endl;
    }
}

void Client::GET_Operation() {

    int file_size;
    char status;
    char filename[255];

    std::ofstream writeFile;
    writeFile.exceptions ( std::ofstream::failbit | std::ofstream::badbit );

    read(server_socket, &file_size, 4);
    read(server_socket, &filename, 255);
    read(server_socket, &status, 1);

    if(status != GET_STATUS_OK) {
        if(status == GET_ERROR_EXIST) {
            std::cout << "Error: File does not exist" << std::endl;
        }

        if(status == GET_ERROR_ACCESS) {
            std::cout << "Error: Access denied" << std::endl;
        }

    }else {
        bzero(receive_buffer, sizeof (receive_buffer));
        Receive_Message(server_socket, receive_buffer, file_size);

        writeFile.open(filename, std::ios::binary);
            
        try {
            if(file_size < MAX_SEND_BUF) {
                writeFile << receive_buffer;
                writeFile.close();
                std::cout << "File downloaded successfuly" << std::endl;
            }
        }
        catch (std::ofstream::failure e) {
            std::cerr << "Exception writing file\n";
        }            
    }
}

void Client::PUT_Operation() {

    int fd;
    int bytes_read = 0;

    char filename[MAX_NAME_LENGTH];
    int file_size = 0;
    char status;

    char part[MAX_SEND_BUF];

    char error_message[MAX_SEND_BUF];
    int error_message_len;

    read(server_socket, &filename, MAX_NAME_LENGTH);

    if ( (fd = open( filename, O_RDONLY ) ) == -1 ) {

        std::cout << "Problem with create a file" << std::endl;

        if(errno == EACCES) {
            status = PUT_ERROR_ACCESS;
        }

        if(errno == ENOENT) {
            status = PUT_ERROR_EXIST;
        }

    }else {

        status = PUT_STATUS_OK;

        // check file size
        file_size = lseek(fd, 0, SEEK_END);
        if (file_size == -1){
            std::cout << "Problem with file seek" << std::endl;
        }

        // get back to file beginning
        lseek(fd, 0, SEEK_SET);
        bzero(part, sizeof (part));

        bytes_read = read(fd, &part, MAX_SEND_BUF);

        write(server_socket, &file_size, 4);
        write(server_socket, &status, 1);

        if(file_size <= MAX_SEND_BUF) {
            write(server_socket, &part, file_size);
            read(server_socket, &status, 1);

            if(status == PUT_STATUS_OK) {
                std::cout << "File has been sended" << std::endl;
            }else {
                read(server_socket, &status, 1);
                read(server_socket, &error_message_len, 4);
                read(server_socket, &error_message, error_message_len);
                std::cout << error_message << std::endl;
            }
        }
    }

}

void Client::Receive_Message(int socket, char* buffer, int message_size) {

    int n, sended_bytes;

    for( n=0; n < message_size; n+= sended_bytes) {
        sended_bytes = read(socket, buffer + n, message_size - n);
    }
}


int main(int argc, char **argv)
{
    if(argc == 2) {
        if( (strcmp(argv[1], "--help") == 0) | (strcmp(argv[1], "-h") == 0) ){

            std::string line;
            std::ifstream help_file("help");

            if (help_file.is_open())
              {
                while ( getline (help_file,line) ) {
                  std::cout << line << std::endl;
                }
                help_file.close();
              }

            else std::cout << "Unable to open file" << std::endl; 

            return 1;

        }else {
            std::cout << "Something goes wrong." << std::endl;
            std::cout << "Try:    ./client --help" << std::endl;
            return 1;
        }
    }

    Client client;
    client.Connect();
    client.Message_Loop();

    return 0;
}

