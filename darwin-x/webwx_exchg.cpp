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

#include "webwx_exchg.h"

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

//      if (DEBUG) {
//          cout << "the GET url is " << endl;
//          cout << syncStr << endl;
//      }

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

            if (user_recvMsgFrom == MyUserInfo["UserName"].asString()) {
                continue;
                // ignore Message from myself.
            }

            // if (user_recvMsgFrom.length() <= 33) {
            //     continue;
            //     // ignore public account
            //     // "gong zhong hao"
            //     // actually it's a '@' + 32-character-long string
            // }
            MyUserName = response_root["AddMsgList"][i]["ToUserName"].asString();
            Users_to_Reply.push_back(user_recvMsgFrom);

            int correspondUser = find_NickName_by_UserName(user_recvMsgFrom);
            if (correspondUser == -1) {
                cout << "got a message from a Stranger. [will auto reply]" << endl;
            } else {
                std::string tmp_get_user = MyContactList[correspondUser]["NickName"].asString();
                std::string tmp_get_alias = MyContactList[correspondUser]["Alias"].asString();
                printf("got a message from %s [%s]. [will auto reply]\n",
                       tmp_get_user.c_str(),
                       tmp_get_alias.c_str());
            }
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

    int Msg_to_reply_count = Users_to_Reply.size();
    int Msg_replied_succeed_count = 0;

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

        // check the Response
        // if reply message failed, print it out as a remind.
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

        if (DEBUG) {
            printf("Ret : %s, ErrMsg : %s, MsgID : %s, LocalID : %s\n",
                   rett.c_str(),
                   terr_msg.c_str(),
                   tmsg_id.c_str(),
                   tlocal_id.c_str());
        }

        if (rett != "0") {
            int correspondUser = find_NickName_by_UserName(*iter);

            if (correspondUser == -1) {
                printf("failed to reply a message from stranger.\n");
                printf("You can get to know it by choose in the Menu.\n");
                Strangers_Reply_failed++;
            } else {
                std::string tmp_reply_fail_user = MyContactList[correspondUser]["NickName"].asString();
                std::string tmp_reply_fail_alias = MyContactList[correspondUser]["Alias"].asString();

                Users_Reply_failed.insert(pair<std::string, std::string>(tmp_reply_fail_user, tmp_reply_fail_alias));
                printf("failed to reply a message FROM %s, whose Alias is %s\n",
                       tmp_reply_fail_user.c_str(),
                       tmp_reply_fail_alias.c_str());
                printf("You can get to know it by choose in the Menu.\n");
            }
        } else {
            // reply succeeded.
            Msg_replied_succeed_count++;
        }
        // check completed
    }

    Users_to_Reply.clear();

    printf("Need to reply %d Messages, %d succeeded\n",
           Msg_to_reply_count,
           Msg_replied_succeed_count);
    if (Msg_to_reply_count > Msg_replied_succeed_count) {
        printf("To check the failure, Enter \'f\'(or \'F\'') at the menu\n");
    }

    return true;
}

int find_NickName_by_UserName(std::string tmp_UserName) {
    std::string tmpCon_User;
    for (int i = 0; i < Number_of_Contacts; i++) {
        tmpCon_User = MyContactList[i]["UserName"].asString();
        if (tmp_UserName == tmpCon_User) {
            return i;
        }
    }
    return -1;
}
