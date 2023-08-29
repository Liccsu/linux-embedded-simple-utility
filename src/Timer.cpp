/*
 * Created by Administrator on 2023/8/24.
 */

#include "Timer.hpp"

Timer::Timer() : start_time(), end_time(), interval_time(), interval_time_t(0) {
}

std::chrono::steady_clock::time_point Timer::start() {
    this->start_time = std::chrono::steady_clock::now();

    return this->start_time;
}

std::chrono::steady_clock::time_point Timer::end() {
    this->end_time = std::chrono::steady_clock::now();
    this->interval_time = this->end_time - this->start_time;
    this->interval_time_t = std::chrono::duration_cast<std::chrono::microseconds>(this->interval_time).count();
    return this->end_time;
}

std::chrono::steady_clock::duration Timer::get_interval_time() const {
    return this->interval_time;
}

time_t Timer::get_interval_time_t() const {
    return this->interval_time_t;
}
