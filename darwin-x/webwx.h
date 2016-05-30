#ifndef WECHAT_AUTOREPLY_MAINFUNCS
#define WECHAT_AUTOREPLY_MAINFUNCS

bool getUUID();
bool showQRImage();

std::string waitForLogin();
bool login();

bool webwxinit_get_initJSON();
bool webwxinit_read_initJSON();
bool getContactList();

bool webwxinit();

int find_NickName_by_UserName(std::string);

bool syncCheck_GET();
int syncCheckMsg_POST();
bool sync_SendMsg_forReply_POST();

#endif
