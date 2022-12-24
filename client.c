#include "client.h"

struct msgbuf send, receive;


int main(int argc, char* argv[]){
    fflush(stdout);
    logIn();
}


// function to log in user 
void logIn(){
    int msgId = msgget(KEY, 0600 | IPC_CREAT);
    char username[USERNAME_SIZE]; 
    char password[PASSWORD_SIZE];
    int pid = getpid();

    while(TRUE){
        fflush(stdout);
        printf("Username:");
        scanf("%s", username);
        printf("Password:");
        scanf("%s", password);

        // sending username and password
        fflush(stdout);
        send.type = LOGIN_MESSAGE_TYPE;
        send.senderId = pid;

        strcpy(send.message, "");
        strcat(send.message, username);
        strcat(send.message, ";");
        strcat(send.message, password);
        
        int msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);
        if (msgSndStatus == -1){
            perror("Message send error\n");
        }

        // check for error 
        fflush(stdout);
        int msgRcvStatus = msgrcv(msgId, &receive, MESSAGE_SIZE, pid, 0);
        if (msgRcvStatus == -1){
            perror("Message send error\n");
        }

        // print error
        fflush(stdout);
        if(receive.error == LOGIN_ERROR_USERNAME_TYPE){
            printf(LOGIN_ERROR_MESSAGE);
        }
        else if(receive.error == LOGIN_ERROR_PASSWORD_TYPE){
            printf(LOGIN_ERROR_MESSAGE2);
        }
        else if(receive.error == LOGIN_ERROR_USER_LOGGED_TYPE){
            printf(LOGIN_ERROR_MESSAGE3);
        }
        else if (receive.error == LOGIN_CONFIRMATION_TYPE){
            printf(LOGIN_CONFIRMATION_MESSAGE);
            break;
        }
    }

}