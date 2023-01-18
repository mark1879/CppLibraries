#include "thread_pool.hpp"

int Thread::generate_id_ = 0;

Thread::Thread(ThreadFunc func)
: thread_func_(func)
, id_(generate_id_++)
{ }

Thread::~Thread()
{ }

void Thread::Start()
{
    std::thread t(thread_func_, id_);
    t.detach();
}

int Thread::get_id() const
{
    return id_;
}

const int kTaskNumThreshold = 10;
const int kThreadNumThreshold = 1024;
const int kThreadMaxIdleTime = 60; // second

ThreadPool::ThreadPool()
    : init_thread_num_(0)
    , thread_num_threshold_(kThreadNumThreshold)
    , current_thread_num_(0)
    , idle_thread_num_(0)
    , task_num_threshold_(kTaskNumThreshold)
    , mode_(PoolMode::MODE_FIXED)
    , is_running_(false)
{ }
    
ThreadPool::~ThreadPool()
{
    is_running_ = false;

    std::unique_lock<std::mutex> lock(task_queue_mutex_);
    queue_not_empty_.notify_all();
    exit_cond_var_.wait(lock, [&]()->bool { return threads_.size() == 0; });
}

void ThreadPool::set_mode(PoolMode mode) 
{
    if (is_running_.load())
        return;

    mode_ = mode;
}

void ThreadPool::set_task_num_threshold_(unsigned int threshold)
{
    if (is_running_.load())
        return;
    
    task_num_threshold_ = threshold;
}

void ThreadPool::set_thread_num_threshold_(unsigned int threshold)
{
    if (is_running_.load())
        return;

    if (mode_ == PoolMode::MODE_CACHED)
        thread_num_threshold_ = threshold;
}

void ThreadPool::Start(int init_thread_num)
{
    if (is_running_.load()) 
    {
        LOG_WARNING("%s, thread pool has been started!", __FUNCTION__);
        return;
    }

    LOG_INFO("%s, init thread num: %d", __FUNCTION__, init_thread_num);
    
    is_running_ = true;
    init_thread_num_ = init_thread_num;

    for (int i = 0; i < init_thread_num_; i++) 
    {
        auto thread = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
        threads_.emplace(thread->get_id(), std::move(thread));
    }

    for (int i = 0; i < init_thread_num_;i++)
    {
        threads_[i]->Start();
        ++idle_thread_num_;
    }
}

void ThreadPool::ThreadFunc(int thread_id)
{
    auto last_time = std::chrono::high_resolution_clock().now();

    for (;;)
    {
        Task task;
        {
            // lock the queue
            std::unique_lock<std::mutex> lock(task_queue_mutex_);

            LOG_INFO("%s, thread id: %d is trying to get a task...", __FUNCTION__, thread_id);

            while (task_queue_.size() == 0)
            {
                LOG_INFO("%s, thread id: %d no task!", __FUNCTION__, thread_id);

                if (!is_running_.load())
                {
                    threads_.erase(thread_id);
                    LOG_INFO("%s, thread id: %d exits!", __FUNCTION__, thread_id);
                    exit_cond_var_.notify_all();
                    return;
                }

                if (mode_ == PoolMode::MODE_CACHED)
                {
                    if (std::cv_status::timeout == 
                        queue_not_empty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        auto now = std::chrono::high_resolution_clock().now();
                        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_time);
                        
                        if (duration.count() >= kThreadMaxIdleTime
                            && current_thread_num_ > init_thread_num_)
                        {
                            // recycle thread
                            threads_.erase(thread_id);
                            --current_thread_num_;
                            --idle_thread_num_;

                            LOG_INFO("%s, thread id: %d has been recycled!", __FUNCTION__, thread_id);
                            return;
                        }
                    }
                }
                else
                {
                    LOG_INFO("%s, thread id: %d is waiting!", __FUNCTION__, thread_id);
                    queue_not_empty_.wait(lock);
                }
            }

            --idle_thread_num_;

            LOG_INFO("%s, thread id: %d get a task successfully!", __FUNCTION__, thread_id);

            task = task_queue_.front();
            task_queue_.pop();

            if (task_queue_.size() > 0)
            {
                queue_not_empty_.notify_all();
            }

            queue_not_full_.notify_all();

        }   // release lock

        if (task != nullptr)
        {   
            // excute the task
            task();
            std::this_thread::sleep_for(std::chrono::seconds(5));

        }

        ++idle_thread_num_;
        last_time = std::chrono::high_resolution_clock().now();
    }
}
