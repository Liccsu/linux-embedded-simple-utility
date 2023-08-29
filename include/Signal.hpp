/*
 * Created by Administrator on 2023/8/26.
 */

#ifndef GREEDYSNAKE_SIGNAL_HPP
#define GREEDYSNAKE_SIGNAL_HPP

#include <csignal>

namespace Signal {
    extern bool RUNNING;

    void signal_handler(int signal);
}

#endif //GREEDYSNAKE_SIGNAL_HPP
