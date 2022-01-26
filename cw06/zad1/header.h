#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define INIT 4
#define CONNECT 5
#define DATA 6
#define MAX_TYPE 7
#define MAX_MSG_SIZE 4000


#ifndef cmb
#define cmb

typedef struct {
    long mtype;
    key_t sender_id;
    char mtext[MAX_MSG_SIZE];
} custom_msgbuf;
#endif

#define msgbuf_size sizeof(custom_msgbuf) - sizeof(long)
