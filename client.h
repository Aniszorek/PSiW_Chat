#include "chatInfo.h"


int logIn(int msgId,int ppid);

void communicationLoop();
void logOut(int childPid);
void requestUsersList();
void requestGroupsList();
void requestGroupUsers(char groupName[MESSAGE_SIZE]);
void printReceivedList(char *message);
void groupJoin(char groupName[GROUPNAME_SIZE]);
void groupExit(char groupName[MESSAGE_SIZE]);
void printHelp();

//sending messages
void sendDirectMessage(char username[USERNAME_SIZE]);

void sendGroupMessage(char groupname[GROUPNAME_SIZE]);


//mute/unmute
void showMuted();
void muteUser(char username[USERNAME_SIZE]);
void unmuteUser(char username[USERNAME_SIZE]);
void muteGroup(char groupname[GROUPNAME_SIZE]);
void unmuteGroup(char groupname[GROUPNAME_SIZE]);

