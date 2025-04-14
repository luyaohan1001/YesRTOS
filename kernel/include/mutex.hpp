namespace YesRTOS {

class mutex {
  public:
    typedef enum {
      OP_SUCCESS,
      OP_FAIL
    } mutex_status_t;

    mutex();
    ~mutex();
    mutex_status_t try_lock();
    mutex_status_t try_unlock();

  private:
    bool atomic_val;
};

}
