#include "chatInfo.h"

#define userConfig "userConfig.txt"


int namePasswordCmp(struct msgbuf message, char str[USERNAME_SIZE + PASSWORD_SIZE + 1]);
int getUserIndex(int searchedId);
int getGroupIndex(char searchedName[GROUPNAME_SIZE]);
int findEmptySlot(int groupIndex);

short isAlreadyInGroup(int groupIndex, int userIndex);

void loadUsersData();
void logInUser();
void logOutUser();
void servCommunicationLoop();
void sendUsersList();
void addUserToGroup();
void sendGroupList();
void removeUserFromGroup();
void sendGroupUsers();
