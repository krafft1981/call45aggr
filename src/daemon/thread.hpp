#pragma once

#include <thread>
#include "systems/Spinlock.h"

class Thread {
public:
    enum Status { CREATED, WAITING, RUNNING, STOPPING, STOPPED };

    std::string StatusName(Thread::Status status);

    Thread(std::string name);
    Thread() {};
    virtual ~Thread();

    void start();
    void stop();
    void join();

    Thread::Status getStatus();

protected:
    virtual void run() = 0;
    virtual void onStart() {};
    virtual void onStop() {};
    virtual void onShutdown() {};

    mutable Spinlock lock_;

private:
    void threadFun();
    std::thread thread_;
    Thread::Status status_;
    std::string name_;
};
