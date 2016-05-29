
extern CURL *easy_handle;

extern std::string autoReplyMsg;
extern bool mode;


void setReplyMsg() {
    std::cin >> autoReplyMsg;
    std::cout << "autoReplyMsg has changed to : \'" << autoReplyMsg << '\'' << std::endl;
}

void setReplyOn() {
    mode = true;
}
void setReplyOff() {
    mode = false;
}

