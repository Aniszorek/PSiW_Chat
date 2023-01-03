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
int logIn(int msgId, int childPid){

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
		send.receiverId = childPid;

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

    //int ppid = getpid();

	int childPid = fork();

	if(childPid > 0)
	{
		 if(logIn(msgId,childPid)==0)
		 {

			//int id = getpid();

			//int childPid = fork();
			// process doing commands from input
            printf("Ready to chat. Enter help to check commands\n");
            while(TRUE){
                char userInput[MESSAGE_SIZE] = "";

                scanf(" %s", userInput);
                //printf("%s\n", userInput);
                if(!strcmp(userInput,"!help"))
                    printHelp();

                else if(strcmp(userInput, "!logout") == 0)
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

				//sending messages
				else if(!strcmp(userInput,"!dm"))
				{
					printf("Enter username of the reciever:\n");
					scanf(" %s",userInput);
					sendDirectMessage(userInput);
				}
				else if(!strcmp(userInput,"!gm"))
				{
					printf("Enter name of the group:\n");
					scanf(" %s",userInput);
					sendGroupMessage(userInput);
				}
            }
		}
	}
	// process receiving message
    else{
        while(TRUE)
        {
            long status;
			status = msgrcv(msgId, &receive, MESSAGE_SIZE, getpid(), 0);

            if(status>0)
            {
                switch(receive.error)
                {
                    case DIRECT_MESSAGE_RECEIVE_TYPE:
                    {
                        printf("%s\n",receive.message);
                        break;
                    }

                    case GROUP_MESSAGE_RECEIVE_TYPE:
                    {
                        printf("%s\n",receive.message);
                        break;
                    }

                    default:
                    break;
                }
            }
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

void printHelp(){
    printf("\n\n");
    printf("Available commands:\n");
    printf("!help - prints this message\n");
    printf("!ulist - prints list of users online\n");
    printf("!glist - prints available groups\n");
    printf("!guser - prints users in group\n");
    printf("!gjoin - join to the group\n");
    printf("!gexit - exit the group\n");
    printf("!um - messages user\n");
    printf("!gm - messages group\n");
    printf("!logout - logout and exit process\n");
    printf("\n");
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


//send dm to the user with specified nickname
void sendDirectMessage(char username[USERNAME_SIZE])
{
	int id = getpid();
	char mBuf[MESSAGE_SIZE - USERNAME_SIZE - 1]={"\0"};
	printf("Type a message:\n");
	scanf(" %s",mBuf);

	send.type = DIRECT_MESSAGE_TYPE;
	send.senderId = id;

	strcpy(send.message,username);
	strcat(send.message,";");
	strcat(send.message,mBuf);

    //printf("message: %s\n",send.message);

	//querry
	msgsnd(msgId,&send,MESSAGE_SIZE,0);

	//rspnd
	msgrcv(msgId,&receive,MESSAGE_SIZE,id,0);

	if(receive.error == DIRECT_MESSAGE_CONFIRMATION_TYPE)
		printf("Message send to %s\n",username);
	else
		printf("%s\n",receive.message);



}


void sendGroupMessage(char groupname[GROUPNAME_SIZE])
{
	int id = getpid();
	char mBuf[MESSAGE_SIZE - USERNAME_SIZE - 1]={"\0"};
	printf("Type a message:\n");
	scanf(" %s",mBuf);

	send.type = GROUP_MESSAGE_TYPE;
	send.senderId = id;

	strcpy(send.message,groupname);
	strcat(send.message,";");
	strcat(send.message,mBuf);


	//querry
	msgsnd(msgId,&send,MESSAGE_SIZE,0);

	//rspnd
	msgrcv(msgId,&receive,MESSAGE_SIZE,id,0);

	if(receive.error == GROUP_MESSAGE_CONFIRMATION_TYPE)
		printf("Message send to %s\n",groupname);
	else
		printf("%s\n",receive.message);
}
