# client-server
Client – Server application written in C/C++ using sockets.

# Introduction

Client – Server application written in C/C++ using sockets. Sockets are stream sockets in the UNIX domain.
It uses threads to handle several clients. It can handle 5 clients. Each client communicate to his own thread.
Client send commands to server and the server do it and return results.

**Available commands:**
 * **ls** – list of directory
 * **cd** – change directory
 * **cp** – copy files
 * **mv** – move files
 * **get** – download file from server
 * **put** – send file to server

# How to compile:
* Use Makefile to build both applications. 
* Execute:
 * “./server” in app directory
 * “./client” in client directory



# Commands:

ls - List files and folders in directory. Use:
  * ls to list current directory
  * ls <argument> to list directory the name specified in the argument
  
cd – change working directory. 
Use:
* cd <argument> to go to the directory specified in the argument, i.e. “cd ..” goes to parent directory

cp – copy files and folders. 
Use:
* cp arg1 arg2 where arg1 and ag2 are files paths

mv – copy files and folders. 
Use:
* mv arg1 arg2 where arg1 and ag2 are files paths

get – get files from server. Max file size is 5120 bytes. 
Use:
* get arg1                          where arg1 is file name

put – send file to server. Max file size is 5120 bytes. Use:
* put arg1 where arg1 is file name


# How It works:


LS:
* Client sends a command to server for example: “ls cat2”
* Server opens a directory “cat2” and read all the files in this directory.
* Server send a request to client:
1 byte – type of operation
4 bytes – message size
1 byte – status
X bytes – message


CD:
* Client sends a command to server for example: “cd ./cat2”
* Server does popen() function which returns error stream
 - If error stream returned message > 0, somethings goes wrong with command
* Server sends respond to Client: 1 byte status
 - If status is 1:
4 bytes error length
X bytes error


CP:
* Client sends a command to server for example: “cp ./a ./cat2”
* Server does popen() function which return error stream
 - If error stream returned message > 0, somethings goes wrong with command
* Server sends a respond to Client: 1 byte status
 - If status is 1:
4 bytes error length
X bytes error


MV:
* Client sends a command to server for example: “cp ./a ./cat2”
* Server does popen() function which return error stream
 - If error stream returned message > 0, somethings goes wrong with command
* Server sends a respond to Client: 1 byte status
 - If status is 1:
4 bytes error length
X bytes of error


GET:
* Client sends a command to server for example: “get a”
* Server opens “a” file and read it to file descriptor
* Server counts length of file
* Server sends a respond to Client:
1 byte – type of operation
4 bytes – file size
1 byte – status
 - If status is 0 (which is correct) - x bytes of file


PUT:
* Client sends a command to server for example: “put a”
* Server saves file name and send status_ok to client
* Client open and read from file descriptor
* Client sends to server
4 bytes – file length
1 byte – status
 - If status is 0 – x bytes of file
* Server sends a respond to Client:
1 byte – status
 - If status is 1:
4 bytes error length
X bytes of error
