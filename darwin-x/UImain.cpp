#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "UImain.h"
#include "modes.cpp"

using namespace std;

void print_UI_main() {
    cout << "Main menu : " << endl;
    cout << " A, a : show the current user's NickName" << endl;
    cout << " S, s : set the autoreply-Msg [only string is available]" << endl;
    cout << " E, e : enable auto-Reply " << endl;
    cout << " D, d : disable auto-Reply " << endl;
    cout << " F, f : check all the reply-failure.[didn't reply by unknown reasons]" << endl;
    cout << " W, w : Clear ALL the reply-failure [include Friends and Strangers]" << endl;
    cout << " Q, q : quit the program. " << endl;
    cout << " H, h : show the Main menu again." << endl;
}

void* UI_main(void *para) {
    cout << "Wechat-auto-Reply.cpp Starting.." << endl;

    char choice;
    bool quit = false;

    showCurrentUserInfo();
    print_UI_main();
    do {
        cin >> choice;

        switch (choice) {
            case 'A':
            case 'a':
            showCurrentUserInfo();
            break;

            case 'F':
            case 'f':
            check_reply_failure();
            break;

            case 'W':
            case 'w':
            ClearAll_UnReply();
            break;

            case 'H':
            case 'h':
            print_UI_main();
            break;

            case 'S':
            case 's':
            setReplyMsg();
            quit = false;
            break;
            // set the auto-reply Msg;

            case 'E':
            case 'e':
            setReplyOn();
            quit = false;
            break;
            // enable auto-Reply;

            case 'D':
            case 'd':
            setReplyOff();
            quit = false;
            break;
            // disable auto-Reply;

            case 'Q':
            case 'q':
            cout << "Now will logout and quit." << endl;
            cout << "If you want to Re-Login with the same account, please wait for 30s at least." << endl;
            quit_program();
            break;

            default:
            quit = false;
        }
    } while (quit == false);

    quit_program();

    return NULL;
}
