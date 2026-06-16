#include "../include/utils.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>

namespace utils {
std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}
std::string toUpper(const std::string& str) {
    std::string r = str;
    std::transform(r.begin(), r.end(), r.begin(), ::toupper);
    return r;
}
int fibonacci(int n) {
    if (n <= 1) return n;
    int a=0,b=1;
    for(int i=2;i<=n;++i){int t=a+b;a=b;b=t;}
    return b;
}
std::vector<std::string> readLines(const std::string& fp) {
    std::vector<std::string> lines;
    std::ifstream f(fp);
    std::string l;
    while(std::getline(f,l)) lines.push_back(l);
    return lines;
}
}
