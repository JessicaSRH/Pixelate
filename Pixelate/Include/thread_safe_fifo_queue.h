#pragma once

namespace Pixelate
{
	template <typename T>
	class ThreadSafeFifoQueue
	{
	public:
		ThreadSafeFifoQueue() = default;
		~ThreadSafeFifoQueue() = default;

		void push(T value)
		{
			std::lock_guard<std::mutex> lock(mtx);
			q.push(std::move(value));
			cv.notify_one();
		}

		void push_range(std::vector<T>& values)
		{
			std::lock_guard<std::mutex> lock(mtx);

			for(T value : values)
				q.push(std::move(value));

			cv.notify_all();
		}

		bool try_pop(T& value)
		{
			std::lock_guard<std::mutex> lock(mtx);
			if (q.empty())
			{
				return false;
			}
			value = std::move(q.front());
			q.pop();
			return true;
		}

		T pop()
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [this] { return !q.empty(); });
			T value = std::move(q.front());
			q.pop();
			return value;
		}

		bool empty() const
		{
			std::lock_guard<std::mutex> lock(mtx);
			return q.empty();
		}

	private:
		mutable std::mutex mtx;
		std::queue<T> q;
		std::condition_variable cv;
	};
}