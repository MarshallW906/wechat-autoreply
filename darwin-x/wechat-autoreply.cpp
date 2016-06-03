// g++ -I /usr/local/include/ -L /usr/local/lib/ -lcurl -ljsoncpp -lpthread wechat-autoreply.cpp

#include <cstdio>
#include <cstdlib>
#include <cstring>

// #include "sys/type.h"
// #include "regex.h"
#include "unistd.h"  // on *nix
#include "pthread.h"
// compile parameter : -lpthread

#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <map>

using namespace std;

#include "curl/curl.h"
#include "json/json.h"
// -lcurl -I /usr/local/include -L /usr/local/lib
// #include "boost/regex.h"

#ifndef W_AUTOREPLY_GLOBAL_VAR
#define W_AUTOREPLY_GLOBAL_VAR

bool DEBUG = true;

int tip = 0;

std::string uuid;

std::string base_uri;
std::string redirect_uri;

std::string skey;
std::string wxsid;
std::string wxuin;  // unsigned long ?
std::string pass_ticket;
std::string deviceId = "e000000000000000";

std::string SyncKey;
std::map<std::string, long> SyncKeyList;

Json::Value BaseRequest(Json::objectValue);
Json::FastWriter json_writer;
std::string BaseRequest_str;

Json::Value MyUserInfo;
Json::Value MyContactList;
int Number_of_Contacts;

std::vector<std::string> Users_to_Reply;
std::string MyUserName;

std::map<std::string, std::string> Users_Reply_failed;
int Strangers_Reply_failed = 0;

CURL *easy_handle;
CURLcode curlRet;
curl_slist *plist_wx;

// { for modes.cpp
std::string autoReplyMsg = "now busy. [This msg is sent by autoReply.cpp]";
std::vector<std::string> ReplyMsgs;

bool ReplySwitch = true;
bool QuitProgram = false;
// } for modes.cpp

// use C regex :

// regex_t reg;
// regmatch_t pm[10];
// const size_t nmatch = 10;
// char ebuf[128];

// std::vector<std::string> SpecialUsers = {"newsapp", "fmessage", "filehelper", "weibo", "qqmail", "tmessage", "qmessage", "qqsync", "floatbottle", "lbsapp", "shakeapp", "medianote", "qqfriend", "readerapp", "blogapp", "facebookapp", "masssendapp", "meishiapp", "feedsapp", "voip", "blogappweixin", "weixin", "brandsessionholder", "weixinreminder", "wxid_novlwrv3lqwv11", "gh_22b87fa7cb3c", "officialaccounts", "notification_messages", "wxitil", "userexperience_alarm"};

#endif  // W_AUTOREPLY_GLOBAL_VAR


#include "common_funcs.cpp"
#include "UImain.cpp"
#include "webwx_init.cpp"
#include "webwx_exchg.cpp"


void process_main() {

    ReplyMsgs.push_back(std::string("now busy. [This msg is sent by autoReply.cpp]"));
    ReplyMsgs.push_back(std::string("I have been asleep. zzzZZZZ... [auto Reply.cpp]"));
    ReplyMsgs.push_back(std::string("Doing homework, no disturbance =n=+!!! [auto Reply.cpp]"));
    
    if (!getUUID()) {
        cout << "获取uuid失败" << endl;
        return;
    }

    showQRImage();

    sleep(1);

    while (waitForLogin() != "200");

    // cout << "后台登录成功." << endl;
    system("rm qrcode.jpg");

    if (!login()) {
        cout << "登录失败" << endl;
        return;
    }

    bool is_init = webwxinit();

    if (!is_init) {
        cout << "初始化失败" << endl;
        return;
    }

    if (DEBUG) {
        cout << endl << "init succeeded" << endl << endl;;
    }

    int pth_ret;

    if (is_init && !DEBUG) {
        pthread_t shell_id;
        pth_ret = pthread_create(&shell_id, NULL, &UI_main, NULL);
    }

    if (!DEBUG && pth_ret) {
        cout << "Create pthread error!" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    while (!QuitProgram) {
        if (DEBUG) {
            cout << "Processing the syncing..." << endl;
        }
        if (syncCheck_GET()) {
            int tmpMsgRecvCount;
            tmpMsgRecvCount = syncCheckMsg_POST();
            if (DEBUG) {
                if (tmpMsgRecvCount) {
                    printf("Got %d Messages, now replying\n", tmpMsgRecvCount);
                }
            }
            sleep(1);
            if ((ReplySwitch) && tmpMsgRecvCount != 0) {
                sync_SendMsg_forReply_POST();
            }
        }
        sleep(1);
    }


}

int main(int argc, char const *argv[]) {
    CURLcode return_code;
    return_code = curl_global_init(CURL_GLOBAL_SSL);
    // use CURL_GLOBAL_ALL on windows
    if (CURLE_OK != return_code) {
        cerr << "init libcurl failed."  << endl;
        return -1;
    }

    // get an "easy_handle" for the session
    easy_handle = curl_easy_init();
    if (NULL == easy_handle) {
        cerr << "get easy_handle failed." << endl;
        return -1;
    }

    // record cookie
    curlRet = curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "");
    curlRet = curl_easy_setopt(easy_handle, CURLOPT_COOKIEJAR, "");
    // cookie

    process_main();

    if (!DEBUG) {
        system("rm chkMsg.json");
        system("rm forLogin.html");
        system("rm init.json");
        system("rm sendMsg_Respon.json");
        system("rm syncCode.html");
        system("rm uuid.html");
        system("rm wfl.html");
    }

    curl_easy_cleanup(easy_handle);

    curl_global_cleanup();

    return 0;
}
