#include <atomic>
#include "thread.hpp"
#include "baremetal_api.h"
#include "preempt_fifo_scheduler.hpp"

namespace YesRTOS {

class mutex {
    public:
    mutex();
    ~mutex();

    void lock();
    void unlock();


    std::atomic<bool> locked;
    YesRTOS::Thread* p_blocked_list;
    YesRTOS::Thread* owner;
};

}
