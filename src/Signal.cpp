/*
 * Created by Administrator on 2023/8/26.
 */

#include <iostream>
#include "Signal.hpp"


bool Signal::RUNNING = true;

void Signal::signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
        case SIGQUIT:
        case SIGKILL:
            std::cout << "\nreceived signal interrupt." << std::endl;
            Signal::RUNNING = false;
            exit(0);
        default:
            break;
    }
}
