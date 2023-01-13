#include "inf151805_inf151891_chatInfo.h"

#define userConfig "userConfig.txt"

#define EMPTY_STRING "\0"

#define USERNAME_INCORRECT -1
#define PASSWORD_INCORRECT -2
#define USER_LOGGED_IN -3
#define TOO_MANY_LOGIN_ATTEMPTS -4
#define LOGIN_ERROR -5
#define USERNAME_PASSWORD_CORRECT 1


#define USER_NOT_EXISTS -50
#define GROUP_NOT_EXISTS -51
#define USER_NOT_LOGGED_IN -52
#define USER_MUTED_ALREADY -53
#define MUTE_SELF_ERROR -54
#define GROUP_MUTED_ALREADY -55
#define USER_MUTED -56
#define DM_SELF_ERROR -57

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

void showMuted();
void muteUser();
void unmuteUser();
void muteGroup();
void unmuteGroup();

void sortMutedUserames(int userindex);
void sortMutedGroupnames(int userindex);
