#include "chatInfo.h"

#define userConfig "userConfig.txt"


int namePasswordCmp(struct msgbuf message, char str[USERNAME_SIZE + PASSWORD_SIZE + 1]);

void loadUsersData();
void logInUser( int msgId, struct msgbuf receive);
void logOutUser(int msgId,struct msgbuf receive);
void servCommunicationLoop(int msgId);