#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include "daemon/daemon.hpp"
#include "thread.hpp"
#include <exception>

Thread::Thread(std::string name)
    :   name_(name),
        status_(CREATED)
{}

Thread::~Thread()
{
    stop();
    onShutdown();
}

void Thread::start() {
    thread_ = std::thread(&Thread::threadFun, this);
}

void Thread::threadFun() {

    status_ = WAITING;
    onStart();

    status_ = RUNNING;
    run();

    status_ = STOPPING;
    onStop();

    status_ = STOPPED;
}

void Thread::stop() {

    int status = getStatus();
    if (status == RUNNING) {
        status_ = STOPPING;
        join();
    }
}

void Thread::join() {
    if (status_ == (RUNNING|STOPPING)) {
        thread_.join();
    }
}

Thread::Status Thread::getStatus() {

    return status_;
}

std::string Thread::StatusName(Thread::Status status) {

    switch (status) {
		case CREATED:	return "CREATED";
		case WAITING:   return "WAITING";
		case RUNNING:   return "RUNNING";
		case STOPPED:   return "STOPPED";
        case STOPPING:  return "STOPPING";
    }
}
