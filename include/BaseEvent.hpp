/*
 * Created by Administrator on 2023/8/26.
 */

#ifndef GREEDYSNAKE_BASEEVENT_HPP
#define GREEDYSNAKE_BASEEVENT_HPP

#include <memory>
#include <tslib.h>


namespace Event {
    class BaseEvent {
    private:
        static std::shared_ptr <BaseEvent> _create;
        struct tsdev *ts_dev;

    protected:
        BaseEvent();

    public:
        virtual ~BaseEvent();

        static std::shared_ptr <BaseEvent> &create();

        void get_event(struct ts_sample &sample);

        static void destroy();
    };

    __attribute__((unused)) static std::shared_ptr <BaseEvent> event = BaseEvent::create();
}


#endif //GREEDYSNAKE_BASEEVENT_HPP
