#ifndef UTIL_LOGGER_H
#define UTIL_LOGGER_H

#include <iostream>
#include <sstream>
#include <mutex>
#include <string>

namespace util {
inline std::mutex &cout_mutex() {
    static std::mutex m;
    return m;
}

template<typename... Args>
inline void log_info(Args&&... args) {
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args));
    std::lock_guard<std::mutex> lk(cout_mutex());
    std::cout << oss.str() << std::endl;
}

template<typename... Args>
inline void log_error(Args&&... args) {
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args));
    std::lock_guard<std::mutex> lk(cout_mutex());
    std::cerr << oss.str() << std::endl;
}

} // namespace util

#endif // UTIL_LOGGER_H