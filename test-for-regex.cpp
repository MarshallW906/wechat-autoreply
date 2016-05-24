#include "iostream"
#include "cstdio"
#include <regex.h>

using namespace std;

int main() {
    regex_t reg;
    regmatch_t pm[10];
    const size_t nmatch = 10;
    char ebuf[128];

    string str = "window.QRLogin.code = 200; window.QRLogin.uuid = \"wcLk3s1_vQ==\";";

    cout << str << endl;

    const char *reg_uuid = "window.QRLogin.code = {*}; window.QRLogin.uuid = \"{*}\"";
    int z = regcomp(&reg, reg_uuid, 0);
    if (z) {
        regerror(z, &reg, ebuf, sizeof(ebuf));
        fprintf(stderr, "%s: pattern '%s' \n", ebuf, str.c_str());
        return false;
    }

    z = regexec( &reg, str.c_str(), nmatch, pm, REG_EXTENDED);
    if (z == REG_NOMATCH) {
        cout << "Didn't get proper Response." << endl;
        return false;
    } else if (z != 0) {
        regerror(z, &reg, ebuf, sizeof(ebuf));
        fprintf(stderr, "%s: regcom('%s')\n", ebuf, str.c_str());
        return false;
    }

    // std::string tmp(str);
    std::string window_code = str.substr(pm[0].rm_so, pm[0].rm_eo);
    string uuid = str.substr(pm[1].rm_so, pm[1].rm_eo);

    cout << "window_code : " << window_code << ", uuid : " << uuid << endl;
}