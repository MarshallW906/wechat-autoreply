#include <string>
#include <ctype.h>
using namespace std;

extern bool DEBUG;

extern std::string SyncKey;
extern std::map<std::string, long> SyncKeyList;

struct MemoryStruct {
    char *memory;
    size_t size;
};

std::string getCurrentSeconds() {
    char tmp[16];
    sprintf(tmp, "%ld", time((time_t*)NULL));
    return std::string(tmp);
}

long getDecreasingSeconds_long() {
    char tmp[16];
    long ascending_time = time((time_t*)NULL);
    long desending_time = 3000000000L - ascending_time;
    return desending_time;
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

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

std::string createSync_urlform(int SyncKey_count) {
    char tmpKeyValue[128];
    std::vector<std::string> t_single_syncKey;
    for (std::map<std::string, long>::iterator iter = SyncKeyList.begin();
            iter != SyncKeyList.end(); iter++) {
        memset(tmpKeyValue, 0, sizeof(tmpKeyValue));
        sprintf(tmpKeyValue, "%s_%ld",
                iter->first.c_str(),
                iter->second);
        t_single_syncKey.push_back(string(tmpKeyValue));
    }

    SyncKey = t_single_syncKey[0];
    for (int i = 0; i < SyncKey_count - 1; i++) {
        SyncKey += string("%7C") + t_single_syncKey[i + 1];
    }

    return SyncKey;
}

std::string UrlEncode(const std::string& szToEncode) {
    std::string src = szToEncode;
    char hex[] = "0123456789ABCDEF";
    string dst;

    for (size_t i = 0; i < src.size(); ++i) {
        unsigned char cc = src[i];
        if (isascii(cc)) {
            if (cc == ' ') {
                dst += "%20";
            } else
                dst += cc;
        } else {
            unsigned char c = static_cast<unsigned char>(src[i]);
            dst += '%';
            dst += hex[c / 16];
            dst += hex[c % 16];
        }
    }
    // cout << "encode result :" << endl << dst << endl;
    return dst;
}


std::string UrlDecode(const std::string& szToDecode) {
    std::string result;
    int hex = 0;
    for (size_t i = 0; i < szToDecode.length(); ++i) {
        switch (szToDecode[i]) {
        case '+':
            result += ' ';
            break;
        case '%':
            if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2])) {
                std::string hexStr = szToDecode.substr(i + 1, 2);
                hex = strtol(hexStr.c_str(), 0, 16);
                //字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@]
                //可以不经过编码直接用于URL
                if (!((hex >= 48 && hex <= 57) || //0-9
                        (hex >= 97 && hex <= 122) ||  //a-z
                        (hex >= 65 && hex <= 90) ||   //A-Z
                        //一些特殊符号及保留字[$-_.+!*'(),]  [$&+,/:;=?@]
                        hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
                        || hex == 0x2a || hex == 0x2b || hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f
                        || hex == 0x3A || hex == 0x3B || hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f
                     )) {
                    result += char(hex);
                    i += 2;
                } else result += '%';
            } else {
                result += '%';
            }
            break;
        default:
            result += szToDecode[i];
            break;
        }
    }
    return result;
}

std::string MsgID_generator() {
    std::string ret = "";
    ret += getCurrentSeconds().substr(0, 7);
    ret += getCurrentSeconds();
    // -2000000L ?
    return ret;
}
