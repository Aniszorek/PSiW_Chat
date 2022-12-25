#include "chatInfo.h"


int logIn();

void communicationLoop();
void logOut(int childPid);
void requestUsersList();
void requestGroupsList();
void requestGroupUsers(char groupName[MESSAGE_SIZE]);
void printReceivedList(char *message);
void groupJoin(char groupName[GROUPNAME_SIZE]);
void groupExit(char groupName[MESSAGE_SIZE]);
