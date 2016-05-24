#include <string>

std::string getCurrentSeconds() {
    char tmp[16];
    sprintf(tmp, "%ld", time((time_t*)NULL));
    return std::string(tmp);
}

std::string InitGET() {
    return std::string("fun=new&lang=zh_CN&_=" + getCurrentSeconds() + "&appid=wx782c26e4c19acffb");
}

std::string QRcodeGET() {
    return std::string("?t=webwx&_=" + getCurrentSeconds());
}


size_t get_request_toFILE(void* buffer, size_t size, size_t nmemb, void* user_p) {
    FILE *fp = (FILE *)user_p;
    size_t return_size = fwrite(buffer, size, nmemb, fp);
    cout << "get the Data and stored to file... succeeded. " << endl;
    // cout << "size_t size : " << size << ", size_t nmemb : " << nmemb << ", size_t return_size : " << return_size << endl;
    return return_size;
}
