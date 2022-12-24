#include "chatInfo.h"

#define userConfig "userConfig.txt"

int loadUsersData();
int namePasswordCmp(struct msgbuf message, char str[USERNAME_SIZE + PASSWORD_SIZE + 1]);

void loginUser(struct msgbuf receive, int msgId);