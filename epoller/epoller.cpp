#include "epoller.h"

#include <assert.h>
#include <unistd.h>

Epoller::Epoller(int maxEvent = 1024) : epFd_(epoll_create1(0)), vecEvents_(maxEvent) {
    assert(epFd_ >= 0);
}

Epoller::~Epoller() { close(epFd_); }

bool Epoller::addFd(int fd, uint32_t events) {
    epoll_event epEvt;
    epEvt.data.fd = fd;
    epEvt.events = events;
    return 0 == epoll_ctl(epFd_, EPOLL_CTL_ADD, fd, &epEvt);
}

bool Epoller::modFd(int fd, uint32_t events) {
    epoll_event epEvt;
    epEvt.data.fd = fd;
    epEvt.events = events;
    return 0 == epoll_ctl(epFd_, EPOLL_CTL_MOD, fd, &epEvt);
}

bool Epoller::delFd(int fd) {
    epoll_event ev = {0};
    return 0 == epoll_ctl(epFd_, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::wait(int timeout = -1) {
    return epoll_wait(epFd_, &vecEvents_[0], vecEvents_.size(), timeout);
}

int Epoller::getEventFd(size_t i) const {
    assert(i >= 0 && i < vecEvents_.size());
    return vecEvents_[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const {
    assert(i >= 0 && i < vecEvents_.size());
    return vecEvents_[i].events;
}
