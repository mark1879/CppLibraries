#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP
#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <thread>
#include <future>
#include <chrono>
#include "logger.hpp"

// 线程类型
class Thread
{
public:
	using ThreadFunc = std::function<void(int)>;

	Thread(ThreadFunc func);
	~Thread();

	void Start();
	int get_id() const;

private:
	ThreadFunc thread_func_;
	static int generate_id_;
	int id_;
};


enum class PoolMode
{
	MODE_FIXED,  // 固定数量的线程
	MODE_CACHED, // 线程数量可动态增长
};


class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	void set_mode(PoolMode mode);
    void set_task_num_threshold_(unsigned int threshold);
    void set_thread_num_threshold_(unsigned int threshold);

	void Start(int init_thread_num = std::thread::hardware_concurrency());

    template<typename Func, typename... Args>
	auto SubmitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
	{
		LOG_INFO("%s, submit task", __FUNCTION__);
		
		using RType = decltype(func(args...));
		auto task = std::make_shared<std::packaged_task<RType()>>(
			std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
		std::future<RType> result = task->get_future();

		{
			std::unique_lock<std::mutex> lock(task_queue_mutex_);

			LOG_INFO("%s, submit in", __FUNCTION__);

			if (!queue_not_full_.wait_for(lock, std::chrono::seconds(1),
				[&]()->bool { return task_queue_.size() < task_num_threshold_; }))
			{
				LOG_ERROR("%s, task queue has been full, submit task failed!", __FUNCTION__);

				auto task = std::make_shared<std::packaged_task<RType()>>(
					[]()->RType { return RType(); });
				(*task)();
				return task->get_future();
			}
		
			task_queue_.emplace([task](){(*task)();});

			queue_not_empty_.notify_all();

			if (mode_ == PoolMode::MODE_CACHED 
				&& task_queue_.size() > idle_thread_num_
				&& current_thread_num_ < thread_num_threshold_)
			{
				LOG_INFO("%s, create new thread!", __FUNCTION__);

				auto thread = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
				int thread_id = thread->get_id();
				threads_.emplace(thread_id, std::move(thread));

				threads_[thread_id]->Start();
				++current_thread_num_;
				++idle_thread_num_;
			}

			LOG_INFO("%s, submit done!", __FUNCTION__);
			LOG_INFO("%s, queue size: %d", __FUNCTION__, (int)task_queue_.size());
		}
		
		return result;
    }

private:
	void ThreadFunc(int thread_id);

private:
	std::unordered_map<int, std::unique_ptr<Thread>> threads_;

	int init_thread_num_;
	int thread_num_threshold_;
	std::atomic_int current_thread_num_;
	std::atomic_int idle_thread_num_;

    using Task = std::function<void()>;
	std::queue<Task> task_queue_;
	unsigned int task_num_threshold_;

	std::mutex task_queue_mutex_; 
	std::condition_variable queue_not_full_;
	std::condition_variable queue_not_empty_;
	std::condition_variable exit_cond_var_;

	PoolMode mode_;
	std::atomic_bool is_running_;
};



#endif