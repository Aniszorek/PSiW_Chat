#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

#define MESSAGE_SIZE 1024
#define USERNAME_SIZE 16
#define PASSWORD_SIZE 16
#define KEY 2102

#define NUM_OF_USERS 9

#define LOGIN_MESSAGE_TYPE 1
#define LOGIN_PASSWORD_TYPE 2
#define LOGIN_ERROR_TYPE 3

#define LOGIN_ERROR_MESSAGE "Username does not exist"
#define LOGIN_ERROR_MESSAGE2 "Incorrect password"

struct msgbuf{
		long type;
		int error;
		int receiverId;
		int senderId;
		char message[1024];
};