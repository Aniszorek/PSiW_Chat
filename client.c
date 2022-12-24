#include "client.h"

struct msgbuf send, receive;


int main(int argc, char* argv[]){
    fflush(stdout);
    logIn();
}


void messageSend(int msgId, struct msgbuf message, int flag){
    int msgStatus = msgsnd(msgId, &message, MESSAGE_SIZE, flag);
    if (msgStatus == -1){
        perror("Message send error\n");
    }
}
void messageReceive(int msgId, struct msgbuf message, long msgType, int flag){
    int msgStatus = msgrcv(msgId, &message, MESSAGE_SIZE, msgType, flag);
    if (msgStatus == -1){
        perror("Message send error\n");
    }
    else printf("%s", message.message);
}

// function to log in user 
void logIn(){
    int msgId = msgget(KEY, 0600 | IPC_CREAT);
    char username[USERNAME_SIZE]; 
    char password[PASSWORD_SIZE];
    int pid = getpid();

    while(TRUE){
        printf("Username:");
        scanf("%s", username);
        printf("Password:");
        scanf("%s", password);

        // sending username 
        send.type = LOGIN_MESSAGE_TYPE;
        send.senderId = pid;
        strcat(send.message, username);
        strcat(send.message, ";");
        strcat(send.message, password);
        
        messageSend(msgId, send,0);

        // check for error 
        messageReceive(msgId, receive, pid, 0);
        if(send.error == LOGIN_ERROR_USERNAME_TYPE)
            printf(LOGIN_ERROR_MESSAGE);
        if(send.error == LOGIN_ERROR_PASSWORD_TYPE){
            printf(LOGIN_ERROR_MESSAGE2);
        }
    }

}


void test(){
    int msgId = msgget(KEY, 0600 | IPC_CREAT);
    struct msgbuf receiv;
    struct msgbuf send;
    receiv.type = 1;
    send.type = 1;
        if(fork() == 0){
        while(1){
            msgrcv(msgId, &receiv, 1024, 1, 0);
            if (strcmp(receiv.message, "exit")==0){
                printf("%s\n",receiv.message);
                break;
            }
            printf("%s\n", receiv.message);
        }
    }
    else{
        while(1){
            scanf("%s", send.message);
            if (strcmp(send.message, "exit")==0){
                msgsnd(msgId, &send, 1024, 0);
                break;
            }
            msgsnd(msgId, &send, 1024, 0);
        }
    }    
}