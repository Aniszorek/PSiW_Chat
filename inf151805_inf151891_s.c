#include "inf151805_inf151891_s.h"


struct User{
    char name[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    int id;
	int sendId;
    int isLogin;
    short incorrectLoginAttempts;

    char mutedUsersList[NUM_OF_USERS][USERNAME_SIZE];
    char mutedGroupsList[NUM_OF_GROUPS][GROUPNAME_SIZE];
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
            switch(receive.type)
            {
                case LOGIN_TYPE:
                {
                    logInUser();
                    break;
                }
                case LOGOUT_TYPE:
                {
                    logOutUser();
                    break;
                }
                case USERS_LIST_TYPE:
                {
                    sendUsersList();
                    break;
                }
                case GROUP_JOIN_REQUEST_TYPE:
                {    addUserToGroup();
                    break;
                }
                case GROUP_LIST_TYPE:
                {    sendGroupList();
                     break;
                }
                case GROUP_EXIT_REQUEST_TYPE:
                {    removeUserFromGroup();
                    break;
                }
                case GROUP_USERS_TYPE:
                {    sendGroupUsers();
                    break;
                }
                case DIRECT_MESSAGE_TYPE:
				{	sendDirectMessage();
					break;
                }
                case GROUP_MESSAGE_TYPE:
				{	sendGroupMessage();
                    break;
                }
                case MUTE_USER_TYPE:
                {
                    muteUser();
                    break;
                }
                case UNMUTE_USER_TYPE:
                {
                    unmuteUser();
					break;
                }
                case SHOW_MUTED_TYPE:
                {
                    showMuted();
					break;
                }
                case MUTE_GROUP_TYPE:
                {
                    muteGroup();
                    break;
                }
                case UNMUTE_GROUP_TYPE:
                {
                    unmuteGroup();
                    break;
                }
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

            //init muted users/group lists
            for(int j = 0; j<NUM_OF_USERS;j++)
            {
                strcpy(users[i].mutedUsersList[j],EMPTY_STRING);
            }
            for(int j = 0; j<NUM_OF_GROUPS;j++)
            {
                strcpy(users[i].mutedGroupsList[j],EMPTY_STRING);
            }

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
        for (int j = 0; j < NUM_OF_USERS; j++){
            groups[i].users[j] = NULL;
        }
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
        printf("%s (%s want exit from %s)\n\n",GROUP_ERROR_NOT_EXIST_MESSAGE,
                            users[userIndex].name,
                            groups[groupIndex].name);
    }
    // if user does not in the group with that name
    else if((userInGroupIndex = isAlreadyInGroup(groupIndex,userIndex))<0){
        send.error = GROUP_EXIT_ERROR_USER_OUT_GROUP_TYPE;
        strcpy(send.message, GROUP_EXIT_ERROR_MESSAGE2);
        printf("%s (%s want exit from %s)\n\n",GROUP_EXIT_ERROR_MESSAGE2,
                            users[userIndex].name,
                            groups[groupIndex].name);
    }
    // exits group
    else{
        groups[groupIndex].users[userInGroupIndex] = NULL;
        groups[groupIndex].usersInGroup--;
        send.error = GROUP_EXIT_CONFIRMATION_TYPE;
        strcpy(send.message, GROUP_EXIT_CONFIRMATION_MESSAGE);
        printf("%s (%s from %s)\n\n",GROUP_EXIT_CONFIRMATION_MESSAGE,
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
        printf("%s (%s want to print users of group)\n\n",
                            GROUP_ERROR_NOT_EXIST_MESSAGE,
                            users[userIndex].name);
    }
    else if(groups[groupIndex].usersInGroup==0){
        send.error =  GROUP_ERROR_EMPTY_TYPE;
        strcpy(send.message, GROUP_ERROR_EMPTY_MESSAGE);
        printf("%s (%s want to print users of %s)\n\n",
                            GROUP_ERROR_EMPTY_MESSAGE,
                            users[userIndex].name,
                            groups[groupIndex].name);
    }
    else{
        int usersInGroup = groups[groupIndex].usersInGroup;
        send.error = GROUP_USERS_CONFIRMATION_TYPE;
        strcpy(send.message, "");
        for(int i = 0; i<usersInGroup; i++){
            //printf("%s", groups[groupIndex].users[i]->name);
            strcat(send.message, groups[groupIndex].users[i]->name);
            strcat(send.message, ";");
        }
        printf("%s (%s want to print users of %s)\n\n",
                            GROUP_USERS_CONFIRMATION_MESSAGE,
                            users[userIndex].name,
                            groups[groupIndex].name);
    }

    int msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);
}

 void sendDirectMessage()
 {
	 //username of a receiver
	 char username[USERNAME_SIZE]= {"\0"};
	 int msgSndStatus=0;
	 int validation = -1; //if all ok - stores receicer send id, else stores error id
	 char buf;

     int messageStart=0;


     //find sender username by given id
	 char senderUsername[USERNAME_SIZE];
     strcpy(senderUsername,users[getUserIndex(receive.senderId)].name);

	 printf("(DirectMessage) Request from %s\n",senderUsername);

	 //find receiver sendId by given username
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


	 //check if receiver is logged in
	 if(validation != USER_NOT_EXISTS)
     {
        for(int i = 0; i < NUM_OF_USERS; i++)
        {
            if(strcmp(username,users[i].name)==0 && users[i].isLogin == FALSE)
            {
                validation = USER_NOT_LOGGED_IN;
                break;
            }

        }
     }


     //check if a sender is not muted by the receiver
     if(validation != USER_NOT_EXISTS && validation != USER_NOT_LOGGED_IN)
     {
         //find receiver index
        int receiverIndex;
        for(int i = 0; i < NUM_OF_USERS; i++)
		{
			if(strcmp(username,users[i].name)==0)
			{
				receiverIndex = getUserIndex(users[i].id);
                break;
			}

		}

		//check if muted
        for(int i = 0; i< NUM_OF_USERS; i++)
        {
            if(strcmp(users[receiverIndex].mutedUsersList[i], senderUsername)==0)
            {
                validation = USER_MUTED;
				break;
            }
        }

     }


     //check if a user tries to send a message to self
     if(strcmp(username,senderUsername) == 0)
     {
            validation = DM_SELF_ERROR;
     }


	 //send error response to sender
	 if(validation == USER_NOT_EXISTS)
	 {
		send.type = receive.senderId;
		send.error = DIRECT_MESSAGE_USER_NOT_EXISTS;
		strcpy(send.message,DIRECT_MESSAGE_USER_NOT_EXISTS_MESSAGE);
		msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

		if(msgSndStatus!=-1)
			printf("(DirectMessage) Error message %d sent to %s\n",USER_NOT_EXISTS,senderUsername);
		else
			printf("(DirectMessage) Error message %d not sent to %s. Error\n",USER_NOT_EXISTS,senderUsername);

		return;
	 }
     if(validation == USER_NOT_LOGGED_IN)
	 {
		send.type = receive.senderId;
		send.error = DIRECT_MESSAGE_USER_NOT_LOGGED_IN;
		strcpy(send.message,DIRECT_MESSAGE_USER_NOT_LOGGED_IN_MESSAGE);
		msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

		if(msgSndStatus!=-1)
			printf("(DirectMessage) Error message %d sent to %s\n",USER_NOT_LOGGED_IN,senderUsername);
		else
			printf("(DirectMessage) Error message %d not sent to %s. Error\n",USER_NOT_LOGGED_IN,senderUsername);

		return;
	 }
	 if(validation == USER_MUTED)
     {
         send.type = receive.senderId;
         send.error = DIRECT_MESSAGE_USER_MUTED;
         strcpy(send.message,DIRECT_MESSAGE_USER_MUTED_MESSAGE);
         msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

		if(msgSndStatus!=-1)
			printf("(DirectMessage) Error message %d sent to %s\n",USER_MUTED,senderUsername);
		else
			printf("(DirectMessage) Error message %d not sent to %s. Error\n",USER_MUTED,senderUsername);

		 return;
     }
     if(validation == DM_SELF_ERROR)
     {
         send.type = receive.senderId;
         send.error = DIRECT_MESSAGE_DM_SELF;
         strcpy(send.message,DIRECT_MESSAGE_DM_SELF_MESSAGE);
         msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

		if(msgSndStatus!=-1)
			printf("(DirectMessage) Error message %d sent to %s\n",DM_SELF_ERROR,senderUsername);
		else
			printf("(DirectMessage) Error message %d not sent to %s. Error\n",DM_SELF_ERROR,senderUsername);

		 return;
     }



	 //send a message;
	 send.type = validation;
	 send.error = DIRECT_MESSAGE_RECEIVE_TYPE;

	 strcpy(send.message,senderUsername);
	 strcat(send.message,": ");
	 strcat(send.message,&receive.message[messageStart]);


	 msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	 if(msgSndStatus!=-1)
			printf("(DirectMessage) Delivered from %s to %s\n",senderUsername,username);
		else
			printf("(DirectMessage) Not delivered from %s to %s. Error\n",senderUsername,username);

	//send a response to sender
	send.type = receive.senderId;
	send.error = DIRECT_MESSAGE_CONFIRMATION_TYPE;
	strcpy(send.message,DIRECT_MESSAGE_DELIVERED);

	msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	if(msgSndStatus!=-1)
			printf("(DirectMessage) Confirmation sent to %s\n",senderUsername);
	else
			printf("(DirectMessage) Confirmation not sent to %s. Error\n",senderUsername);



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
	 int validation = GROUP_NOT_EXISTS; //set to TRUE if all ok, else set to error code
	 char buf;

	 //username of sender
	 char username[USERNAME_SIZE] = {"\0"};

     int messageStart=0;

     int groupIsMuted = FALSE;

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

	 //printf("%s\n",groupName);


	//find sender name
	strcpy(username,users[getUserIndex(receive.senderId)].name);

	printf("(GroupMessage) Request from %s\n",username);

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
			printf("(GroupMessage) Error message %d sent to %s\n",GROUP_NOT_EXISTS,username);
		else
			printf("(GroupMessage) Error message %d not sent to %s. Error\n",GROUP_NOT_EXISTS,username);

		return;
	 }


	 //send a message;
	 for(int i = 0; i < g->usersInGroup; i++)
	 {
         groupIsMuted = FALSE;
		 send.type = g->users[i]->sendId;
		 send.error = GROUP_MESSAGE_RECEIVE_TYPE;

         //check if this group is muted by this user
         for(int j=0; j<NUM_OF_GROUPS;j++)
         {
           if(strcmp(g->users[i]->mutedGroupsList[j],groupName) == 0)
           {
                groupIsMuted = TRUE;
				break;
           }
         }

         if(groupIsMuted)
         {
                continue;
         }

		 strcpy(send.message,groupName);
		 strcat(send.message,": ");
		 strcat(send.message,username);
		 strcat(send.message,": ");
		 strcat(send.message,&receive.message[messageStart]);

		 msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

		 if(msgSndStatus!=-1)
				printf("(GroupMessage) (%s) message from %s delivered to group member %s\n",groupName,username,g->users[i]->name);
		 else
				printf("(GroupMessage) (%s) message from %s not delivered to group member %s. Error\n\n",groupName,username,g->users[i]->name);
	 }



	//send a response to sender
	send.type = receive.senderId;
	send.error = GROUP_MESSAGE_CONFIRMATION_TYPE;
	strcpy(send.message,GROUP_MESSAGE_DELIVERED);

	msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	if(msgSndStatus!=-1)
			printf("(GroupMessage) (%s) Confirmation sent to %s\n",groupName,username);
	else
			printf("(GroupMessage) (%s) Confirmation not sent to %s. Error\n",groupName,username);
}

void showMuted()
{
     int userindex = getUserIndex(receive.senderId);
     int msgSndStatus=0;

     char msg[MESSAGE_SIZE];

 	 printf("(ShowMuted) Request from %s\n",users[userindex].name);

     strcpy(msg,EMPTY_STRING);
     strcpy(msg,"Users:\n");

     //add muted users to message
     sortMutedUserames(userindex);
     for(int i = 0; i < NUM_OF_USERS;i++)
     {
        if(strcmp(users[userindex].mutedUsersList[i],EMPTY_STRING) !=0)
        {
            strcat(msg,users[userindex].mutedUsersList[i]);
            strcat(msg,"\n");
        }
     }

     strcat(msg,"Groups:\n");
     //add muted groups to message:
     sortMutedGroupnames(userindex);
     for(int i = 0; i < NUM_OF_GROUPS;i++)
     {
        if(strcmp(users[userindex].mutedGroupsList[i],EMPTY_STRING) !=0)
        {
            strcat(msg,users[userindex].mutedGroupsList[i]);
            strcat(msg,"\n");
        }
     }

     //send response
     send.type = receive.senderId;
	 send.error = SHOW_MUTED_CONFIRMATION_TYPE;
	 strcpy(send.message,msg);

	 msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	 if(msgSndStatus!=-1)
	 		printf("(ShowMuted) Confirmation sent to %s\n",users[userindex].name);
	 else
			printf("(ShowMuted) Confirmation not sent to %s. Error\n",users[userindex].name);


}

void muteUser()
{
	 //username to mute
     char username[USERNAME_SIZE];
     int userindex = getUserIndex(receive.senderId);
     int msgSndStatus=0;
	 int validation = -1; //stores error code


	 printf("(MuteUser) Request from %s\n",users[userindex].name);

     strcpy(username, receive.message);

     //check if user exists
     validation = checkIfUserExists(username);

     //check if user is not muted already
     if(validation != USER_NOT_EXISTS)
     {
        for(int i =0; i<NUM_OF_USERS;i++)
        {
            if(strcmp(users[userindex].mutedUsersList[i], username) == 0)
            {
               validation = USER_MUTED_ALREADY;
            }
        }
     }

     //check for self mute
     if(validation != USER_NOT_EXISTS && validation != USER_MUTED_ALREADY)
     {
            if(strcmp(username, users[userindex].name)== 0)
            {
                validation = MUTE_SELF_ERROR;
            }
     }

	 //send error message
     if(validation == USER_NOT_EXISTS)
     {
            //send error message user not exist
            send.type = receive.senderId;
			send.error = MUTE_USER_NOT_EXIST;
			strcpy(send.message,MUTE_USER_NOT_EXIST_MESSAGE);

            msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(MuteUser) Error message %d sent to %s\n",USER_NOT_EXISTS,users[userindex].name);
            else
                printf("(MuteUser) Error message %d not sent to %s. Error\n",USER_NOT_EXISTS,users[userindex].name);
            return;
     }
     if(validation == USER_MUTED_ALREADY)
     {
         //send error message user muted already
         send.type = receive.senderId;
         send.error = MUTE_USER_MUTED_ALREADY;
         strcpy(send.message,MUTE_USER_MUTED_ALREADY_MESSAGE);

         msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(MuteUser) Error message %d sent to %s\n",USER_MUTED_ALREADY,users[userindex].name);
            else
                printf("(MuteUser) Error message %d not sent to %s. Error\n",USER_MUTED_ALREADY,users[userindex].name);
         return;
     }
     if(validation == MUTE_SELF_ERROR)
     {
         //send error message cant mute self
         send.type = receive.senderId;
         send.error = MUTE_USER_CANT_MUTE_SELF;
         strcpy(send.message,MUTE_USER_CANT_MUTE_SELF_MESSAGE);

         msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(MuteUser) Error message %d sent to %s\n",MUTE_SELF_ERROR,users[userindex].name);
            else
                printf("(MuteUser) Error message %d not sent to %s. Error\n",MUTE_SELF_ERROR,users[userindex].name);
         return;
     }


     //everything ok, mute user
     //find free space in mutedUsersList array
     for(int i =0; i<NUM_OF_USERS;i++)
     {
            if(strcmp(users[userindex].mutedUsersList[i],EMPTY_STRING) == 0)
            {
                strcpy(users[userindex].mutedUsersList[i],username);
                break;
            }
     }




     //send confirmation message
     send.type = receive.senderId;
	 send.error = MUTE_USER_CONFIRMATION_TYPE;
	 strcpy(send.message,USER_MUTED_MESSAGE);

	 msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	 if(msgSndStatus!=-1)
	 		printf("(MuteUser) Confirmation sent to %s\n",users[userindex].name);
	 else
			printf("(MuteUser) Confirmation not sent to %s. Error\n",users[userindex].name);
}


void unmuteUser()
{
	 //username to unmute
     char username[USERNAME_SIZE];
     int userindex = getUserIndex(receive.senderId);
     int msgSndStatus=0;
	 int validation = -1; //stores error code

	 printf("(UnmuteUser) Request from %s\n",users[userindex].name);

     strcpy(username, receive.message);

     //check if user exists
     validation = checkIfUserExists(username);

    //check if user is muted
     if(validation != USER_NOT_EXISTS)
     {
        for(int i =0; i<NUM_OF_USERS;i++)
        {
            if(strcmp(users[userindex].mutedUsersList[i], username) == 0)
            {
               validation = USER_MUTED_ALREADY;
               break;
            }
        }
     }

     //check for self unmute
     if(validation != USER_NOT_EXISTS && validation != USER_MUTED_ALREADY)
     {
            if(strcmp(username, users[userindex].name)== 0)
            {
                validation = MUTE_SELF_ERROR;
            }
     }


     if(validation == USER_NOT_EXISTS)
     {
            //send error message user not exist
            send.type = receive.senderId;
			send.error = UNMUTE_USER_NOT_EXIST;
			strcpy(send.message,UNMUTE_USER_NOT_EXIST_MESSAGE);

            msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(UnmuteUser) Error message %d sent to %s\n",USER_NOT_EXISTS,users[userindex].name);
            else
                printf("(UnmuteUser) Error message %d not sent to %s. Error\n",USER_NOT_EXISTS,users[userindex].name);
            return;
     }

     if(validation == MUTE_SELF_ERROR)
     {
         //send error message cant unmute self
         send.type = receive.senderId;
         send.error = UNMUTE_USER_CANT_UNMUTE_SELF;
         strcpy(send.message,UNMUTE_USER_CANT_UNMUTE_SELF_MESSAGE);

         msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(UnmuteUser) Error message %d sent to %s\n",MUTE_SELF_ERROR,users[userindex].name);
            else
                printf("(UnmuteUser) Error message %d not sent to %s. Error\n",MUTE_SELF_ERROR,users[userindex].name);
         return;
     }

     if(validation != USER_MUTED_ALREADY)
     {
         //send error message user is not muted
         send.type = receive.senderId;
         send.error = UNMUTE_USER_NOT_MUTED;
         strcpy(send.message,UNMUTE_USER_NOT_MUTED_MESSAGE);

         msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(UnmuteUser) Error message %d sent to %s\n",USER_MUTED_ALREADY,users[userindex].name);
            else
                printf("(UnmuteUser) Error message %d not sent to %s. Error\n",USER_MUTED_ALREADY,users[userindex].name);
         return;
     }


    //user is found and is muted - unmute
    for(int i =0; i<NUM_OF_USERS;i++)
     {
            if(strcmp(users[userindex].mutedUsersList[i],username) == 0)
            {
                strcpy(users[userindex].mutedUsersList[i],EMPTY_STRING);
                break;
            }
     }

     //send confirmation message
     send.type = receive.senderId;
	 send.error = UNMUTE_USER_CONFIRMATION_TYPE;
	 strcpy(send.message,USER_UNMUTED_MESSAGE);

	 msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	 if(msgSndStatus!=-1)
	 		printf("(UnmuteUser) Confirmation sent to %s\n",users[userindex].name);
	 else
			printf("(UnmuteUser) Confirmation not sent to %s. Error\n",users[userindex].name);


}

void muteGroup()
{
	 //groupname to be muted
     char groupname[GROUPNAME_SIZE];
     int userindex = getUserIndex(receive.senderId);
     int msgSndStatus=0;
	 int validation = GROUP_NOT_EXISTS; //stores error code


 	 printf("(MuteGroup) Request from %s\n",users[userindex].name);

     strcpy(groupname, receive.message);

     //check if group exists
    for(int i = 0; i < NUM_OF_GROUPS; i++)
	{
		if(strcmp(groups[i].name,groupname) ==0)
		{
			validation = TRUE;
		}
	}

     //check if group is not muted already
     if(validation == TRUE)
     {
        for(int i =0; i<NUM_OF_GROUPS;i++)
        {
            if(strcmp(users[userindex].mutedGroupsList[i], groupname) == 0)
            {
               validation = GROUP_MUTED_ALREADY;
            }
        }
     }

	 //send error message
     if(validation == GROUP_NOT_EXISTS)
     {
            //send error message group not exist
            send.type = receive.senderId;
			send.error = MUTE_GROUP_NOT_EXIST;
			strcpy(send.message,MUTE_GROUP_NOT_EXIST_MESSAGE);

            msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(MuteGroup) Error message %d sent to %s\n",GROUP_NOT_EXISTS,users[userindex].name);
            else
                printf("(MuteGroup) Error message %d not sent to %s. Error\n",GROUP_NOT_EXISTS,users[userindex].name);
            return;
     }
     if(validation == GROUP_MUTED_ALREADY)
     {
         //send error message group muted already
         send.type = receive.senderId;
         send.error = MUTE_GROUP_MUTED_ALREADY;
         strcpy(send.message,MUTE_GROUP_MUTED_ALREADY_MESSAGE);

         msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(MuteGroup) Error message %d sent to %s\n",GROUP_MUTED_ALREADY,users[userindex].name);
            else
                printf("(MuteGroup) Error message %d not sent to %s. Error\n",GROUP_MUTED_ALREADY,users[userindex].name);
         return;
     }


     //everything ok, mute group
     //find free space in mutedGroupsList array
     for(int i =0; i<NUM_OF_GROUPS;i++)
     {
            if(strcmp(users[userindex].mutedGroupsList[i],EMPTY_STRING) == 0)
            {
                strcpy(users[userindex].mutedGroupsList[i],groupname);
                break;
            }
     }




     //send confirmation message
     send.type = receive.senderId;
	 send.error = MUTE_GROUP_CONFIRMATION_TYPE;
	 strcpy(send.message,GROUP_MUTED_MESSAGE);

	 msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	 if(msgSndStatus!=-1)
	 		printf("(MuteGroup) Confirmation sent to %s\n",users[userindex].name);
	 else
			printf("(MuteGroup) Confirmation not sent to %s. Error\n",users[userindex].name);

}

void unmuteGroup()
{
	 //group to be unmuted
     char groupname[USERNAME_SIZE];
     int userindex = getUserIndex(receive.senderId);
     int msgSndStatus=0;
	 int validation = GROUP_NOT_EXISTS; //stores error code

	 printf("(UnmuteGroup) Request from %s\n",users[userindex].name);

     strcpy(groupname, receive.message);

     //check if group exists
    for(int i = 0; i < NUM_OF_GROUPS; i++)
	{
		if(strcmp(groups[i].name,groupname) ==0)
		{
			validation = TRUE;
		}
	}

    //check if group is muted
     if(validation == TRUE)
     {
        for(int i =0; i<NUM_OF_GROUPS;i++)
        {
            if(strcmp(users[userindex].mutedGroupsList[i], groupname) == 0)
            {
               validation = GROUP_MUTED_ALREADY;
            }
        }
     }

	 //send error message
     if(validation == GROUP_NOT_EXISTS)
     {
            //send error message group not exist
            send.type = receive.senderId;
			send.error = UNMUTE_GROUP_NOT_EXIST;
			strcpy(send.message,UNMUTE_GROUP_NOT_EXIST_MESSAGE);

            msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(UnmuteGroup) Error message %d sent to %s\n",GROUP_NOT_EXISTS,users[userindex].name);
            else
                printf("(UnmuteGroup) Error message %d not sent to %s. Error\n",GROUP_NOT_EXISTS,users[userindex].name);
            return;
     }

     if(validation != GROUP_MUTED_ALREADY)
     {
         //send error message group is not muted
         send.type = receive.senderId;
         send.error = UNMUTE_GROUP_NOT_MUTED;
         strcpy(send.message,UNMUTE_GROUP_NOT_MUTED_MESSAGE);

         msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

            if(msgSndStatus!=-1)
                printf("(UnmuteGroup) Error message %d sent to %s\n",GROUP_MUTED_ALREADY,users[userindex].name);
            else
                printf("(UnmuteGroup) Error message %d not sent to %s. Error\n",GROUP_MUTED_ALREADY,users[userindex].name);
         return;
     }


    //group is found and is muted - unmute
    for(int i =0; i<NUM_OF_GROUPS;i++)
     {
            if(strcmp(users[userindex].mutedGroupsList[i],groupname) == 0)
            {
                strcpy(users[userindex].mutedGroupsList[i],EMPTY_STRING);
                break;
            }
     }

     //send confirmation message
     send.type = receive.senderId;
	 send.error = UNMUTE_GROUP_CONFIRMATION_TYPE;
	 strcpy(send.message,GROUP_UNMUTED_MESSAGE);

	 msgSndStatus = msgsnd(msgId, &send, MESSAGE_SIZE, 0);

	 if(msgSndStatus!=-1)
	 		printf("(UnmuteGroup) Confirmation sent to %s\n",users[userindex].name);
	 else
			printf("(UnmuteGroup) Confirmation not sent to %s. Error\n", users[userindex].name);


}

void sortMutedUserames(int userindex)
{
    char tmp[USERNAME_SIZE];
    for(int i=0; i < NUM_OF_USERS; i++)
    {
        for(int j = i+1; j < NUM_OF_USERS; j++)
        {
            if(strcmp(users[userindex].mutedUsersList[i],users[userindex].mutedUsersList[j]) >0)
            {
                strcpy(tmp,users[userindex].mutedUsersList[i]);
                strcpy(users[userindex].mutedUsersList[i],users[userindex].mutedUsersList[j]);
                strcpy(users[userindex].mutedUsersList[j],tmp);
            }
        }
    }
}


void sortMutedGroupnames(int userindex)
{
    char tmp[GROUPNAME_SIZE];
    for(int i=0; i < NUM_OF_GROUPS; i++)
    {
        for(int j = i+1; j < NUM_OF_GROUPS; j++)
        {
            if(strcmp(users[userindex].mutedGroupsList[i],users[userindex].mutedGroupsList[j]) >0)
            {
                strcpy(tmp,users[userindex].mutedGroupsList[i]);
                strcpy(users[userindex].mutedGroupsList[i],users[userindex].mutedGroupsList[j]);
                strcpy(users[userindex].mutedGroupsList[j],tmp);
            }
        }
    }
}


