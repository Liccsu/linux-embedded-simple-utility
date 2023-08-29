/*
 * Created by Administrator on 2023/8/26.
 */

#include <thread>
#include <utility>
#include "Button.hpp"


Button::Button(int32_t _x1, int32_t _y1, int32_t _x2, int32_t _y2, uint32_t _color, int32_t _rad,
               const TextView &_textView) : click_callback([] {}),
                                            x1_(_x1), y1_(_y1), x2_(_x2), y2_(_y2), color_(_color), rad_(_rad),
                                            textView_(_textView) {
    this->x1 = _x1;
    this->y1 = _y1;
    this->x2 = _x2;
    this->y2 = _y2;
    this->color = _color;
    this->rad = _rad;
    this->textView = _textView;

    /* 启动监听线程 */
    std::thread t([this] {
        struct ts_sample sample{};
        uint32_t pressure = 0;
        bool press_inside = false;
        while (true) {
            /* 循环监听点击 */
            Event::event->get_event(sample);
            if (sample.pressure) {
                /* 按下 */
                if (pressure == 0) {
                    if (sample.x >= this->x1 && sample.x <= this->x2 && sample.y >= this->y1 && sample.y <= this->y2) {
                        if (!(((sample.x < this->x1 + rad && sample.y < this->y1 + rad) &&
                               (Utils::get_distance<int32_t>(this->x1 + rad, this->y1 + rad, sample.x, sample.y) >
                                rad)) || /* 左上角 */
                              ((sample.x > this->x2 - rad && sample.y < this->y1 + rad) &&
                               (Utils::get_distance<int32_t>(this->x2 - rad, this->y1 + rad, sample.x, sample.y) >
                                rad)) || /* 右上角 */
                              ((sample.x < this->x1 + rad && sample.y > this->y2 - rad) &&
                               (Utils::get_distance<int32_t>(this->x1 + rad, this->y2 - rad, sample.x, sample.y) >
                                rad)) || /* 左下角 */
                              ((sample.x > this->x2 - rad && sample.y > this->y2 + rad) &&
                               (Utils::get_distance<int32_t>(this->x2 - rad, this->y2 - rad, sample.x, sample.y) >
                                rad)))) { /* 右下角 */
                            press_inside = true;
                            this->onPress();
                        }
                    }
                }
            } else {
                /* 抬起 */
                if (press_inside) {
                    if (sample.x >= this->x1 && sample.x <= this->x2 && sample.y >= this->y1 && sample.y <= this->y2) {
                        if (!(((sample.x < this->x1 + rad && sample.y < this->y1 + rad) &&
                               (Utils::get_distance<int32_t>(this->x1 + rad, this->y1 + rad, sample.x, sample.y) >
                                rad)) || /* 左上角 */
                              ((sample.x > this->x2 - rad && sample.y < this->y1 + rad) &&
                               (Utils::get_distance<int32_t>(this->x2 - rad, this->y1 + rad, sample.x, sample.y) >
                                rad)) || /* 右上角 */
                              ((sample.x < this->x1 + rad && sample.y > this->y2 - rad) &&
                               (Utils::get_distance<int32_t>(this->x1 + rad, this->y2 - rad, sample.x, sample.y) >
                                rad)) || /* 左下角 */
                              ((sample.x > this->x2 - rad && sample.y > this->y2 - rad) &&
                               (Utils::get_distance<int32_t>(this->x2 - rad, this->y2 - rad, sample.x, sample.y) >
                                rad)))) { /* 右下角 */
                            this->onClick();
                            this->onRelease();
                        }
                    }
                    press_inside = false;
                }
            }
            pressure = sample.pressure;
        }
    });
    t.detach();
}

void Button::setOnClickListener(std::function<void()> _callback) {
    this->click_callback = std::move(_callback);
}

void Button::onClick() {
    this->click_callback();
}

void Button::setOnPressListener(std::function<void()> _callback) {
    this->press_callback = std::move(_callback);
}

void Button::onPress() {
    this->press_callback();
}

void Button::setOnReleaseListener(std::function<void()> _callback) {
    this->release_callback = std::move(_callback);
}

void Button::onRelease() {
    this->release_callback();
}

const int32_t &Button::get_x1() const {
    return x1_;
}

const int32_t &Button::get_y1() const {
    return y1_;
}

const int32_t &Button::get_x2() const {
    return x2_;
}

const int32_t &Button::get_y2() const {
    return y2_;
}

const int32_t &Button::get_rad() const {
    return rad_;
}

const uint32_t &Button::get_color() const {
    return color_;
}

const TextView &Button::get_textView() const {
    return textView_;
}
