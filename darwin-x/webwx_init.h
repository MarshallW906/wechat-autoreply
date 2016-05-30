#ifndef WECHAT_AUTOREPLY_INIT_FUNCS
#define WECHAT_AUTOREPLY_INIT_FUNCS

bool getUUID();
bool showQRImage();

std::string waitForLogin();
bool login();

bool webwxinit_get_initJSON();
bool webwxinit_read_initJSON();
bool getContactList();

bool webwxinit();

#endif
