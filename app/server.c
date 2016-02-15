#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>   
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "server.h"


int main() {

    int fromlen;
    pthread_t pthread;
    int i, server_socket, client_socket, len, pid;
    struct sockaddr_un saun, client;

    // setting to the server directory
    int res = chdir("./server_dir");
    if(res == -1) {
    	perror("Error setting to server directory");
    }

    if ((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("Server: socket");
        exit(1);
    }

    // Create the address we will be binding to.
    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path, ADDRESS);

    // unlink addrees for safety
    unlink(ADDRESS);
    len = sizeof(saun.sun_family) + strlen(saun.sun_path);

    if (bind(server_socket, (struct sockaddr *) &saun, len) < 0) {
        perror("Server: bind");
        exit(1);
    }

    // Listen on the socket
    if (listen(server_socket, 5) < 0) {
        perror("Server: listen");
        exit(1);
    }

    // Accepting connections
    while( (client_socket = accept(server_socket, (struct sockaddr *)&client, &fromlen)) ) {
	  
	    puts("Connection accepted.");

	    // Create new thread for each client
	    if( pthread_create( &pthread, NULL, thread_handler, (void *)&client_socket ) < 0 ) {
	    	perror("Server: pthread_create");
	    	exit(1);
	    }
	}

	if( client_socket < 0 ) {
	        perror("Server: accept");
	        exit(1);
	    }

    return 0;
}

// function for new created thread
void *thread_handler(void *client_soc) {

	int socket  = *(int*)client_soc;

    printf("Waiting for command...\n");

    bool bWaiting = true;

    int data_recv_len = 0;
	char recieve_buffer[MAX_SEND_BUF];
	int *command_size;
	char operation;
	int status;

	// getting current working directory
	char server_directory[MAX_NAME_LENGTH];
    getcwd(server_directory, sizeof(server_directory));

	char directory[MAX_NAME_LENGTH];


		do{
			bzero(recieve_buffer, sizeof (recieve_buffer));
			command_size = (int*)malloc(sizeof(int));
			status=-1;

			data_recv_len = read(socket, recieve_buffer, sizeof (recieve_buffer) );

			if(data_recv_len > 0)
			{

				*command_size = strlen(recieve_buffer);

				bzero(directory, sizeof (directory));

				if( recieve_buffer[0] =='l' && recieve_buffer[1] == 's' ) {
					
					DIR *dir;
					// struct allowed to read a directory
				    struct dirent *dirp;
				    char files[MAX_SEND_BUF];
				    char *file;
				    int message_size = 0;

				    operation = LS;

				    bzero(files, sizeof (files));

				    file = (char *)malloc(MAX_SEND_BUF * sizeof(char));

				    // if command is "ls" user want to list current directory
				    if(*command_size == 3) {

				    	directory[0] = '.';
				    	directory[1] = '\0';

				    }else // list directory from arg
				    {
				    	int len = *command_size - 3;
					    memcpy( directory, &recieve_buffer[3], len-1 );
						directory[len-1] = '\0';
				    }
				    
				    // reading directory and save results in files array
				    int i = 0;
				    int isdir=0;
				    if ( (dir = opendir(directory)) == NULL ) {

				        printf("Error with open a directory \n");

				    } else {

				    	while( (dirp = readdir(dir)) != NULL ) {

				    		strcpy(file, dirp->d_name);

				    		if(file[0] != '.') {
				    			strcat(files, file);
					    	 	strcat(files, "\t");
				    		}	
					    }

					    closedir(dir);

					    message_size = strlen(files);

					    // if len of files is greater than 0 ls done well
					    if(message_size > 0) {
					    	status = LS_STATUS_OK;
					    }else {
					    	status = LS_STATUS_BAD;
					    }

					    printf("LS operation \n");

					    // sending message to client
					    write(socket, &operation, 1);
					    write(socket, &message_size, 4);
					    write(socket, &status, 1);
					    send_message(socket, files, message_size);

					    free(dirp);
					    free(file);
				    }
				}

				if( recieve_buffer[0] =='c' && recieve_buffer[1] == 'd'  ) {

					operation = CD;

					int len = *command_size-3;

				    memcpy( directory, &recieve_buffer[3], len-1 );
					
					directory[len-1] = '\0';

					char cwd[1024];

				    getcwd(cwd, sizeof(cwd));

				  	// statements blocking getting away from server directory (cd ..)
			    	if( strcmp(directory, "..") == 0 ) {

			    		if( strcmp (cwd, server_directory) != 0) {
			    			status = chdir(directory);
			    		}

			    	}else{

			    		status = chdir(directory);
			    	}

				    if(status  != CD_STATUS_OK) {
				    	status = CD_STATUS_BAD;
				    }

				    write(socket, &operation, 1);
				    write(socket, &status, 1);

				}

				if( recieve_buffer[0] =='c' && recieve_buffer[1] == 'p' ) {

					operation = CP;

					int len = *command_size;

					char args[MAX_SEND_BUF];
					char *arg_pointers[4];
					char *argr = " -r 2>&1";

					memcpy( args, &recieve_buffer[0], len);

					// sending error stream to input stream
					args[len - 1] = ' ';
					args[len] = '-';
					args[len + 1] = 'r';
					args[len + 2] = ' ';
					args[len + 3] = '2';
					args[len + 4] = '>';
					args[len + 5] = '&';
					args[len + 6] = '1';
					args[len + 7] = '\0';

					char child_message[100];

					// open a process, do a command and return results
					FILE* file = popen(args, "r");
					
					fgets(child_message, 100, file);

					printf("%s \n", child_message);

					int message_size = strlen(child_message);

					pclose(file);

					// if messege_size == 0 there is no error
					if(message_size == 0 ) {

						//send data to client
						status = CP_STATUS_OK;
						write(socket, &operation, 1);
						write(socket, &message_size, 4);
						write(socket, &status, 1);

					}else {

						status = CP_STATUS_BAD;
						write(socket, &operation, 1);
						write(socket, &message_size, 4);
					    write(socket, &status, 1);
					    send_message(socket, child_message, message_size);
					}
					bzero(child_message, sizeof (child_message));
				}


				if( recieve_buffer[0] =='m' && recieve_buffer[1] == 'v' ) {

					operation = MV;

					int len = *command_size;

					char args[MAX_SEND_BUF];
					char *arg_pointers[4];

					memcpy( args, &recieve_buffer[0], len);

					args[len - 1] = ' ';
					args[len] = '2';
					args[len + 1] = '>';
					args[len + 2] = '&';
					args[len + 3] = '1';
					args[len + 4] = '\0';

					char child_message[100];

					FILE* file = popen(args, "r");
					
					fgets(child_message, 100, file);

					printf("%s \n", child_message);

					int message_size = strlen(child_message);

					pclose(file);

					if(message_size == 0 ) {

						status = MV_STATUS_OK;
						write(socket, &operation, 1);
						write(socket, &message_size, 4);
						write(socket, &status, 1);

					}else {

						status = MV_STATUS_BAD;
						write(socket, &operation, 1);
						write(socket, &message_size, 4);
					    write(socket, &status, 1);
					    send_message(socket, child_message, message_size);

					}
					bzero(child_message, sizeof (child_message));
				}

				if( recieve_buffer[0] =='g' && recieve_buffer[1] == 'e' && recieve_buffer[2] == 't' ) {

					operation = GET;

					int len = *command_size;
					int fd;
					int file_size=0;
					int bytes_written=0;
					int bytes_read = 0;
					char filename[MAX_NAME_LENGTH];
					char part[MAX_SEND_BUF];

					memcpy( filename, &recieve_buffer[4], len - 4);

					filename[len- 5] = '\0';

					if ( (fd = open( filename, O_RDONLY ) ) == -1 ) {

						printf("Problem with open a file \n");

						if(errno == EACCES) {
							status = GET_ERROR_ACCESS;
						}

						if(errno == ENOENT) {
							status = GET_ERROR_EXIST;
						}

						// write data to client
						write(socket, &operation, 1);
						write(socket, &file_size, 4);
						write(socket, &filename, MAX_NAME_LENGTH);
						write(socket, &status, 1);

					}else {

						status = GET_STATUS_OK;

						// get size of a file
						file_size = lseek(fd, 0, SEEK_END);
						if (file_size == -1){
							printf("Problem with file seek \n");
						}

						// get back to file beginning
						lseek(fd, 0, SEEK_SET);

						bzero(part, sizeof (part));

						bytes_read = read(fd, &part, MAX_SEND_BUF);

						write(socket, &operation, 1);
						write(socket, &file_size, 4);
						write(socket, &filename, MAX_NAME_LENGTH);
						write(socket, &status, 1);

						if(file_size <= MAX_SEND_BUF) {
							write(socket, &part, file_size);
							printf("File sended \n");
						}
					}
				}

				if( recieve_buffer[0] =='p' && recieve_buffer[1] == 'u' && recieve_buffer[2] == 't' ) {

				    char size[2];
				    char status;
				    char filename[MAX_NAME_LENGTH];
				    int file_length;

				    int len = *command_size;

				    char part[MAX_SEND_BUF];

				    operation = PUT;

				    memcpy( filename, &recieve_buffer[4], len - 4);

				    filename[len- 5] = '\0';

				    write(socket, &operation, 1);
				    write(socket, &filename, MAX_NAME_LENGTH);

				    read(socket, &file_length, 4);
				    read(socket, &status, 1);

				    if(status == PUT_STATUS_BAD) {
				        printf("Error. \n");

				    }else {
				        bzero(part, sizeof (part));
				        read(socket, &part, file_length);

				        int fd;

				        if( (fd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1 ) {
				        	char *error;
				        	error = strerror(errno);
				        	int error_length = strlen( error );
				            printf("errno: %s \n", error);
				            status = PUT_STATUS_BAD;
				            write(socket, &status, 1);
				            write(socket, &error_length, 4);
				            write(socket, &error, MAX_SEND_BUF);
				        }else {

				            if(file_length < MAX_SEND_BUF) {
				                write(fd, part, file_length);
				                close(fd);
				                printf("File was sended \n");
				                status = PUT_STATUS_OK;
				                write(socket, &status, 1);
				            }
				        }
				    }
				}
				
				if(strstr(recieve_buffer, "quit")!=0)
				{
					printf("Exit command received -> quitting \n");
					bWaiting = false;
					break;
				}
			}
			else
			{
				printf("Error on recv() call \n");
			}
		}while(data_recv_len > 0);

    close(socket);
    free(command_size);

    return 0;
}

void send_message(int socket, char* buffer, int message_size) {

	int n, sended_bytes;

	for( n=0; n < message_size; n+= sended_bytes) {
		sended_bytes = write(socket, buffer + n, message_size - n) ;
		printf("sended bytes: %d \n", sended_bytes);
	}

}
