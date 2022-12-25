#include "chatInfo.h"

int logIn();

void communicationLoop();
void logOut(int childPid);
void requestUsersList();
void printUsersList();
void groupJoin(char userInput[GROUPNAME_SIZE]);
void requestGroupsList();
void printGroupsList();
void groupExit(char userInput[MESSAGE_SIZE]);