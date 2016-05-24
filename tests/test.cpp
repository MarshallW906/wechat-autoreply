#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>
using namespace std;

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    cout << "hello world" << endl;

    curl_global_cleanup();
}
