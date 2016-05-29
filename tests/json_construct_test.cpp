#include <json/json.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <time.h>
using namespace std;

int main() {

    cout << time((time_t*)NULL) << endl;
    return 0;
}

// g++ -L /usr/local/lib/ -ljsoncpp -I /usr/local/include/
