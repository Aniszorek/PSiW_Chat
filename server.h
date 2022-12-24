#include "chatInfo.h"

#define userConfig "userConfig.txt"


int namePasswordCmp(struct msgbuf message, char str[USERNAME_SIZE + PASSWORD_SIZE + 1]);

void loadUsersData();
void logInUser();
void logOutUser();
void servCommunicationLoop();
void sendUsersList();