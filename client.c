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

    communicationLoop();
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

        // print error or confirmation
        fflush(stdout);
        printf("%s", receive.message);

        // if successfully login, end while
        if (receive.error == LOGIN_CONFIRMATION_TYPE)
            return 0;
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

                if(strcmp(userInput, "!logout") == 0)
                    logOut(childPid);

                else if(strcmp(userInput, "!ulist") == 0)
                    requestUsersList();

                else if(strcmp(userInput, "!gjoin") == 0){
                    printf("Enter name of group: \n");
                    scanf(" %s", userInput);
                    groupJoin(userInput);
                }

                else if(strcmp(userInput, "!glist") == 0)
                    requestGroupsList();

                else if(strcmp(userInput, "!gexit") == 0){
                    printf("Enter name of group: \n");
                    scanf(" %s", userInput);
                    groupExit(userInput);
                }

                else if(strcmp(userInput, "!guser") == 0){
                    printf("Enter name of group: \n");
                    scanf(" %s", userInput);
                    requestGroupUsers(userInput);
                }

            }
        }
        // process receiving message
        else{
            //pass
        }
    }
}  

// function to log out user - exitting program
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

// request for list of all users online
void requestUsersList(){
    int id = getpid();
    send.type = USERS_LIST_TYPE;
    send.senderId = id;
    
    msgsnd(msgId, &send, MESSAGE_SIZE, 0);
    msgrcv(msgId, &receive, MESSAGE_SIZE,id,0);

    printReceivedList("Logged in users: ");
}

// print list received from server in the form of:
// name1;name2;name3
void printReceivedList(char *message){
    int i = 0;
    char c;

    printf("\n%s\n", message);
    while((c = receive.message[i++]) != '\0'){
        if(c == ';')
            printf("\n");
        else
            printf("%c", c);
    }
    printf("\n");
}

// joins user to the group 
void groupJoin(char groupName[GROUPNAME_SIZE]){
    int id = getpid();

    send.type = GROUP_JOIN_REQUEST_TYPE;
    send.senderId = id;
    strcpy(send.message, groupName);

    msgsnd(msgId, &send, MESSAGE_SIZE, 0);

    msgrcv(msgId, &receive, MESSAGE_SIZE, id, 0);
    printf("%s", receive.message);
}

// request for list of all groups
void requestGroupsList(){
    int id = getpid();
    send.type = GROUP_LIST_TYPE;
    send.senderId = id;

    // send request
    msgsnd(msgId, &send, MESSAGE_SIZE, 0);
    // receive list
    msgrcv(msgId, &receive, MESSAGE_SIZE, id, 0);

    printReceivedList("All Groups: ");
}
// removes user from the group
void groupExit(char groupName[MESSAGE_SIZE]){
    int id = getpid();

    send.type = GROUP_EXIT_REQUEST_TYPE;
    send.senderId = id;
    strcpy(send.message, groupName);

    msgsnd(msgId, &send, MESSAGE_SIZE, 0);

    msgrcv(msgId, &receive, MESSAGE_SIZE, id, 0);
    printf("%s", receive.message);
}

// request for list of all users in the group
void requestGroupUsers(char groupName[MESSAGE_SIZE]){
    int id = getpid();
    send.type = GROUP_USERS_TYPE;
    send.senderId = id;
    
    strcpy(send.message, groupName);

    msgsnd(msgId, &send, MESSAGE_SIZE, 0);
    msgrcv(msgId, &receive, MESSAGE_SIZE,id,0);

    if(receive.error == GROUP_USERS_CONFIRMATION_TYPE)
        printReceivedList("Users in the group: ");
    else
        printf("%s\n", receive.message);
}