#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace utils {

std::string getCurrentTime();
std::string toUpper(const std::string& str);
int fibonacci(int n);
std::vector<std::string> readLines(const std::string& filepath);

} // namespace utils

#endif // UTILS_H
