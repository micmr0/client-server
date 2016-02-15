#define ADDRESS     "/tmp/server"  /* addr to connect */
#define MAX_SEND_BUF		5120
#define MAX_NAME_LENGTH		255

#define LS 0 		//list directory
#define CD 1 		//change directory
#define CP 2 		//copy file
#define MV 3 		//move file
#define GET 4		//get file
#define PUT 5		//put file

#define LS_STATUS_OK 0
#define LS_STATUS_BAD 1

#define CD_STATUS_OK 0
#define CD_STATUS_BAD 1

#define CP_STATUS_OK 0
#define CP_STATUS_BAD 1

#define MV_STATUS_OK 0
#define MV_STATUS_BAD 1

#define GET_STATUS_OK 0
#define GET_STATUS_BAD 1

#define GET_ERROR_ACCESS 3
#define GET_ERROR_EXIST 4

#define PUT_STATUS_OK 0
#define PUT_STATUS_BAD 1

#define PUT_ERROR_ACCESS 3
#define PUT_ERROR_EXIST 4

void *thread_handler(void *);
void send_message();
