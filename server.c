#include "server.h"


struct User{
    char name[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    int id;
    int isLogin;
};

struct User users[NUM_OF_USERS];
struct msgbuf send, receive;


int main(){
    long receiveStatus;
    int msgId = msgget(KEY, 0600 | IPC_CREAT);
    printf("a");
    int a = loadUsersData();
    printf("a");
    printf("b");
    fflush(stdout);
    receiveStatus = msgrcv(msgId, &receive, MESSAGE_SIZE, 0, 0);
    printf("c");
    if(receiveStatus > 0){
        switch(receive.type){
            case LOGIN_MESSAGE_TYPE:
                loginUser(receive, msgId);
                break;
        }
    }
}


void loginUser(struct msgbuf receive, int msgId){
    int flag = -1;

    // check if user exist
    for (int i = 0; i< NUM_OF_USERS; i++){
        if (strcmp(receive.message, users[i].name) == 0){
            flag = i;
            users[i].id = receive.senderId;
            users[i].isLogin = TRUE;
            break;
        }
    }
    // if username not exist
    if(flag == - 1){
        send.error = LOGIN_ERROR_TYPE;
        send.type = receive.senderId;
        strcpy(send.message, LOGIN_ERROR_MESSAGE);
        msgsnd(msgId, &send,MESSAGE_SIZE,0);
        printf("%s", LOGIN_ERROR_MESSAGE2);
    }
    // load password

}


int loadUsersData(){
    printf("1");
    FILE *file = fopen("userConfig.txt", "r");
    if (file == NULL)
    {
        perror("Can't open userConfig.txt");
        return 1;
    }
    printf("2");

    for (int i = 0; i < NUM_OF_USERS; i++) {
        fscanf(file, "%s", users[i].name);
        fscanf(file, "%s", users[i].password);
        users[i].id = 0;
        users[i].isLogin = 0;
        printf("%d. %s with password %s loaded\n", i, users[i].name, users[i].password);
    }
    fclose(file);
    return 0;
}


