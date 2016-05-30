#ifndef WECHAT_AUTOREPLY_MSGEXCHG_FUNCS
#define WECHAT_AUTOREPLY_MSGEXCHG_FUNCS

int find_NickName_by_UserName(std::string);

bool syncCheck_GET();
int syncCheckMsg_POST();
bool sync_SendMsg_forReply_POST();

#endif
