#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> //epoll_ctl()
// #include <fcntl.h>  // fcntl()
// #include <unistd.h> // close()
// #include <assert.h> // close()
#include <vector>
// #include <errno.h>

class Epoller {
public:
    explicit Epoller(int maxEvent);
    Epoller(): Epoller(1024) {}
    Epoller(const Epoller&) = delete;
    ~Epoller();

    bool addFd(int fd, uint32_t events);
    bool modFd(int fd, uint32_t events);
    bool delFd(int fd);
    int wait(int timeoutMs);
    int getEventFd(size_t i) const;
    uint32_t getEvents(size_t i) const;

private:
    int epFd_;
    std::vector<struct epoll_event> vecEvents_;
};

#endif //EPOLLER_H