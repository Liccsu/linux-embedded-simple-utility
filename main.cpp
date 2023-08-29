#include "DisplayScreen.hpp"
#include "Signal.hpp"
#include "SysInfo.hpp"
#include "LOG.hpp"


int main() {
    signal(SIGINT, Signal::signal_handler);
    signal(SIGQUIT, Signal::signal_handler);
    signal(SIGKILL, Signal::signal_handler);
    std::shared_ptr <DisplayScreen> screen = DisplayScreen::create(20, SYNC_MODE_ON);
    try {
        int32_t flag = 1;
        int32_t r1 = 60;
        int32_t r2 = 60;
        SysInfo info;
        while (Signal::RUNNING) {
            for (int i = 0, j = 0; i < 800; i += 8, j++) {
                screen->clear();
                r2 = r1 + flag * j;
                screen->add_filled_circle(r2, (flag > 0) ? i : (800 - i), 260, 0xFF0000);
                screen->add_text(Utils::str_format("\r帧率: %.2lf : %.2lf ms    CPU:%ld%  RAM: %ldMB / %ldMB",
                                                   screen->current_frame_rate(),
                                                   screen->current_frame_time(), info.get_usage_cpu(),
                                                   info.get_usage_ram(), info.get_total_ram()), 10, 10, 22, 0x00FF00);
                screen->refresh();
            }
            flag = -flag;
            r1 = r2;
        }
    } catch (std::exception &e) {
        LOG(LOG_ERR, "%s:%s:%s:%d", __FILE__, __FUNCTION__, __LINE__, e.what());
        exit(0);
    }
    DisplayScreen::destroy();
    return 0;
}
