#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0

#define NUM_OF_USERS 9
#define NUM_OF_GROUPS 3

#define MESSAGE_SIZE 1024
#define USERNAME_SIZE 16
#define PASSWORD_SIZE 16
#define GROUPNAME_SIZE 16

#define KEY 2107

#define LOGIN_TYPE 1
#define LOGIN_ERROR_PASSWORD_TYPE 2
#define LOGIN_ERROR_USERNAME_TYPE 3
#define LOGIN_ERROR_USER_LOGGED_TYPE 4
#define LOGIN_CONFIRMATION_TYPE 5
#define LOGOUT_TYPE 6
#define USERS_LIST_TYPE 7
#define GROUP_JOIN_REQUEST_TYPE 8
#define GROUP_JOIN_ERROR_NAME_TYPE 9
#define GROUP_JOIN_ERROR_USER_IN_GROUP_TYPE 10
#define GROUP_JOIN_ERROR_FULL_TYPE 11
#define GROUP_JOIN_CONFIRMATION_TYPE 12
#define GROUP_LIST_TYPE 13

#define LOGIN_ERROR_MESSAGE "Username does not exist\n"
#define LOGIN_ERROR_MESSAGE2 "Incorrect password\n"
#define LOGIN_ERROR_MESSAGE3 "User already logged in\n"
#define LOGIN_CONFIRMATION_MESSAGE "User logged in. Welcome!\n"
#define LOGOUT_CONFIRMATION_MESSAGE "User logged out.\n"
#define GROUP_JOIN_ERROR_MESSAGE1 "Group with that name does not exist\n"
#define GROUP_JOIN_ERROR_MESSAGE2 "You already are member of the group\n"
#define GROUP_JOIN_ERROR_MESSAGE3 "Group is full\n"
#define GROUP_JOIN_CONFIRMATION_MESSAGE "User joined to group\n"


struct msgbuf{
		long type;
		int error;
		int receiverId;
		int senderId;
		char message[1024];
};