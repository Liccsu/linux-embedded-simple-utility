/*
 * Created by Administrator on 2023/8/26.
 */

#include <stdexcept>
#include <memory>
#include <tslib.h>
#include "BaseEvent.hpp"
#include "LOG.hpp"

Event::BaseEvent::BaseEvent() : ts_dev() {
    atexit(Event::BaseEvent::destroy);
    this->ts_dev = ts_setup(nullptr, 0);
    if (!this->ts_dev) {
        throw std::runtime_error("ts_setup error");
    }
}

Event::BaseEvent::~BaseEvent() {
    LOG(LOG_WARN, "%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    ts_close(this->ts_dev);
    Event::BaseEvent::_create = nullptr;
}

std::shared_ptr <Event::BaseEvent> Event::BaseEvent::_create = nullptr;

std::shared_ptr <Event::BaseEvent> &Event::BaseEvent::create() {
    if (_create == nullptr) {
        _create = std::shared_ptr<BaseEvent>(new BaseEvent());
    }

    return _create;
}

void Event::BaseEvent::get_event(ts_sample &sample) {
    if (ts_read(this->ts_dev, &sample, 1) < 0) {
        throw std::runtime_error("ts_read error");
    }
}

void Event::BaseEvent::destroy() {
    delete _create.get();
}
