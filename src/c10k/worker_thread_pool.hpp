//
// Created by lz on 2/9/17.
//

#ifndef C10K_SERVER_WORKER_THREAD_POOL_HPP
#define C10K_SERVER_WORKER_THREAD_POOL_HPP

#include "round_robin_pool.hpp"
#include "worker_thread.hpp"

namespace c10k
{
    namespace detail {
        using WorkerThreadPool = detail::RoundRobinPool;
    }
}

#endif //C10K_SERVER_WORKER_THREAD_POOL_HPP
