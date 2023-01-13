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
#define GROUP_MESSAGE_GROUP_NOT_EXISTS 41
#define GROUP_MESSAGE_GROUP_NOT_EXISTS_MESSAGE "Group with specifed name does not exist!\n"
#define GROUP_MESSAGE_RECEIVE_TYPE 42
#define GROUP_MESSAGE_CONFIRMATION_TYPE 43
#define GROUP_MESSAGE_DELIVERED "server has sent a group message\n"

#define MUTE_USER_TYPE 27
#define MUTE_USER_CONFIRMATION_TYPE 44
#define USER_MUTED_MESSAGE "user has been muted\n"
#define MUTE_USER_NOT_EXIST 45
#define MUTE_USER_NOT_EXIST_MESSAGE "user is not found\n"
#define MUTE_USER_MUTED_ALREADY 46
#define MUTE_USER_MUTED_ALREADY_MESSAGE "user is muted already\n"
#define MUTE_USER_CANT_MUTE_SELF 47
#define MUTE_USER_CANT_MUTE_SELF_MESSAGE "You cannot mute yourself!\n"

#define SHOW_MUTED_TYPE 28
#define SHOW_MUTED_CONFIRMATION_TYPE 48

#define UNMUTE_USER_TYPE 29
#define UNMUTE_USER_CONFIRMATION_TYPE 49
#define USER_UNMUTED_MESSAGE "user has been unmuted\n"
#define UNMUTE_USER_NOT_EXIST 50
#define UNMUTE_USER_NOT_EXIST_MESSAGE "user is not found\n"
#define UNMUTE_USER_CANT_UNMUTE_SELF 51
#define UNMUTE_USER_CANT_UNMUTE_SELF_MESSAGE "You cannot unmute yourself!\n"
#define UNMUTE_USER_NOT_MUTED 52
#define UNMUTE_USER_NOT_MUTED_MESSAGE "This user is not muted!\n"

#define DIRECT_MESSAGE_USER_MUTED 53
#define DIRECT_MESSAGE_USER_MUTED_MESSAGE "You are being muted by this user, sorry!\n"

#define MUTE_GROUP_TYPE 30
#define MUTE_GROUP_CONFIRMATION_TYPE 53
#define GROUP_MUTED_MESSAGE "group has been muted\n"
#define MUTE_GROUP_NOT_EXIST 54
#define MUTE_GROUP_NOT_EXIST_MESSAGE "Group with specifed name does not exist!\n"
#define MUTE_GROUP_MUTED_ALREADY 55
#define MUTE_GROUP_MUTED_ALREADY_MESSAGE "group is muted already\n"

#define UNMUTE_GROUP_TYPE 31
#define UNMUTE_GROUP_CONFIRMATION_TYPE 56
#define GROUP_UNMUTED_MESSAGE "group has been unmuted\n"
#define UNMUTE_GROUP_NOT_EXIST 57
#define UNMUTE_GROUP_NOT_EXIST_MESSAGE "Group with specifed name does not exist!\n"
#define UNMUTE_GROUP_NOT_MUTED 58
#define UNMUTE_GROUP_NOT_MUTED_MESSAGE "This group is not muted!\n"


#define DIRECT_MESSAGE_USER_NOT_LOGGED_IN 59
#define DIRECT_MESSAGE_USER_NOT_LOGGED_IN_MESSAGE "User is not logged in\n"
#define DIRECT_MESSAGE_DM_SELF 60
#define DIRECT_MESSAGE_DM_SELF_MESSAGE "You cannot send a message to yourself!\n"

#define NUMBER_OF_MESSAGE_TYPES 40


struct msgbuf{
		long type;
		int error;
		int receiverId;
		int senderId;
		char message[1024];
};
