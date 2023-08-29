/*
 * Created by Administrator on 2023/8/24.
 */

#ifndef GREEDYSNAKE_TIMER_HPP
#define GREEDYSNAKE_TIMER_HPP

#include <cstdint>
#include <chrono>


class Timer {
private:
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    std::chrono::steady_clock::duration interval_time;
    time_t interval_time_t;
public:
    Timer();

    std::chrono::steady_clock::time_point start();

    std::chrono::steady_clock::time_point end();

    std::chrono::steady_clock::duration get_interval_time() const;

    time_t get_interval_time_t() const;

    __attribute__((unused)) static void delay_us(time_t us) {
        auto start = std::chrono::steady_clock::now();
        while (true) {
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            if (duration.count() >= us) {
                break;
            }
        }
    }
};


#endif //GREEDYSNAKE_TIMER_HPP
