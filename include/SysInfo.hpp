/*
 * Created by Administrator on 2023/8/28.
 */

#ifndef GREEDYSNAKE_SYSINFO_HPP
#define GREEDYSNAKE_SYSINFO_HPP

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statfs.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <sstream>
#include <thread>
#include <utility>
#include "Utils.hpp"


#define Mb 1048576
#define gap_t 1

typedef struct CPU_PACKED {
    std::string name;
    ulong user;
    ulong nice;
    ulong system;
    ulong idle;
} CPU_OCCUPY;


class SysInfo {
private:
    CPU_OCCUPY old_st;
    CPU_OCCUPY new_st;

    ulong cpu_usage;
    ulong total_ram;
    ulong free_ram;
    ulong total_swap;
    ulong free_swap;

    static void get_cpu_occupy(CPU_OCCUPY &cpu_st);

    void cal_cpu_occupy();

    void get_ram_occupy();

public:
    SysInfo();

    ulong get_usage_cpu() const;

    ulong get_total_ram() const;

    ulong get_free_ram() const;

    ulong get_total_swap() const;

    ulong get_free_swap() const;

    ulong get_usage_ram() const;

    ulong get_usage_swap() const;
};


#endif //GREEDYSNAKE_SYSINFO_HPP
