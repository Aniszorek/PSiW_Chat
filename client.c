#include "client.h"

struct msgbuf send, receive;
int msgId;


int main(int argc, char* argv[]){
    fflush(stdout);
    msgId = msgget(KEY, 0600 | IPC_CREAT);

    if(msgId < 0 ){
        perror("Error getting msgID\n");
        exit(1);
    }

    communicationLoop(msgId);
    
    
}


// function to log in user 
int logIn(){
    
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
        send.type = LOGIN_TYPE;
        send.senderId = pid;

        strcpy(send.message, username);
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
            return 0;
        }
    }
    return 1;

}

void communicationLoop(){
    if(logIn(msgId)==0){
        int childPid = fork();

        // process doing commands from input
        if(childPid > 0){
            printf("Ready to chat.\n");
            while(TRUE){
                char userInput[MESSAGE_SIZE] = "";
                
                scanf(" %s", userInput);
                //printf("%s\n", userInput);

                if(strcmp(userInput, "!logout")==0)
                    logOut(childPid);
                if(strcmp(userInput, "!ulist")==0)
                    requestUsersList();
            }
        }
        // process receiving message
        else{
            //pass
        }
    }
}   
void logOut(int childPid){
    int id = getpid();

    send.type = LOGOUT_TYPE;
    send.senderId = id;

    msgsnd(msgId, &send, MESSAGE_SIZE, 0);
    msgrcv(msgId, &receive, MESSAGE_SIZE, id, 0);

    printf("%s",receive.message);
    kill(childPid, SIGKILL);
    exit(0);
}

void requestUsersList(){
    int id = getpid();
    send.type = USERS_LIST_TYPE;
    send.senderId = id;
    
    msgsnd(msgId, &send, MESSAGE_SIZE, 0);
    msgrcv(msgId, &receive, MESSAGE_SIZE,id,0);

    printUsersList();
}

void printUsersList(){
    int i = 0;
    char c;

    printf("\nLogged in users: \n");
    while((c = receive.message[i]) != '\0'){
        if(c == ';')
            printf("\n");
        else
            printf("%c", c);
        i++;
    }
}