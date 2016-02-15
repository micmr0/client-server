#define LS_STATUS_OK 0
#define LS_STATUS_BAD 1

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

#define MAX_RECEIVE_BUF     5120
#define MAX_SEND_BUF        5120
#define MAX_NAME_LENGTH     255

static const char* address = "/tmp/server"; // server address to connect

enum operation_type {LS, CD, CP, MV, GET, PUT};

class Client {

    private:
        int server_socket;
        int data_len;
        char receive_buffer[MAX_RECEIVE_BUF];
        char send_buffer[MAX_SEND_BUF];
        operation_type operation;

    public:
        Client();

        void Connect();
        void Message_Loop();
        void LS_Operation();
        void CD_Operation();
        void CP_Operation();
        void MV_Operation();
        void GET_Operation();
        void PUT_Operation();
        void Receive_Message(int, char*, int);
};