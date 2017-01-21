//
// Created by lz on 1/21/17.
//

#ifndef C10K_SERVER_EVENT_LOOP_HPP
#define C10K_SERVER_EVENT_LOOP_HPP

#endif //C10K_SERVER_EVENT_LOOP_HPP

#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <cstdlib>
#include <unordered_map>
#include <functional>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <mutex>
#include "utils.hpp"

namespace c10k
{

    enum struct EventCategory
    {
        POLLIN = EPOLLIN,
        POLLOUT = EPOLLOUT
    };

    struct EventType
    {
    private:
        int event_type_;
    public:
        explicit EventType(int et):
                event_type_(et)
        {}

        explicit EventType(EventCategory ec):
                EventType((int)ec)
        {}

        bool is(EventCategory ec) const
        {
            return event_type_ & (int)ec;
        }

        void set(EventCategory ec)
        {
            event_type_ |= (int) ec;
        }

        void unset(EventCategory ec)
        {
            event_type_ &= ~ (int)ec;
        }

        explicit operator int() const
        {
            return event_type_;
        }

        template<typename OST>
                friend OST &operator <<(OST &o, EventType et)
        {
            o << "[";
            if (et.is(EventCategory::POLLIN))
                o << " POLLIN ";
            if (et.is(EventCategory::POLLOUT))
                o << " POLLOUT ";
            o << "]";
            return o;
        }
    };

    class EventLoop;
    struct Event
    {
        EventLoop *event_loop;
        int fd;
        EventType event_type;

        template<typename OST>
                friend OST &operator <<(OST &o, const Event &e)
        {
            o << "Eventloop=" << (void*)e.event_loop << ", " <<
                   "fd=" << e.fd << ", " <<
                                       "event_type=" << e.event_type;
            return o;
        }
    };

    using EventHandler = std::function<void(const Event &)>;
    inline void NullEventHandler(const Event &) {}

    namespace detail
    {
        struct PollData
        {
            int fd;
            EventHandler handler;

            PollData(int fd, EventHandler eh):
                    fd(fd), handler(std::move(eh))
            {}
        };
    }

    // thread-safe Eventloop run in each thread
    class EventLoop
    {
    private:
        int epollfd;
        bool loop_enabled_ = true;
        bool in_loop_ = false;
        std::shared_ptr<spdlog::logger> logger;

        std::unordered_map<int, std::unique_ptr<detail::PollData>> fd_to_poll_data;
        std::mutex map_mutex;
        using LoggerT = decltype(logger);

        // workaround before inline var :)
        static constexpr int epoll_event_buf_size()
        {
            return 1024;
        }

        void handle_events(epoll_event *st, epoll_event *ed);

    public:
        EventLoop(int max_event, const LoggerT &logger = spdlog::stdout_color_mt("Eventloop"));
        EventLoop(const EventLoop &) = delete;
        EventLoop &operator=(const EventLoop &) = delete;

        void loop();

        void add_event(int fd, EventType et, EventHandler handler);

        void remove_event(int fd);

        void modify_event(int fd, EventType et, EventHandler handler);

        // whether in loop
        bool in_loop() const
        {
            return in_loop_;
        }

        // whether loop is enabled
        bool loop_enabled() const
        {
            return loop_enabled_;
        }

        bool enable_loop()
        {
            loop_enabled_ = true;
        }

        bool disable_loop()
        {
            loop_enabled_ = false;
        }

    };
}