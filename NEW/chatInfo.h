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
#include <errno.h>

#define TRUE 1
#define FALSE 0

#define NUM_OF_USERS 9
#define NUM_OF_GROUPS 3

#define MAX_LOGIN_ATTEMPS 3

#define MESSAGE_SIZE 1024
#define USERNAME_SIZE 16
#define PASSWORD_SIZE 16
#define GROUPNAME_SIZE 16


#define KEY 2107

#define LOGIN_TYPE 1
#define LOGIN_ERROR_PASSWORD_TYPE 2
#define LOGIN_ERROR_USERNAME_TYPE 3
#define LOGIN_ERROR_USER_LOGGED_TYPE 4
#define LOGIN_ERROR_ATTEMPTS_TYPE -1
#define LOGIN_CONFIRMATION_TYPE 5
#define LOGOUT_TYPE 6
#define USERS_LIST_TYPE 7
#define GROUP_JOIN_REQUEST_TYPE 8
#define GROUP_JOIN_ERROR_NAME_TYPE 9
#define GROUP_JOIN_ERROR_USER_IN_GROUP_TYPE 10
#define GROUP_JOIN_ERROR_FULL_TYPE 11
#define GROUP_JOIN_CONFIRMATION_TYPE 12
#define GROUP_LIST_TYPE 13
#define GROUP_EXIT_REQUEST_TYPE 14
#define GROUP_EXIT_ERROR_NAME_TYPE 15
#define GROUP_EXIT_ERROR_USER_OUT_GROUP_TYPE 16
#define GROUP_EXIT_CONFIRMATION_TYPE 17
#define GROUP_USERS_TYPE 18
#define GROUP_USERS_ERROR_NAME_TYPE 19
#define GROUP_ERROR_EMPTY_TYPE 20
#define GROUP_USERS_CONFIRMATION_TYPE 21

#define LOGIN_ERROR_MESSAGE "Username does not exist\n"
#define LOGIN_ERROR_MESSAGE2 "Incorrect password\n"
#define LOGIN_ERROR_MESSAGE3 "User already logged in\n"
#define LOGIN_ERROR_MESSAGE4 "Too many login attempts. Please contact the site to clarify the situation\n"
#define LOGIN_CONFIRMATION_MESSAGE "User logged in. Welcome!\n"
#define LOGOUT_CONFIRMATION_MESSAGE "User logged out.\n"
#define GROUP_JOIN_ERROR_MESSAGE2 "You already are member of the group\n"
#define GROUP_JOIN_ERROR_MESSAGE3 "Group is full\n"
#define GROUP_JOIN_CONFIRMATION_MESSAGE "User joined to group\n"
#define GROUP_EXIT_ERROR_MESSAGE2 "You aren't member of the group\n"
#define GROUP_EXIT_CONFIRMATION_MESSAGE "User exited from the group\n"
#define GROUP_ERROR_NOT_EXIST_MESSAGE "Group with that name does not exist\n"
#define GROUP_ERROR_EMPTY_MESSAGE "The group is empty"
#define GROUP_USERS_CONFIRMATION_MESSAGE "List of users in the group sent"

#define DIRECT_MESSAGE_TYPE 22
#define DIRECT_MESSAGE_CONFIRMATION_TYPE 23
#define DIRECT_MESSAGE_RECEIVE_TYPE 24
#define DIRECT_MESSAGE_USER_NOT_EXISTS 25


#define DIRECT_MESSAGE_USER_NOT_EXISTS_MESSAGE "User is not found\n"
#define DIRECT_MESSAGE_DELIVERED "server has sent a message\n"

#define GROUP_MESSAGE_TYPE 26
#define GROUP_MESSAGE_GROUP_NOT_EXISTS 31
#define GROUP_MESSAGE_GROUP_NOT_EXISTS_MESSAGE "Group with specifed name does not exist!\n"
#define GROUP_MESSAGE_RECEIVE_TYPE 32
#define GROUP_MESSAGE_CONFIRMATION_TYPE 33
#define GROUP_MESSAGE_DELIVERED "server has sent a group message\n"

#define NUMBER_OF_MESSAGE_TYPES 30


struct msgbuf{
		long type;
		int error;
		int receiverId;
		int senderId;
		char message[1024];
};
