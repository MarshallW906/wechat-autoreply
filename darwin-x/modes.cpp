#include "modes.h"

using namespace std;

extern CURL *easy_handle;

extern Json::Value MyUserInfo;

extern std::string autoReplyMsg;
extern std::vector<std::string> ReplyMsgs;

extern bool ReplySwitch;
extern bool QuitProgram;

extern std::map<std::string, std::string> Users_Reply_failed;
extern int Strangers_Reply_failed;

void showCurrentUserInfo() {
    std::string tmp_NickName = MyUserInfo["NickName"].asString();

    printf("Current User's NickName : %s\n", tmp_NickName.c_str());
}

void setReplyMsg() {
    if (ReplySwitch == true) {
        cout << "Cannot change the reply message when the autoReply Switch is ON." << endl;
        cout << "Please Enter \'d\' or \'D\' to disable it and then try again." << endl;
        return;
    }
    cout << "Enter the new message you want to AUTO-Reply : " << endl;

    try {
        cout << "Enter 1 to select a pre-stored Msg, 2 to set your own reply Msg. : ";
        int mode;
        do {
             cin >> mode;
        } while (!(mode == 1 || mode == 2));

        switch (mode) {
            case 1:
            for (int i = 0; i < ReplyMsgs.size(); i++)
                cout << i + 1 << " : " << ReplyMsgs[i] << endl;
            do {
                cin >> mode;
                autoReplyMsg = ReplyMsgs[mode - 1];
            } while (!(mode > 0 && mode <= ReplyMsgs.size()));
            break;
            
            case 2:
            char c;
            c = getchar();
            getline(cin, autoReplyMsg);
            break;
        }   
    } catch (const std::exception& e) {
        cout << "Change autoReply Message failed." << endl;
        cout << "The following is the standard Error info." << endl;

        std::cerr << e.what() << '\n';
    }

    std::cout << "autoReplyMsg has changed to : \'" << autoReplyMsg << '\'' << std::endl;
};

void setReplyOn() {
    cout << "auto reply ON" << endl;
    ReplySwitch = true;
};

void setReplyOff() {
    cout << "auto reply OFF" << endl;
    ReplySwitch = false;
};

void quit_program() {
    QuitProgram = true;
};

void check_reply_failure() {
    int tmpC = 0;
    for (std::map<std::string, std::string>::iterator iter = Users_Reply_failed.begin();
        iter != Users_Reply_failed.end(); iter++) {
        tmpC++;
        cout << tmpC << "  " << iter->first << "   " << iter->second << endl;
    }
    cout << endl << "Number of Un-reply messages from strangers : " << Strangers_Reply_failed;

    char choice;
    cout << "Clear the List of Friends ?" << endl;
    cout << "(Enter 'y' to clear, any other key to cancel) : ";
    cin >> choice;
    if (choice == 'y') {
        Users_Reply_failed.clear();
        cout << "All of the un-reply-list has been cleaned." << endl;
    }

    cout << "Clear the Count of Strangers ?" << endl;
    cout << "(Enter 'y' to clear, any other key to cancel) : ";

    cin >> choice;
    if (choice == 'y') {
        Strangers_Reply_failed = 0;
        cout << "All of the Strangers count has been cleaned." << endl;
    }
}

void ClearAll_UnReply() {
    Users_Reply_failed.clear();
    Strangers_Reply_failed = 0;
    cout << "CLear all failure" << endl;
}

