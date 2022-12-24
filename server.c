#include "server.h"


struct User{
    char name[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    int id;
    int isLogin;
};

struct Group{
    char name[GROUPNAME_SIZE];
    short usersInGroup;
    struct User users[NUM_OF_GROUP];
};

struct User users[NUM_OF_USERS];
struct Group groups[NUM_OF_GROUP];
struct msgbuf send, receive;
int msgId;


int main(){
    fflush(stdout);
    msgId = msgget(KEY, 0600 | IPC_CREAT);

    loadUsersData();

    fflush(stdout);

    servCommunicationLoop(msgId);
    
}


// return:
// 0 if username incorrect
// -1 if password incorrect
// 1 if correct
// -2 if error
int namePasswordCmp(struct msgbuf message, char str[USERNAME_SIZE + PASSWORD_SIZE + 1]){
    int tmp = -1;
    int maxNum = USERNAME_SIZE + PASSWORD_SIZE + 1;
    //compare username
    for (int i = 0; i < maxNum; i++ ){
        if(message.message[i] == ';' && str[i] == ';'){
            tmp = i;
            break;
        }
        if (message.message[i] != str[i]){
            return 0;
        }
    }
    // if ; is not in [i] position then username is not correct
    if(tmp == -1) return 0;
    //compare password
    for (int j = tmp; j<maxNum; j++){
        if(message.message[j] == '\0' && str[j] == '\0'){
            return 1;
        }
        if(message.message[j] != str[j]){
            return -1;
        }
    }
    return -2;
}

void logInUser(){
    int flag = -1;
    int validation=-2;
    

    // check if user exist
    for (int i = 0; i< NUM_OF_USERS; i++){
        fflush(stdout);
        char tempStr[USERNAME_SIZE + PASSWORD_SIZE + 1];

        strcpy(tempStr, users[i].name);
        strcat(tempStr, ";");
        strcat(tempStr, users[i].password);

        validation = namePasswordCmp(receive, tempStr);
        if (validation == 1){
            // if user already logged in
            if(users[i].isLogin){
                validation = -3;
                break;
            }
            else{
                flag = i;
                users[i].id = receive.senderId;
                users[i].isLogin = TRUE;
                break;
            }

        }
        else if (validation == -1) break;
    }

    if(flag == - 1){
        // username does not exist
        if(validation == 0){
            send.error = LOGIN_ERROR_USERNAME_TYPE;
            strcpy(send.message, LOGIN_ERROR_MESSAGE);
            printf("%s\n", LOGIN_ERROR_MESSAGE);
        }
        // password is incorrect
        else if (validation == -1) {
            send.error = LOGIN_ERROR_PASSWORD_TYPE;
            strcpy(send.message, LOGIN_ERROR_MESSAGE2);
            printf("%s\n", LOGIN_ERROR_MESSAGE2);
        }
        // user already logged in
        else{
            send.error = LOGIN_ERROR_USER_LOGGED_TYPE;
            strcpy(send.message, LOGIN_ERROR_MESSAGE3);
            printf("%s\n", LOGIN_ERROR_MESSAGE3);
        }
    }
    // user logged in
    else{
        send.error = LOGIN_CONFIRMATION_TYPE;
        strcpy(send.message, LOGIN_CONFIRMATION_MESSAGE);
        printf("%s (%s)\n", LOGIN_CONFIRMATION_MESSAGE, users[flag].name);
    }
    // send information
    send.type = receive.senderId;
    msgsnd(msgId, &send,MESSAGE_SIZE,0);
}

void loadUsersData(){
    FILE *file = fopen("userConfig.txt", "r");
    if (file == NULL)
    {
        perror("Can't open userConfig.txt");
        exit(3);
    }

    // read username and password
    for (int i = 0; i < NUM_OF_USERS; i++) {
        fscanf(file, "%s", users[i].name);
        fscanf(file, "%s", users[i].password);
        users[i].id = 0;
        users[i].isLogin = FALSE;
        printf("%d. %s with password %s loaded\n", i, users[i].name, users[i].password);
    }
    fclose(file);
}

void logOutUser(){
    int flag = -1;
    // search for user to log out
    for (int i = 0; i< NUM_OF_USERS; i++){
        if (users[i].id == receive.senderId){
            send.type = users[i].id;
            users[i].id = 0;
            users[i].isLogin = FALSE;
            flag = i;
            break;
        }
    }
    if(flag==-1){
        perror("User not found. Logout error");
        exit(2);
    }
    else{
        strcpy(send.message, LOGOUT_CONFIRMATION_MESSAGE);
        printf("%s (%s)\n", LOGOUT_CONFIRMATION_MESSAGE, users[flag].name);
        msgsnd(msgId, &send, MESSAGE_SIZE, 0);
    }   
}

void servCommunicationLoop(){
    long receiveStatus;
    while(TRUE){
        fflush(stdout);
        receiveStatus = msgrcv(msgId, &receive, MESSAGE_SIZE, 0, 0);
        if(receiveStatus > 0){
            switch(receive.type){
                case LOGIN_TYPE:
                    logInUser();
                    break;
                case LOGOUT_TYPE:
                    logOutUser();
                    break;
                case USERS_LIST_TYPE:
                    sendUsersList();
                    break;
            }
        }
    }  
}

void sendUsersList(){
    int validation = 0;
    send.type = receive.senderId;
    strcpy(send.message,"");

    for(int i = 0; i< NUM_OF_USERS; i++){
        if(users[i].isLogin){
            strcat(send.message, users[i].name);
            strcat(send.message, ";");
            validation = 1;
        }
    }
    if(!validation){
        perror("No users detected!");
        exit(4);
    }
    else{
        msgsnd(msgId, &send, MESSAGE_SIZE, 0);
    }
}
