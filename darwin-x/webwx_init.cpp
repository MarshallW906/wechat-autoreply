#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <cassert>

#include <fstream>
#include <vector>
#include <queue>
#include <map>

#include "unistd.h"  // on *nix


#include "curl/curl.h"
#include "json/json.h"
// -lcurl -I /usr/local/include -L /usr/local/lib
// #include "boost/regex.h"

using namespace std;

#include "webwx_init.h"


extern bool DEBUG;

extern int tip;

extern std::string uuid;

extern std::string base_uri;
extern std::string redirect_uri;

extern std::string skey;
extern std::string wxsid;
extern std::string wxuin;  // unsigned long ?
extern std::string pass_ticket;
extern std::string deviceId;

extern std::string SyncKey;
extern std::map<std::string, long> SyncKeyList;

extern Json::Value BaseRequest;
extern Json::FastWriter json_writer;
extern std::string BaseRequest_str;

extern Json::Value MyUserInfo;
extern Json::Value MyContactList;
extern int Number_of_Contacts;

extern std::vector<std::string> Users_to_Reply;
extern std::string MyUserName;

extern std::map<std::string, std::string> Users_Reply_failed;
extern int Strangers_Reply_failed;

extern CURL *easy_handle;
extern CURLcode curlRet;
extern curl_slist *plist_wx;

// { for modes.cpp
extern std::string autoReplyMsg;
extern bool ReplySwitch;
extern bool QuitProgram;
// } for modes.cpp


bool getUUID() {
    // get uuid
    string toGetUUIDstr = string("https://login.weixin.qq.com/jslogin?") + InitGET();
    const char * toGetUUID = toGetUUIDstr.c_str();

    if (DEBUG) {
        cout << toGetUUID << endl;
    }

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

    if (DEBUG) {
        cout << "geting uuid" << endl;
    }

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

    if (DEBUG) {
        cout << tempToGetuuid << endl;
    }

    std::string tmp(tempToGetuuid);
    std::string window_code = tmp.substr(22, 3);
    uuid = tmp.substr(50, 12);

    if (DEBUG) {
        cout << window_code << "," << uuid << endl;

        cout << "wait for an arbitrary input to continue" << endl;
        cin >> tmp;
    }
    if (window_code == "200") return true;

    return false;
}

bool showQRImage() {

    string toGetQRcode = string("https://login.weixin.qq.com/qrcode/") + uuid + QRcodeGET();

    if (DEBUG) {
        cout << toGetQRcode << endl;
    }

    const char *tgqr = toGetQRcode.c_str();

    if (DEBUG) {
        cout << "getting qrcode ..." << endl;
        cout << tgqr << endl;
    }

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
        if (DEBUG) {
            cout << "base_uri : " << endl << base_uri << endl;
            cout << "redirect_uri : " << endl << redirect_uri << endl;
        }
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

    // start getting MyUserInfo
    MyUserInfo["NickName"] = init_root["User"]["NickName"];
    MyUserInfo["UserName"] = init_root["User"]["UserName"];
    // getting MyUserInfo completed.

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

bool getContactList() {
    ifstream initFS_ContactList;
    initFS_ContactList.open("init.json");
    assert(initFS_ContactList.is_open());

    Json::Reader json_reader;
    Json::Value init_root_CL;
    if (!json_reader.parse(initFS_ContactList, init_root_CL, false)) {
        cout << "init.json parsing for getContactList, error" << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    initFS_ContactList.close();

    // start getting Contacts
    int Number_of_Contacts = init_root_CL["Count"].asInt();

    for (int i = 0; i < Number_of_Contacts; i++) {
        Json::Value tmpCont(Json::objectValue);
        tmpCont["UserName"] = init_root_CL["ContactList"][i]["UserName"];
        tmpCont["NickName"] = init_root_CL["ContactList"][i]["NickName"];
        tmpCont["Alias"] = init_root_CL["ContactList"][i]["Alias"];

        MyContactList.append(tmpCont);
    }
    // getting Contacts completed.

    return true;

}

bool webwxinit() {
    if (!webwxinit_get_initJSON()) {
        cout << "get init.json failed." << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    if (!webwxinit_read_initJSON()) {
        cout << "read init.json failed." << endl;
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        exit(0);
    }

    bool res = getContactList();
    return res;
}
