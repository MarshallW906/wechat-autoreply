#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <cassert>

// #include "sys/type.h"
// #include "regex.h"
#include "unistd.h"  // on *nix

#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
// #include <map>

using namespace std;

#include "curl/curl.h"
#include "json/json.h"
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

std::vector<std::string> Users_to_Reply;
std::string MyUserName;

Json::Value BaseRequest(Json::objectValue);
Json::FastWriter json_writer;
std::string BaseRequest_str;

CURL *easy_handle;
CURLcode curlRet;
curl_slist *plist_wx;

// { for modes.cpp
std::string autoReplyMsg = "now busy. [This msg is sent by autoReply.cpp]";
bool mode;
// } for modes.cpp

// use C regex :

// regex_t reg;
// regmatch_t pm[10];
// const size_t nmatch = 10;
// char ebuf[128];

// std::vector<std::string> SpecialUsers = {"newsapp", "fmessage", "filehelper", "weibo", "qqmail", "tmessage", "qmessage", "qqsync", "floatbottle", "lbsapp", "shakeapp", "medianote", "qqfriend", "readerapp", "blogapp", "facebookapp", "masssendapp", "meishiapp", "feedsapp", "voip", "blogappweixin", "weixin", "brandsessionholder", "weixinreminder", "wxid_novlwrv3lqwv11", "gh_22b87fa7cb3c", "officialaccounts", "notification_messages", "wxitil", "userexperience_alarm"};

#endif  // W_AUTOREPLY_GLOBAL_VAR

#include "funcs.cpp"
#include "modes.cpp"

bool getUUID() {
    // get uuid
    string toGetUUIDstr = string("https://login.weixin.qq.com/jslogin?") + InitGET();
    const char * toGetUUID = toGetUUIDstr.c_str();
    cout << toGetUUID << endl;

    FILE *fp;
    if ((fp = fopen("uuid.html", "wb")) == NULL) {
        cout << "open uuid.html failed. " << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    curl_easy_setopt(easy_handle, CURLOPT_URL, toGetUUID);
    curl_easy_setopt(easy_handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);
    cout << "geting uuid" << endl;
    curl_easy_perform(easy_handle);

    fclose(fp);

    FILE *fp2;
    if ((fp2 = fopen("uuid.html", "r")) == NULL) {
        cout << "open uuid.html (ad read) failed." << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }
    char tempToGetuuid[128];
    fgets(tempToGetuuid, 127, fp2);
    fclose(fp2);
    cout << tempToGetuuid << endl;

    std::string tmp(tempToGetuuid);
    std::string window_code = tmp.substr(22, 3);
    uuid = tmp.substr(50, 12);

    cout << window_code << "," << uuid << endl;

    cout << "wait for an arbitrary input to continue" << endl;
    cin >> tmp;
    if (window_code == "200") return true;

    return false;
}

bool showQRImage() {

    string toGetQRcode = string("https://login.weixin.qq.com/qrcode/") + uuid + QRcodeGET();
    cout << toGetQRcode << endl;
    const char *tgqr = toGetQRcode.c_str();

    cout << "getting qrcode ..." << endl;
    cout << tgqr << endl;
    curl_easy_setopt(easy_handle, CURLOPT_URL, tgqr);

    FILE *qrcodeFP;
    if ((qrcodeFP = fopen("qrcode.jpg", "wb")) == NULL) {
        cout << "cannot make a new file qrcode.jpg" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    if (DEBUG) {
        cout << "file open qrcode.jpg succeeded. " << endl;
    }

    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, qrcodeFP);
    curl_easy_perform(easy_handle);

    fclose(qrcodeFP);

    tip = 1;

    cout << "扫描二维码 后台登录网页版微信" << endl;

    // on darwin-apple-xxxx
    system("open qrcode.jpg");

    return true;
}

std::string waitForLogin() {
    // wait for login()
    char waiturl[128];
    sprintf(waiturl, "https://login.weixin.qq.com/cgi-bin/mmwebwx-bin/login?tip=%d&uuid=%s&_=%s", tip, uuid.c_str(), getCurrentSeconds().c_str() );

    if (DEBUG) {
        cout << waiturl << endl;
    }

    curl_easy_setopt(easy_handle, CURLOPT_URL, waiturl);

    FILE *waitForLoginFP;
    if ((waitForLoginFP = fopen("wfl.html", "wb")) == NULL) {
        cout << "open wfl.html failed." << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, waitForLoginFP);
    curl_easy_perform(easy_handle);

    fclose(waitForLoginFP);

    if ((waitForLoginFP = fopen("wfl.html", "rb")) == NULL) {
        cout << "open wfl.html (as read) failed" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }
    char tempWFL[2048], tempRedirect[512] = {0};
    fgets(tempWFL, 2047, waitForLoginFP);

    if (!feof(waitForLoginFP)) {
        fgets(tempRedirect, 511, waitForLoginFP);
    }

    if (DEBUG) {
        cout << "WFL code : " << endl  << tempWFL << endl;
    }

    string tmp(tempWFL), tmpR(tempRedirect);
    string::size_type position;
    position = tmp.find("=");

    if (position == tmp.npos) {
        cout << "WaitforLogin() Didn't get proper Response." << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    std::string window_code = tmp.substr(position + 1, 3);
    cout << "window_code : " << window_code << endl;

    // wait for login() completed



    if (window_code == "201") {
        cout << "成功扫描,请在手机上点击确认以登录" << endl;
        tip = 0;
    } else if (window_code == "200") { /* 已登录 */
        cout << "正在登录..." << endl;
        position = tmpR.find("=\"");
        redirect_uri = tmpR.substr(position + 2, tmpR.end() - tmpR.begin() - position - 4) + "&fun=new";
        position = redirect_uri.rfind("/");
        base_uri = redirect_uri.substr(0, position);
        cout << "base_uri : " << endl << base_uri << endl;
        cout << "redirect_uri : " << endl << redirect_uri << endl;

        // on darwin-apple-xxxx
        system("osascript -e 'quit app \"Preview\"'");
        // on other platform, re-write it

    } else if (window_code == "408") {
        ;
        // Time out.
    }

    return window_code;
}

bool login() {
    const char *logCstr = redirect_uri.c_str();
    char responseForLogin[512];

    FILE *loginFP;
    if ((loginFP = fopen("forLogin.html", "wb")) == NULL) {
        cout << "open forLogin.html failed" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    curl_easy_setopt(easy_handle, CURLOPT_URL, logCstr);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, loginFP);

    curl_easy_perform(easy_handle);

    fclose(loginFP);

    if ((loginFP = fopen("forLogin.html", "rb")) == NULL) {
        cout << "open forLogin.html failed" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }
    fgets(responseForLogin, 511, loginFP);

    // cout << "response for Login : " << endl;
    // cout << responseForLogin << endl;

    std::string tmpResFL(responseForLogin);
    std::string::size_type positionStart, positionStop;

    positionStart = tmpResFL.find("<skey>");
    if (positionStart == tmpResFL.npos) {
        return false;
    }
    positionStop = tmpResFL.find("</skey>");
    if (positionStart == tmpResFL.npos) {
        return false;
    }
    skey = tmpResFL.substr(positionStart + 6, positionStop - positionStart - 6);

    positionStart = tmpResFL.find("<wxsid>");
    if (positionStart == tmpResFL.npos) {
        return false;
    }
    positionStop = tmpResFL.find("</wxsid>");
    if (positionStart == tmpResFL.npos) {
        return false;
    }
    wxsid = tmpResFL.substr(positionStart + 7, positionStop - positionStart - 7);

    positionStart = tmpResFL.find("<wxuin>");
    if (positionStart == tmpResFL.npos) {
        return false;
    }
    positionStop = tmpResFL.find("</wxuin>");
    if (positionStart == tmpResFL.npos) {
        return false;
    }
    wxuin = tmpResFL.substr(positionStart + 7, positionStop - positionStart - 7);

    positionStart = tmpResFL.find("<pass_ticket>");
    if (positionStart == tmpResFL.npos) {
        return false;
    }
    positionStop = tmpResFL.find("</pass_ticket>");
    if (positionStart == tmpResFL.npos) {
        return false;
    }
    pass_ticket =  tmpResFL.substr(positionStart + 13, positionStop - positionStart - 13);

    if (DEBUG) {
        cout << "skey : " << skey << " , wxsid : " << wxsid << " , wxuin : " << wxuin << " , pass_ticket : " << pass_ticket << endl;
    }

    BaseRequest["Uin"] = wxuin;
    BaseRequest["Sid"] = wxsid;
    BaseRequest["Skey"] = skey;
    BaseRequest["DeviceID"] = deviceId;
    BaseRequest_str = json_writer.write(BaseRequest);

    if (DEBUG) {
        cout << "login succeeded." << endl;
        cout << "BaseRequest_str (json) : " << endl << BaseRequest_str << endl;
    }
    return true;
}

bool webwxinit_get_initJSON() {
    char tmpUri[512];
    const char *tmp_pass_ticket = pass_ticket.c_str();
    const char *tmp_skey = skey.c_str();
    const char *tmp_sec_time = getCurrentSeconds().c_str();
    sprintf(tmpUri, "/webwxinit?pass_ticket=%s&skey=%s&r=%s",
            tmp_pass_ticket,
            tmp_skey,
            tmp_sec_time);
    std::string url = base_uri + string(tmpUri);

    // std::string::size_type position_40;
    // position_40 = url.find("@");
    // url.replace(position_40, 1, "%40");
    const char *init_url = url.c_str();

    Json::Value post_data(Json::objectValue);
    Json::FastWriter post_data_writer;

    post_data["BaseRequest"] = BaseRequest;


    // for temporarily interrupt
    // cin >> tmpUri;

    std::string post_data_str = post_data_writer.write(post_data).c_str();
    const char *p_str = post_data_str.c_str();

    if (DEBUG) {
        cout << p_str << endl;
    }

    curl_easy_setopt(easy_handle, CURLOPT_POST, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, p_str);

    plist_wx = curl_slist_append(NULL, "ContentType:application/json; charset=UTF-8");
    curl_slist_append(plist_wx, "User-Agent:Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36");

    curlRet = curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, plist_wx);


    FILE *initFP;
    if ((initFP = fopen("init.json", "wb")) == NULL) {
        cout << "open init.json failed." << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }
    curl_easy_setopt(easy_handle, CURLOPT_URL, init_url);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, initFP);
    curl_easy_perform(easy_handle);

    fclose(initFP);

    return true;
}

bool webwxinit_read_initJSON() {
    ifstream initFS;
    initFS.open("init.json");
    assert(initFS.is_open());

    Json::Reader json_reader;
    Json::Value init_root;

    if (!json_reader.parse(initFS, init_root, false)) {
        cout << "init.json parsing error" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }
    initFS.close();

    if (DEBUG) {
        cout << "initFS Write succeeded." << endl;
    }

    // update SyncKeyList
    SyncKeyList.clear();
    int SyncKey_count = init_root["SyncKey"]["Count"].asInt();
    for (int i = 0; i < SyncKey_count; i++) {
        SyncKeyList.insert(pair<std::string, long>(init_root["SyncKey"]["List"][i]["Key"].asString(), init_root["SyncKey"]["List"][i]["Val"].asLargestInt()));
    }
    // update completed.

    createSync_urlform(SyncKey_count);

    // std::string ErrMsg = init_root["BaseResponse"]["ErrMsg"].asString();
    int Ret = init_root["BaseResponse"]["Ret"].asInt();

    if (DEBUG) {
        cout << init_root["BaseResponse"] << endl;
        cout << "SyncKey : " << endl << SyncKey << endl;
        cout << "Ret : " << Ret << endl;
        // printf("Ret : %d, ErrMsg: %s\n", Ret, ErrMsg.c_str());
        // << ", ErrMsg : " << ErrMsg
    }

    if ((int)Ret != 0) {
        cout << "will return false" << endl;
        return false;
    }

    cout << "will return true" << endl;
    return true;
}

bool webwxinit() {
    if (!webwxinit_get_initJSON()) {
        cout << "get init.json failed." << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    bool res = webwxinit_read_initJSON();

    return res;
}
bool syncCheck_GET() {
    string url = base_uri + string("/synccheck?");
    char tmpSyncParams[512];
    sprintf(tmpSyncParams, "skey=%s&sid=%s&uin=%s&deviceId=%s&synckey=%s&r=%s",
            skey.c_str(),
            wxsid.c_str(),
            wxuin.c_str(),
            deviceId.c_str(),
            SyncKey.c_str(),
            getCurrentSeconds().c_str());

    std::string syncStr = url + string(tmpSyncParams);

    if (DEBUG) {

    }

    std::string::size_type position_40;
    position_40 = syncStr.find("@");
    syncStr.replace(position_40, 1, "%40");

    const char *sync_url = syncStr.c_str();

    if (DEBUG) {
        cout << "the GET url is " << endl;
        cout << syncStr << endl;
    }

    curl_easy_setopt(easy_handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_POST, 0);
    curl_easy_setopt(easy_handle, CURLOPT_URL, sync_url);

    FILE *syncFP;
    if ((syncFP = fopen("syncCode.html", "wb")) == NULL) {
        cout << "cannot open syncCode.html" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, syncFP);

    curl_easy_perform(easy_handle);
    fclose(syncFP);

    if ((syncFP = fopen("syncCode.html", "rb")) == NULL) {
        cout << "cannot open syncCode.html" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }


    char tmp_syncCode[128];
    fgets(tmp_syncCode, 127, syncFP);

    std::string synccheck_response(tmp_syncCode);

    std::string::size_type valuePos1, valuePos2;
    valuePos1 = synccheck_response.find("retcode:\"");
    valuePos2 = synccheck_response.find("\",");
    std::string tmp_retcode = synccheck_response.substr(valuePos1 + 9, valuePos2 - valuePos1 - 9);

    valuePos1 = synccheck_response.find("selector:\"");
    valuePos2 = synccheck_response.find("\"}");
    std::string tmp_selector = synccheck_response.substr(valuePos1 + 10, valuePos2 - valuePos1 - 10);

    if (DEBUG) {
        printf("retcode : %s, selector : %s\n",
               tmp_retcode.c_str(),
               tmp_selector.c_str());
    }

    fclose(syncFP);

    if (tmp_retcode == "1101") {
        cout << "操作过于频繁，请退出20秒后再运行" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    if (tmp_retcode != "0") {
        return false;
        // Unknown error, ignored
    }

    if (tmp_selector != "0") {
        return true;
        // some Message has come
    }

    return false;

}

int syncCheckMsg_POST() {

    string url = base_uri + string("/webwxsync?");
    char tmpSyncPostUrl[512];
    sprintf(tmpSyncPostUrl, "sid=%s/&skey=%s&pass_ticket=%s",
            wxsid.c_str(),
            skey.c_str(),
            pass_ticket.c_str());
    std::string tmp_sync_post_str = url + string(tmpSyncPostUrl);

    std::string::size_type position_40;
    position_40 = tmp_sync_post_str.find("@");
    tmp_sync_post_str.replace(position_40, 1, "%40");
    const char *sync_post_url = tmp_sync_post_str.c_str();

    if (DEBUG) {
        cout << "the POST url is " << endl;
        cout << sync_post_url << endl;
    }

    // create post datafield (as Json)
    Json::Value root(Json::objectValue);
    Json::Value t_BaseReqest(Json::objectValue);
    Json::FastWriter json_request_writer;
    t_BaseReqest["Uin"] = wxuin;
    t_BaseReqest["Sid"] = wxsid;
    t_BaseReqest["SKey"] = skey;
    t_BaseReqest["DeviceID"] = deviceId;
    root["BaseRequest"] = t_BaseReqest;

    Json::Value t_SyncKey(Json::objectValue);
    t_SyncKey["Count"] = (unsigned int)SyncKeyList.size();
    int tmp_synckey_count = SyncKeyList.size();

    // cout << "creating t_List----------------" << endl;
    Json::Value t_List;
    for (std::map<std::string, long>::iterator iter = SyncKeyList.begin();
            iter != SyncKeyList.end(); iter++) {

        Json::Value tmpSK(Json::objectValue);
        tmpSK["Key"] = iter->first;
        tmpSK["Val"] = (unsigned int)iter->second;

        // cout << "Key and Val registered" << endl;
        // cout << "Key : " << tmpSK["Key"].asString() << endl;
        // cout << "Val : " << tmpSK["Val"].asString() << endl;

        t_List.append(tmpSK);

    }
    // cout << "creating t_List succeeded----------------" << endl;


    t_SyncKey["List"] = t_List;
    root["SyncKey"] = t_SyncKey;
    // root["rr"] = (unsigned int)getDecreasingSeconds_long();
    std::string post_json_data = json_request_writer.write(root);

    const char *post_json_datafield = post_json_data.c_str();

    if (DEBUG) {
        cout << "syncCheckMsg_POST Json_Data : " << endl;
        cout << post_json_datafield << endl;
    }

    // JSON create completed.

    FILE *chkMsgFP;
    if ((chkMsgFP = fopen("chkMsg.json", "wb")) == NULL) {
        cout << "open chkMsg.json failed" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }
    // perform https request
    curl_easy_setopt(easy_handle, CURLOPT_HTTPGET, 0);
    curl_easy_setopt(easy_handle, CURLOPT_POST, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_URL, sync_post_url);
    curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, post_json_datafield);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, chkMsgFP);
    curl_easy_perform(easy_handle);
    fclose(chkMsgFP);

    ifstream chkMsgFS;
    chkMsgFS.open("chkMsg.json");
    assert(chkMsgFS.is_open());

    Json::Reader json_reader;
    Json::Value response_root;

    if (!json_reader.parse(chkMsgFS, response_root, false)) {
        cout << "chkMsgFS.json parsing error" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    // update SyncKeyList
    SyncKeyList.clear();
    int count = response_root["SyncKey"]["Count"].asInt();
    for (int i = 0; i < count; i++) {
        SyncKeyList.insert(pair<std::string, long>(response_root["SyncKey"]["List"][i]["Key"].asString(), response_root["SyncKey"]["List"][i]["Val"].asLargestInt()));
    }
    // update completed.

    // get the Received Message
    int MsgCount = response_root["AddMsgCount"].asInt();

    if (MsgCount != 0) {
        std::string user_recvMsgFrom;

        for (int i = 0; i < MsgCount; i++) {
            if (response_root["AddMsgList"][i]["AppMsgType"].asInt() != 0) {
                continue;
                // ignore App message
            }

            user_recvMsgFrom = response_root["AddMsgList"][i]["FromUserName"].asString();
            std::string::size_type position;
            position = user_recvMsgFrom.find("@@");
            // from chatrooms
            if (position != user_recvMsgFrom.npos) {
                continue;
                // ignore chatrooms
            }

            // if (user_recvMsgFrom.length() <= 33) {
            //     continue;
            //     // ignore public account
            //     // "gong zhong hao"
            //     // actually it's a '@' + 32-character-long string
            // }
            MyUserName = response_root["AddMsgList"][i]["ToUserName"].asString();
            Users_to_Reply.push_back(user_recvMsgFrom);
            // update the reply-list (stored in std::vector)
        }
    }

    if (DEBUG) {
        cout << "The reply-list (vector) :" << endl;
        for (std::vector<string>::iterator iter = Users_to_Reply.begin();
                iter != Users_to_Reply.end(); iter++) {
            cout << *iter << endl;
        }
    }

    return Users_to_Reply.size();
}

bool sync_SendMsg_forReply_POST() {
    std::string sendMsg_str = base_uri + string("/webwxsendmsg");
    const char *sendMsg_url = sendMsg_str.c_str();
    // create POST url completed.

    Json::Value root;
    Json::FastWriter json_writer;

    root["BaseRequest"] = BaseRequest;

    Json::Value Msg(Json::objectValue);
    Msg["Content"] = autoReplyMsg;
    Msg["FromUserName"] = MyUserName;
    Msg["Type"] = 1;

    root["Msg"] = Msg;
    root["Scene"] = 0;
    // create general POST datafield completed.

    curl_easy_setopt(easy_handle, CURLOPT_URL, sendMsg_url);

    for (std::vector<std::string>::iterator iter = Users_to_Reply.begin();
            iter != Users_to_Reply.end(); iter++) {
        root["Msg"]["ToUserName"] = *iter;
        std::string tmsg_id_post = MsgID_generator();
        root["Msg"]["ClientMsgId"] = tmsg_id_post;
        root["Msg"]["LocalID"] = tmsg_id_post;

        std::string post_json_data = json_writer.write(root);
        const char* post_json_datafield = post_json_data.c_str();

        if (DEBUG) {
            cout << root << endl;
        }

        FILE *sendMsg_Respon;
        if ((sendMsg_Respon = fopen("sendMsg_Respon.json", "wb")) == NULL) {
            cout << "cannot open and write sendMsg_Respon.json" << endl;
            curl_easy_cleanup(easy_handle);
            curl_global_cleanup();
            exit(0);
        }

        curl_easy_setopt(easy_handle, CURLOPT_HTTPGET, 0);
        curl_easy_setopt(easy_handle, CURLOPT_POST, 1L);
        curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, post_json_datafield);
        curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, sendMsg_Respon);

        curl_easy_perform(easy_handle);
        fclose(sendMsg_Respon);

        if (DEBUG) {
            ifstream sendMsg_ResponFS;
            sendMsg_ResponFS.open("sendMsg_Respon.json");
            assert(sendMsg_ResponFS.is_open());

            Json::Reader json_reader;
            Json::Value sMsg_Respon_root;

            if (!json_reader.parse(sendMsg_ResponFS, sMsg_Respon_root, false)) {
                cout << "sendMsg_Respon.json parsing error" << endl;
                curl_easy_cleanup(easy_handle);
                curl_global_cleanup();
                exit(0);
            }
            sendMsg_ResponFS.close();

            std::string rett = sMsg_Respon_root["BaseResponse"]["Ret"].asString();
            std::string terr_msg = sMsg_Respon_root["BaseResponse"]["ErrMsg"].asString();
            std::string tmsg_id = sMsg_Respon_root["MsgID"].asString();
            std::string tlocal_id = sMsg_Respon_root["LocalID"].asString();

            printf("Ret : %s, ErrMsg : %s, MsgID : %s, LocalID : %s\n",
                rett.c_str(),
                terr_msg.c_str(),
                tmsg_id.c_str(),
                tlocal_id.c_str());
        }
    }

    Users_to_Reply.clear();

    return true;
}

void process_main() {

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

    if (!webwxinit()) {
        cout << "初始化失败" << endl;
        return;
    }

    if (DEBUG) {
        cout << endl << "init succeeded" << endl << endl;;
    }

    while (1) {
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
            if (tmpMsgRecvCount != 0) {
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

    curl_easy_cleanup(easy_handle);

    curl_global_cleanup();

    return 0;
}
