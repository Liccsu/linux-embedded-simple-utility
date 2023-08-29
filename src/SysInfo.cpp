/*
 * Created by Administrator on 2023/8/28.
 */

#include "SysInfo.hpp"


SysInfo::SysInfo() : old_st(), new_st(), cpu_usage(0), total_ram(0), free_ram(0), total_swap(0), free_swap(0) {
    std::thread t([this] {
        while (true) {
            SysInfo::get_cpu_occupy(this->old_st);
            std::this_thread::sleep_for(std::chrono::seconds(gap_t));
            SysInfo::get_cpu_occupy(this->new_st);

            this->cal_cpu_occupy();
            this->get_ram_occupy();
        }
    });

    t.detach();
}

void SysInfo::get_cpu_occupy(CPU_OCCUPY &cpu_st) {
    constexpr
    size_t buffSize = 256;
    char buff[buffSize];

    std::unique_ptr <std::ifstream> filePtr(new std::ifstream("/proc/stat"));
    if (!filePtr->is_open()) {
        return;
    }
    filePtr->read(buff, buffSize);

    std::string buffView(buff, filePtr->gcount());

    std::istringstream iss((std::string(buffView)));
    iss >> cpu_st.name >> cpu_st.user >> cpu_st.nice >> cpu_st.system >> cpu_st.idle;
}

void SysInfo::cal_cpu_occupy() {
    ulong od, nd;
    ulong id, sd;
    //第一次(用户+优先级+系统+空闲)的时间再赋给od
    od = this->old_st.user + this->old_st.nice + this->old_st.system + this->old_st.idle;
    //第二次（用户+优先级+系统+空闲）的时间再赋给nd
    nd = this->new_st.user + this->new_st.nice + this->new_st.system + this->new_st.idle;
    //用户第一次和第二次的时间之差再赋给id
    id = this->new_st.user - this->old_st.user;
    //系统第一次和第二次的时间之差再赋给sd
    sd = this->new_st.system - this->old_st.system;
    if ((nd - od) != 0) {
        //((用户+系统)乘100)除(第一次和第二次的时间差)再赋给g_cpu_used
        this->cpu_usage = ((sd + id) * 100) / (nd - od);
    } else {
        this->cpu_usage = 0;
    }
}

ulong SysInfo::get_usage_cpu() const {
    return this->cpu_usage;
}

ulong SysInfo::get_total_ram() const {
    return this->total_ram;
}

ulong SysInfo::get_free_ram() const {
    return this->free_ram;
}

ulong SysInfo::get_total_swap() const {
    return this->total_swap;
}

ulong SysInfo::get_free_swap() const {
    return this->free_swap;
}

void SysInfo::get_ram_occupy() {
    struct sysinfo s_info{};
    if (sysinfo(&s_info) == 0) {
        this->total_ram = s_info.totalram / Mb;
        this->free_ram = s_info.freeram / Mb;
        this->total_swap = s_info.totalswap / Mb;
        this->free_swap = s_info.freeswap / Mb;
    }
}

ulong SysInfo::get_usage_ram() const {
    return this->total_ram - this->free_ram;
}

ulong SysInfo::get_usage_swap() const {
    return this->total_swap - this->free_swap;
}
