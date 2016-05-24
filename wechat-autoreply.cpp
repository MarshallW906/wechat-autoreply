#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>

// #include "sys/type.h"
// #include "regex.h"
#include "unistd.h"  // on *nix

#include <iostream>
#include <map>
using namespace std;

#include "curl/curl.h"
// #include "/usr/local/include/boost/regex.h"
#include <json/json.h>
#include "cmockery.h"

#ifndef W_AUTOREPLY_GLOBAL_VAR
#define W_AUTOREPLY_GLOBAL_VAR

int tip = 0;

std::string uuid;

std::string base_uri;
std::string redirect_uri;

std::string skey;
std::string wxsid;
std::string wxuin;
std::string pass_ticket;
std::string deviceId = "e000000000000000";

std::map<std::string, std::string> BaseRequest;

CURL *easy_handle;

// use re :

// regex_t reg;
// regmatch_t pm[10];
// const size_t nmatch = 10;
// char ebuf[128];

#endif  // W_AUTOREPLY_GLOBAL_VAR

#include "funcs.cpp"

bool getUUID() {
    // get uuid
    string toGetUUIDstr = string("https://login.weixin.qq.com/jslogin?") + InitGET();
    const char * toGetUUID = toGetUUIDstr.c_str();
    cout << toGetUUID << endl;

    FILE *fp = fopen("uuid.html", "wb");

    curl_easy_setopt(easy_handle, CURLOPT_URL, toGetUUID);
    curl_easy_setopt(easy_handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);
    cout << "geting uuid" << endl;
    curl_easy_perform(easy_handle);

    fclose(fp);

    FILE *fp2 = fopen("uuid.html", "r");
    char tempToGetuuid[128];
    fgets(tempToGetuuid, 127, fp2);
    fclose(fp);
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
    FILE *qrcodeFP = fopen("qrcode.jpg", "wb");
    cout << "file open qrcode.jpg succeeded. " << endl;
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, qrcodeFP);
    curl_easy_perform(easy_handle);

    fclose(qrcodeFP);
    tip = 1;
    cout << "扫描二维码 后台登录网页版微信" << endl;
    system("open qrcode.jpg");

    return true;
}

std::string waitForLogin() {
    // wait for login()
    char waiturl[128];
    sprintf(waiturl, "https://login.weixin.qq.com/cgi-bin/mmwebwx-bin/login?tip=%d&uuid=%s&_=%s", tip, uuid.c_str(), getCurrentSeconds().c_str() );
    cout << waiturl << endl;

    curl_easy_setopt(easy_handle, CURLOPT_URL, waiturl);

    FILE *waitForLoginFP = fopen("wfl.html", "wb");

    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, waitForLoginFP);
    curl_easy_perform(easy_handle);

    fclose(waitForLoginFP);

    waitForLoginFP = fopen("wfl.html", "rb");
    char tempWFL[2048], tempRedirect[512] = {0};
    fgets(tempWFL, 2047, waitForLoginFP);
    if (!feof(waitForLoginFP)) {
        fgets(tempRedirect, 511, waitForLoginFP);
    }
    cout << "WFL code : " << endl  << tempWFL << endl;

    string tmp(tempWFL), tmpR(tempRedirect);
    string::size_type position;
    position = tmp.find("=");
    if (position == tmp.npos) {
        cout << "WaitforLogin() Didn't get proper Response." << endl;
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
        redirect_uri = tmpR.substr(position + 2, tmpR.end() - tmpR.begin() - position - 4);
        position = redirect_uri.rfind("/");
        base_uri = redirect_uri.substr(0, position);
        cout << "base_uri : " << endl << base_uri << endl;
        cout << "redirect_uri : " << endl << redirect_uri << endl;
        system("osascript -e 'quit app \"Preview\"'");
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
        exit(0);
    }

    curl_easy_setopt(easy_handle, CURLOPT_URL, logCstr);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &get_request_toFILE);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, loginFP);

    curl_easy_perform(easy_handle);

    fclose(loginFP);

    if ((loginFP = fopen("forLogin.html", "rb")) == NULL) {
        cout << "open forLogin.html failed" << endl;
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

    cout << "skey : " << skey << " , wxsid : " << wxsid << " , wxuin : " << wxuin << " , pass_ticket : " << pass_ticket << endl;

    BaseRequest.insert(pair<std::string, std::string>("Uin", wxuin));
    BaseRequest.insert(pair<std::string, std::string>("Sid", wxsid));
    BaseRequest.insert(pair<std::string, std::string>("Skey", skey));
    BaseRequest.insert(pair<std::string, std::string>("DeviceID", deviceId));

    cout << BaseRequest.begin()->first << " -> " << BaseRequest.begin()->second << endl;
    return true;
}

// bool webwxinit() {
//     char tmpUri[64];
//     sprintf(tmpUri, "/webwxinit?pass_ticket=%s&skey=%s&r=%s", pass_ticket, skey, getCurrentSeconds().c_str());
//     std::string url = base_uri + string(tmpUri);


//     return false;
// }

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

    // if (!webwxinit()) {
    //     cout << "初始化失败" << endl;
    //     return;
    // }
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

    process_main();

    curl_easy_cleanup(easy_handle);

    curl_global_cleanup();

    return 0;
}
