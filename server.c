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
    struct User *users[100];
};

struct User users[NUM_OF_USERS];
struct Group groups[NUM_OF_GROUPS];
struct msgbuf send, receive;
int msgId;


int main(){
    fflush(stdout);
    msgId = msgget(KEY, 0600 | IPC_CREAT);

    loadUsersData();

    fflush(stdout);

    servCommunicationLoop(msgId);
    
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
                case GROUP_JOIN_REQUEST_TYPE:
                    addUserToGroup();
                    break;
                case GROUP_LIST_TYPE:
                    sendGroupList();
                    break;
                case GROUP_EXIT_REQUEST_TYPE:
                    removeUserFromGroup();
                    break;
                case GROUP_USERS_TYPE:
                    sendGroupUsers();
                    break;
            }
        }
    }  
}

// checks if username and password is correct
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

// login user
void logInUser(){
    int flag = -1;
    int validation=-2;
    

    // check if user exist and password is correct
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

// load data from config
// load name of users, password, and name of groups
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
    printf("\n");
    for (int i = 0; i < NUM_OF_GROUPS; i++){
        fscanf(file, "%s", groups[i].name);
        groups[i].usersInGroup = 0;
        printf("%d. %s loaded\n", i, groups[i].name);
    }
    fclose(file);
}

// log out user
void logOutUser(){
    // search for user to log out
    int userIndex = getUserIndex(receive.senderId);

    if(userIndex==-1){
        perror("User not found. Logout error");
        exit(2);
    }
    else{
        send.type = users[userIndex].id;
        users[userIndex].id = 0;
        users[userIndex].isLogin = FALSE;
        strcpy(send.message, LOGOUT_CONFIRMATION_MESSAGE);
        printf("%s (%s)\n", LOGOUT_CONFIRMATION_MESSAGE, users[userIndex].name);
        msgsnd(msgId, &send, MESSAGE_SIZE, 0);
    }   
}

// send to user list of all users online
void sendUsersList(){
    short validation = FALSE;
    send.type = receive.senderId;
    strcpy(send.message,"");

    for(int i = 0; i< NUM_OF_USERS; i++){
        if(users[i].isLogin){
            strcat(send.message, users[i].name);
            strcat(send.message, ";");
            validation = TRUE;
        }
    }
    if(!validation){
        perror("No users detected!");
        exit(4);
    }

    int msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

    if(msgSndStatus!=-1)
        printf("Users list sended\n");
    else
        printf("Users list not sended. Error\n");
}

// add user to the group
void addUserToGroup(){
    int userIndex = getUserIndex(receive.senderId);
    int groupIndex; 
    int emptySlot;
    short isInGroup;

    send.type = receive.senderId;

    if(userIndex<0){
        perror("User not found. joinUserToGroup error");
        exit(6);
    }
    else if((groupIndex=getGroupIndex(receive.message)) < 0){
        send.error = GROUP_JOIN_ERROR_NAME_TYPE;
        strcpy(send.message, GROUP_ERROR_NOT_EXIST_MESSAGE);
        printf("%s(%s want join to %s)\n",GROUP_ERROR_NOT_EXIST_MESSAGE,
                            users[userIndex].name,  
                            groups[groupIndex].name);
    }
    else if((isInGroup = isAlreadyInGroup(groupIndex,userIndex))>=0){
        send.error = GROUP_JOIN_ERROR_USER_IN_GROUP_TYPE;
        strcpy(send.message, GROUP_JOIN_ERROR_MESSAGE2);
        printf("%s(%s want join to %s)\n",GROUP_JOIN_ERROR_MESSAGE2,
                            users[userIndex].name,  
                            groups[groupIndex].name);
    }
    else if((emptySlot = findEmptySlot(groupIndex))<0){
        send.error = GROUP_JOIN_ERROR_FULL_TYPE;
        strcpy(send.message, GROUP_JOIN_ERROR_MESSAGE3);
        printf("%s(%s want join to %s)\n",GROUP_JOIN_ERROR_MESSAGE3,
                            users[userIndex].name,  
                            groups[groupIndex].name);
    }
    // join group
    else{
        groups[groupIndex].users[emptySlot] = &users[userIndex];
        groups[groupIndex].usersInGroup++;
        send.error = GROUP_JOIN_CONFIRMATION_TYPE;
        strcpy(send.message, GROUP_JOIN_CONFIRMATION_MESSAGE);
        printf("%s(%s to %s)\n",GROUP_JOIN_CONFIRMATION_MESSAGE,
                            users[userIndex].name,  
                            groups[groupIndex].name);
    }

    msgsnd(msgId, &send, MESSAGE_SIZE, 0);
}

// return: 
// index of user if everything good
// -1 if can't found id
int getUserIndex(int searchedId){
    for(int i = 0; i<NUM_OF_USERS; i++){
        if(users[i].id == searchedId)
            return i;
    }
    return -1;
}
// return:
// index of group if everything good
// -1 if can't found name
int getGroupIndex(char searchedName[GROUPNAME_SIZE]){
    for (int i = 0; i<NUM_OF_GROUPS; i++){
        if(strcmp(groups[i].name, searchedName) == 0){
            return i;
        }
    }
    return -1;
}

short isAlreadyInGroup(int groupIndex, int userIndex){
    for(int i = 0; i< NUM_OF_USERS;i ++){
        if(groups[groupIndex].users[i] != NULL && 
               strcmp(groups[groupIndex].users[i]->name,users[userIndex].name) == 0)
           return i;
    }
    return -1;
}
// return:
// index of empty slot
// -1 if can't found empty slot
int findEmptySlot(int groupIndex){
    for(int i = 0; i<NUM_OF_USERS; i++){
        if(groups[groupIndex].users[i] == NULL){
            return i;
        }
    }
    return -1;
}

// send to user list of all groups
void sendGroupList(){
    send.type = receive.senderId;
    strcpy(send.message,"");

    for(int i = 0; i<NUM_OF_GROUPS; i++){
        strcat(send.message, groups[i].name);
        strcat(send.message, ";");
    }
    
    int msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

    if(msgSndStatus!=-1)
        printf("Groups list sended\n");
    else
        printf("Groups list not sended. Error\n");
}

// remove user from the group
void removeUserFromGroup(){
    int userIndex = getUserIndex(receive.senderId);
    int groupIndex; 
    int userInGroupIndex;

    send.type = receive.senderId;

    // if not found user with that pid
    if(userIndex<0){
        perror("User not found. joinUserToGroup error");
        exit(6);
    }
    // if not found group with that name
    else if((groupIndex=getGroupIndex(receive.message)) < 0){
        send.error = GROUP_EXIT_ERROR_NAME_TYPE;
        strcpy(send.message, GROUP_ERROR_NOT_EXIST_MESSAGE);
        printf("%s(%s want exit from %s)\n",GROUP_ERROR_NOT_EXIST_MESSAGE,
                            users[userIndex].name,  
                            groups[groupIndex].name);
    }
    // if user does not in the group with that name
    else if((userInGroupIndex = isAlreadyInGroup(groupIndex,userIndex))<0){
        send.error = GROUP_EXIT_ERROR_USER_OUT_GROUP_TYPE;
        strcpy(send.message, GROUP_EXIT_ERROR_MESSAGE2);
        printf("%s(%s want exit from %s)\n",GROUP_EXIT_ERROR_MESSAGE2,
                            users[userIndex].name,  
                            groups[groupIndex].name);
    }
    // exits group
    else{
        groups[groupIndex].users[userInGroupIndex] = NULL;
        send.error = GROUP_EXIT_CONFIRMATION_TYPE;
        strcpy(send.message, GROUP_EXIT_CONFIRMATION_MESSAGE);
        printf("%s(%s from %s)\n",GROUP_EXIT_CONFIRMATION_MESSAGE,
                            users[userIndex].name,  
                            groups[groupIndex].name);
    }

    msgsnd(msgId, &send, MESSAGE_SIZE, 0);
}

// send users of the group
void sendGroupUsers(){
    send.type = receive.senderId;

    int groupIndex = getGroupIndex(receive.message);
    int userIndex = getUserIndex(receive.senderId);

    if(groupIndex < 0){
        send.error =  GROUP_USERS_ERROR_NAME_TYPE;
        strcpy(send.message, GROUP_ERROR_NOT_EXIST_MESSAGE);
        printf("%s(%s want to print users of group)\n",
                            GROUP_ERROR_NOT_EXIST_MESSAGE,
                            users[userIndex].name);
    }
    else if(groups[groupIndex].usersInGroup==0){
        send.error =  GROUP_ERROR_EMPTY_TYPE;
        strcpy(send.message, GROUP_ERROR_EMPTY_MESSAGE);
        printf("%s(%s want to print users of %s)\n",
                            GROUP_ERROR_EMPTY_MESSAGE,
                            users[userIndex].name,  
                            groups[groupIndex].name);
    }
    else{
        int usersInGroup = groups[groupIndex].usersInGroup;
        send.error = GROUP_USERS_CONFIRMATION_TYPE;
        strcpy(send.message, "");
        for(int i = 0; i<usersInGroup; i++){
            strcat(send.message, groups[groupIndex].users[i]->name);
            strcat(send.message, ";");
        }
    }

    int msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

    if(msgSndStatus!=-1)
        printf("Groups list sended\n");
    else
        printf("Groups list not sended. Error\n");    
}