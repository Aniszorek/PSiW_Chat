#include "server.h"


struct User{
    char name[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    int id;
	int sendId;
    int isLogin;
    short incorrectLoginAttempts;
};

struct Group{
    char name[GROUPNAME_SIZE];
    short usersInGroup;
    struct User *users[NUM_OF_USERS];
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
        receiveStatus = msgrcv(msgId, &receive, MESSAGE_SIZE, -NUMBER_OF_MESSAGE_TYPES, 0);
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
				case DIRECT_MESSAGE_TYPE:
					sendDirectMessage();
					break;
				case GROUP_MESSAGE_TYPE:
					sendGroupMessage();
					break;
            }
        }
    }
}

// checks if username and password is correct
int namePasswordCmp(struct msgbuf message, char str[USERNAME_SIZE + PASSWORD_SIZE + 1], int userIndex){
    int tmp = -1;
    int maxNum = USERNAME_SIZE + PASSWORD_SIZE + 1;
    // compare username
    for (int i = 0; i < maxNum; i++ ){
        if(message.message[i] == ';' && str[i] == ';'){
            tmp = i;
            // username correct
            if(users[userIndex].incorrectLoginAttempts >= MAX_LOGIN_ATTEMPS)
                return TOO_MANY_LOGIN_ATTEMPTS;
            break;
        }

        if (message.message[i] != str[i]){
            return USERNAME_INCORRECT;
        }
    }
    // compare password
    for (int j = tmp; j<maxNum; j++){
        if(message.message[j] == '\0' && str[j] == '\0'){
            if (users[userIndex].isLogin)
                return USER_LOGGED_IN;
            else
                return USERNAME_PASSWORD_CORRECT;
        }
        if(message.message[j] != str[j]){
            return PASSWORD_INCORRECT;
        }
    }
    return LOGIN_ERROR;
}

// login user
void logInUser(){
    int userIndex = -1;
    int validation=-2;


    // check if user exist and password is correct
    for (int i = 0; i< NUM_OF_USERS; i++){
        userIndex = i;
        fflush(stdout);
        char userNamePassword[USERNAME_SIZE + PASSWORD_SIZE + 1];

        strcpy(userNamePassword, users[i].name);
        strcat(userNamePassword, ";");
        strcat(userNamePassword, users[i].password);

        // compare username and password
        validation = namePasswordCmp(receive, userNamePassword, i);


        if (validation == USERNAME_PASSWORD_CORRECT){
            users[i].id = receive.senderId;
            users[i].isLogin = TRUE;

			users[i].sendId = receive.receiverId;

            break;
        }
        else if (validation == LOGIN_ERROR) {
            perror("Can't compare username and password");
            exit(7);
        }
        // if username is correct break loop
        else if (validation < -1)
            break;
    }


    // username does not exist
    if(validation == USERNAME_INCORRECT){
        send.error = LOGIN_ERROR_USERNAME_TYPE;
        strcpy(send.message, LOGIN_ERROR_MESSAGE);
        printf("%s\n", LOGIN_ERROR_MESSAGE);
    }
    else if (validation == PASSWORD_INCORRECT) {
        users[userIndex].incorrectLoginAttempts++;
        send.error = LOGIN_ERROR_PASSWORD_TYPE;
        strcpy(send.message, LOGIN_ERROR_MESSAGE2);
        printf("%s (%s)\n\n", LOGIN_ERROR_MESSAGE2, users[userIndex].name);
    }
    else if (validation == USER_LOGGED_IN){
        users[userIndex].incorrectLoginAttempts++;
        send.error = LOGIN_ERROR_USER_LOGGED_TYPE;
        strcpy(send.message, LOGIN_ERROR_MESSAGE3);
        printf("%s (%s)\n\n", LOGIN_ERROR_MESSAGE3, users[userIndex].name);
    }
    else if (validation == TOO_MANY_LOGIN_ATTEMPTS){
        send.error = LOGIN_ERROR_ATTEMPTS_TYPE;
        strcpy(send.message, LOGIN_ERROR_MESSAGE4);
        printf("%s (%s)\n\n", LOGIN_ERROR_MESSAGE4, users[userIndex].name);
    }
    // user logged in
    else{
        send.error = LOGIN_CONFIRMATION_TYPE;
        strcpy(send.message, LOGIN_CONFIRMATION_MESSAGE);
        printf("%s (%s)\n\n", LOGIN_CONFIRMATION_MESSAGE, users[userIndex].name);
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
        users[i].incorrectLoginAttempts = 0;
        printf("%d. %s with password %s loaded\n", i, users[i].name, users[i].password);
    }
    printf("\n");

    // read group name
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
    // if not found group name
    else if((groupIndex=getGroupIndex(receive.message)) < 0){
        send.error = GROUP_JOIN_ERROR_NAME_TYPE;
        strcpy(send.message, GROUP_ERROR_NOT_EXIST_MESSAGE);
        printf("%s(%s want join to %s)\n\n",GROUP_ERROR_NOT_EXIST_MESSAGE,
                            users[userIndex].name,
                            groups[groupIndex].name);
    }
    // if user already in group
    else if((isInGroup = isAlreadyInGroup(groupIndex,userIndex))>=0){
        send.error = GROUP_JOIN_ERROR_USER_IN_GROUP_TYPE;
        strcpy(send.message, GROUP_JOIN_ERROR_MESSAGE2);
        printf("%s(%s want join to %s)\n\n",GROUP_JOIN_ERROR_MESSAGE2,
                            users[userIndex].name,
                            groups[groupIndex].name);
    }
    // if group is full
    else if((emptySlot = findEmptySlot(groupIndex))<0){
        send.error = GROUP_JOIN_ERROR_FULL_TYPE;
        strcpy(send.message, GROUP_JOIN_ERROR_MESSAGE3);
        printf("%s(%s want join to %s)\n\n",GROUP_JOIN_ERROR_MESSAGE3,
                            users[userIndex].name,
                            groups[groupIndex].name);
    }
    // join group
    else{
        groups[groupIndex].users[emptySlot] = &users[userIndex];
        groups[groupIndex].usersInGroup++;
        send.error = GROUP_JOIN_CONFIRMATION_TYPE;
        strcpy(send.message, GROUP_JOIN_CONFIRMATION_MESSAGE);
        printf("%s(%s to %s)\n\n",GROUP_JOIN_CONFIRMATION_MESSAGE,
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

// return:
// index of user in group
// -1 if user not in group
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
        printf("Groups list sended\n\n");
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
        printf("%s(%s want exit from %s)\n\n",GROUP_ERROR_NOT_EXIST_MESSAGE,
                            users[userIndex].name,
                            groups[groupIndex].name);
    }
    // if user does not in the group with that name
    else if((userInGroupIndex = isAlreadyInGroup(groupIndex,userIndex))<0){
        send.error = GROUP_EXIT_ERROR_USER_OUT_GROUP_TYPE;
        strcpy(send.message, GROUP_EXIT_ERROR_MESSAGE2);
        printf("%s(%s want exit from %s)\n\n",GROUP_EXIT_ERROR_MESSAGE2,
                            users[userIndex].name,
                            groups[groupIndex].name);
    }
    // exits group
    else{
        groups[groupIndex].users[userInGroupIndex] = NULL;
        send.error = GROUP_EXIT_CONFIRMATION_TYPE;
        strcpy(send.message, GROUP_EXIT_CONFIRMATION_MESSAGE);
        printf("%s(%s from %s)\n\n",GROUP_EXIT_CONFIRMATION_MESSAGE,
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
        printf("%s(%s want to print users of group)\n\n",
                            GROUP_ERROR_NOT_EXIST_MESSAGE,
                            users[userIndex].name);
    }
    else if(groups[groupIndex].usersInGroup==0){
        send.error =  GROUP_ERROR_EMPTY_TYPE;
        strcpy(send.message, GROUP_ERROR_EMPTY_MESSAGE);
        printf("%s(%s want to print users of %s)\n\n",
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
        printf("Groups list sended\n\n");
    else
        printf("Groups list not sended. Error\n");
}


 void sendDirectMessage()
 {
	 char username[USERNAME_SIZE]= {"\0"};
	 int msgSndStatus=0;
	 int validation = -1;
	 char buf;

     int messageStart=0;

	 //find receiver id by given username
	 for(int i =0;i<=USERNAME_SIZE;i++)
	 {
			buf = receive.message[i];
			if(buf != ';')
			{
				strncat(username,&buf,1);
			}
			else
			{
				validation = checkIfUserExists(username);
                messageStart = i+1;
				break;
			}
	 }


	 //send error response to sender
	 if(validation == USER_NOT_EXISTS)
	 {
			send.type = receive.senderId;
			send.error = DIRECT_MESSAGE_USER_NOT_EXISTS;
			strcpy(send.message,DIRECT_MESSAGE_USER_NOT_EXISTS_MESSAGE);

		msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

		if(msgSndStatus!=-1)
			printf("Error message sent\n\n");
		else
			printf("Error message not sent. Error\n");

		return;
	 }


	 //send a message;
	 send.type = validation;
	 send.error = DIRECT_MESSAGE_RECEIVE_TYPE;

	 //find sender username by given id
	 char senderUsername[USERNAME_SIZE];
	 for(int i = 0;i < NUM_OF_USERS;i++)
	 {
		 if(users[i].id == receive.senderId)
		 {
			 strcpy(senderUsername,users[i].name);
		 }
	 }

	 //char mBuf[MESSAGE_SIZE - USERNAME_SIZE - 1]={"\0"};
	 strcpy(send.message,senderUsername);
	 strcat(send.message,": ");
	 strcat(send.message,&receive.message[messageStart]);


	 msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	 if(msgSndStatus!=-1)
			printf("Direct message delivered\n\n");
		else
			printf("Direct message not delivered. Error\n");

	//send a response to sender
	send.type = receive.senderId;
	send.error = DIRECT_MESSAGE_CONFIRMATION_TYPE;
	strcpy(send.message,DIRECT_MESSAGE_DELIVERED);

	msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	if(msgSndStatus!=-1)
			printf("Confirmation sent\n\n");
	else
			printf("Confirmation not sent. Error\n");



 }

 //returns users id or error if user not found
 int checkIfUserExists(char username[USERNAME_SIZE])
 {
		for(int i = 0; i < NUM_OF_USERS; i++)
		{
			if(strcmp(username,users[i].name)==0)
			{
				return users[i].sendId;
			}

		}

		return USER_NOT_EXISTS;
 }


void sendGroupMessage()
{
	 char groupName[GROUPNAME_SIZE]= {"\0"};
	 int msgSndStatus=0;
	 int validation = GROUP_NOT_EXISTS;
	 char buf;

	 char username[USERNAME_SIZE] = {"\0"};

     int messageStart=0;

	 //find group name
	 for(int i =0;i<=GROUPNAME_SIZE;i++)
	 {
			buf = receive.message[i];
			if(buf != ';')
			{
				strncat(groupName,&buf,1);
			}
			else
			{
                messageStart = i+1;
				break;
			}
	 }

	 printf("%s\n",groupName);


	//find sender name
	strcpy(username,users[getUserIndex(receive.senderId)].name);


     printf("%s\n",username);

	struct Group* g;

	//check if group exists
	for(int i = 0; i < NUM_OF_GROUPS; i++)
	{
		if(strcmp(groups[i].name,groupName) ==0)
		{
			g = &groups[i];
			validation = TRUE;
		}
	}



	 //send error response to sender
	 if(validation == GROUP_NOT_EXISTS)
	 {
			send.type = receive.senderId;
			send.error = GROUP_MESSAGE_GROUP_NOT_EXISTS;
			strcpy(send.message,GROUP_MESSAGE_GROUP_NOT_EXISTS_MESSAGE);

		msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

		if(msgSndStatus!=-1)
			printf("Error message (group not found)  sent\n\n");
		else
			printf("Error message (group not found) not sent. Error\n");

		return;
	 }


	 //send a message;
	 for(int i = 0; i < g->usersInGroup; i++)
	 {
		 send.type = g->users[i]->sendId;
		 send.error = GROUP_MESSAGE_RECEIVE_TYPE;

		 strcpy(send.message,groupName);
		 strcat(send.message,": ");
		 strcat(send.message,username);
		 strcat(send.message,": ");
		 strcat(send.message,&receive.message[messageStart]);

		 msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

		 if(msgSndStatus!=-1)
				printf("Direct message delivered\n\n");
		 else
				printf("Direct message not delivered. Error\n");
	 }



	//send a response to sender
	send.type = receive.senderId;
	send.error = GROUP_MESSAGE_CONFIRMATION_TYPE;
	strcpy(send.message,GROUP_MESSAGE_DELIVERED);

	msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	if(msgSndStatus!=-1)
			printf("Confirmation sent\n\n");
	else
			printf("Confirmation not sent. Error\n");
}
