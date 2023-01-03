#include "chatInfo.h"

#define userConfig "userConfig.txt"

#define USERNAME_INCORRECT -1
#define PASSWORD_INCORRECT -2
#define USER_LOGGED_IN -3
#define TOO_MANY_LOGIN_ATTEMPTS -4
#define LOGIN_ERROR -5
#define USERNAME_PASSWORD_CORRECT 1


#define USER_NOT_EXISTS -50
#define GROUP_NOT_EXISTS -51

int namePasswordCmp(struct msgbuf message, char str[USERNAME_SIZE + PASSWORD_SIZE + 1],int userIndex);
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


void sendDirectMessage();
int checkIfUserExists(char username[USERNAME_SIZE]);

void sendGroupMessage();
