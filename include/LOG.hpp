/*
 * Created by Administrator on 2023/8/27.
 */

#ifndef GREEDYSNAKE_LOG_HPP
#define GREEDYSNAKE_LOG_HPP

#include <iostream>
#include <string>
#include <memory>
#include "Utils.hpp"

#define DEBUG 1


typedef enum {
    LOG_INFO = 0,
    LOG_WARN = 1,
    LOG_ERR = 2
} LOG_LEVEL;

inline std::string str_info(const std::string &str) {
    return "\033[32mI: " + str + "\033[39m\n";
}

inline std::string str_warn(const std::string &str) {
    return "\033[33mW: " + str + "\033[39m\n";
}

inline std::string str_err(const std::string &str) {
    return "\033[31mE: " + str + "\033[39m\n";
}

template<typename ... Args>
void LOG(LOG_LEVEL level, const std::string &format, Args ... args) {
#ifdef DEBUG
    std::string msg = Utils::str_format(format, args...);

    switch (level) {
        default:
        case LOG_INFO:
            std::cout << str_info(msg) << std::endl;
            break;
        case LOG_WARN:
            std::cout << str_warn(msg) << std::endl;
            break;
        case LOG_ERR:
            std::cout << str_err(msg) << std::endl;
            break;
    }
#endif
}

#endif //GREEDYSNAKE_LOG_HPP
