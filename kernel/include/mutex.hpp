#include <atomic>

class mutex {
    mutex();
    ~mutex();

    void lock();
    void unlock();


    std::atomic<bool> locked;
};
