#ifndef WECHAT_AUTOREPLY_COMMON_FUNCS
#define WECHAT_AUTOREPLY_COMMON_FUNCS

std::string getCurrentSeconds();
long getDecreasingSeconds_long();

std::string InitGET();
std::string QRcodeGET();

size_t get_request_toFILE(void* buffer, size_t size, size_t nmemb, void* user_p);
size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);
// the curl_ease_Callback func used to get & write DATA

std::string createSync_urlform(int SyncKey_count);
// create a part of url for further use of syncing.


std::string UrlEncode(const std::string& szToEncode);
std::string UrlDecode(const std::string& szToDecode);
// urlform : encode & decode

std::string MsgID_generator();

#endif
