#include "chatInfo.h"


int logIn();

void communicationLoop();
void logOut(int childPid);
void requestUsersList();
void printReceivedList(char *message);
void groupJoin(char groupName[GROUPNAME_SIZE]);
void requestGroupsList();
void groupExit(char groupName[MESSAGE_SIZE]);
void requestGroupUsers(char groupName[MESSAGE_SIZE]);