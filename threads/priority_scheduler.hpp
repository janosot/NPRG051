#ifndef PRIORITY_SCHEDULER_HPP
#define PRIORITY_SCHEDULER_HPP

#include <limits>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <semaphore>
#include <vector>
#include <queue>
#include <cassert>

constexpr size_t CACHE_LINE_SIZE = 64;

/**
 * @brief The main scheduler class
 * 
 * @tparam VTLS_T virtual thread local storage type. Must be default constructable.
 */
template<typename VTLS_T>
class scheduler {
public:
    /**
     * @brief Virtual thread ID
     * 
     */
    using vthread_id_t = size_t;
    
    /**
     * @brief An invalid virtual thread ID
     * 
     */
    static constexpr size_t INVALID_VTHREAD_ID = std::numeric_limits<vthread_id_t>::max();
    
    /**
     * @brief A class holding information about a virtual thread
     */
    class alignas(CACHE_LINE_SIZE) vthread_info {
    public:
        /**
         * @brief Returns the virtual thread ID
         */
        vthread_id_t id() const { return id_; }

        /**
         * @brief Returns a thread local data of the virtual thread 
         */
        VTLS_T &data() { return data_; }

        /* 
        * @brief vthread_info constructor
        */
        vthread_info(vthread_id_t id) : id_(id) {}

    private:
        vthread_id_t id_;
        VTLS_T data_;
    };

    /**
     * @brief A simple unit executed atop of the virtual threads
     */
    class task {
    public:
        virtual ~task() {}

        /**
         * @brief A function implementing the main body of the task. Each task executes this function and terminates
         * 
         * @param s A scheduler invoking this task
         * @param info Information about the virtual thread executing the task
         */
        virtual void run(scheduler &s, vthread_info &info) = 0;
    };

    /**
     * @brief Construct a new scheduler object
     * 
     * @param num_threads Number of real threads
     * @param time_to_idle_ms Time in milliseconds before a thread goes to sleep
     */
    scheduler(size_t num_threads, size_t time_to_idle_ms)
        : thread_count_(num_threads), max_idle_(time_to_idle_ms)
    {
        vthreads_.reserve(num_threads);
        for (int64_t i = num_threads - 1; i >= 0; i--)
        {
            vthreads_.push_back(std::make_unique<vthread_info>(i));
            vthreads_semaphore_.release();
        }
        for (size_t i = 0; i < num_threads; i++)
        {
            init_thread();
        }
    }

    /**
     * @brief Destruct the scheduler object
     */
    ~scheduler()
    {
        exit_semaphore_.acquire();
    }

    /**
     * @brief Add a new task into the scheduler's queue
     * @note Can be called in parallel
     * 
     * @param task Task to be added
     */
    void add_task(std::unique_ptr<task> &&t)
    {
        // lock
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        tasks_.push(std::move(t));
    }

private:
    const size_t thread_count_;
    const size_t max_idle_;

    std::atomic<size_t> active_threads_ = 0;
    std::atomic<size_t> idle_threads_ = 0;

    std::vector<std::unique_ptr<vthread_info>> vthreads_;
    std::mutex vthreads_mutex_;

    std::queue<std::unique_ptr<task>> tasks_;
    std::mutex tasks_mutex_;

    std::counting_semaphore<> vthreads_semaphore_{0};
    std::binary_semaphore exit_semaphore_{0};

    void init_thread()
    {
        std::thread([this]()
        {
            active_threads_++;
            idle_threads_++;

            std::this_thread::sleep_for(std::chrono::milliseconds(max_idle_));

            std::unique_ptr<task> current_task;
            std::unique_ptr<vthread_info> current_vthread;

            // Main loop
            auto last_time = std::chrono::high_resolution_clock::now();
            while (std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - last_time).count() <= max_idle_)
            {
                // Get a new task
                {
                    std::lock_guard<std::mutex> lock(tasks_mutex_);
                    if (!tasks_.empty() && !current_task)
                    {
                        current_task = std::move(tasks_.front());
                        tasks_.pop();
                        idle_threads_--;
                    }
                }
                // Get a new vthread
                {
                    if (!vthreads_semaphore_.try_acquire())
                    {
                        continue;
                    }
                    std::lock_guard<std::mutex> lock(vthreads_mutex_);
                    current_vthread = std::move(vthreads_.back());
                    vthreads_.pop_back();
                }
                // Execute the task
                if (current_task)
                {
                    current_task->run(*this, *current_vthread);
                    current_task = nullptr;
                    last_time = std::chrono::high_resolution_clock::now();
                    idle_threads_++;
                }
                // Put the vthread back into the pool
                {
                    std::lock_guard<std::mutex> lock(vthreads_mutex_);
                    vthreads_.insert(
                        std::upper_bound(vthreads_.begin(), vthreads_.end(), current_vthread),
                        std::move(current_vthread));
                    vthreads_semaphore_.release();
                }
            }
            active_threads_--;
            if (active_threads_ == 0)
            {
                exit_semaphore_.release();
            }
        }).detach();
    }
};

#endif 