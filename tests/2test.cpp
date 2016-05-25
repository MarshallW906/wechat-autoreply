#include "cstdio"
#include "cstdlib"
#include "cstring"

#include "iostream"

using namespace std;

#include "curl/curl.h"

size_t process_data(void *buffer, size_t size, size_t nmemb, void *user_p)
{
    FILE *fp = (FILE *)user_p;
    size_t return_size = fwrite(buffer, size, nmemb, fp);
    cout << (char *)buffer << endl;
    return return_size;
}



int main() {
    // 初始化libcurl
    CURLcode return_code;
    return_code = curl_global_init(CURL_GLOBAL_SSL);
    if (CURLE_OK != return_code) {
        cerr << "init libcurl failed." << endl;
        return -1;
    }


    // 获取easy handleÏ
    CURL *easy_handle = curl_easy_init();
    if (NULL == easy_handle) {
        cerr << "get a easy handle failed." << endl;
        curl_global_cleanup();
        return -1;
    }

    FILE *fp = fopen("data.html", "ab+");   //
    // 设置easy handle属性
    curl_easy_setopt(easy_handle, CURLOPT_URL, "http://blog.csdn.net/JGood");
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &process_data);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);

    // 执行数据请求
    curl_easy_perform(easy_handle);

    // 释放资源
    fclose(fp);
    curl_easy_cleanup(easy_handle);
    curl_global_cleanup();

}