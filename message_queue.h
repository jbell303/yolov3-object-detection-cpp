// Message Queue

#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <algorithm>
#include <queue>

template<class T>
class MessageQueue
{
public:
	MessageQueue() {}

	T receive()
	{
		// perform vector modification under the lock
		std::unique_lock<std::mutex> uLock(_mutex);

		// block until there are messages in the queue
		_cond.wait(uLock, [this] { return !_messages.empty();  });

		// remove the last vector element from queue
		T message = std::move(_messages.front());
		_messages.pop_front();

		return message;
	}

	void send(T&& message)
	{
		// perform vector modification under the lock
		std::lock_guard<std::mutex> uLock(_mutex);

		// add vector to the queue
		_messages.push_back(std::move(message));
		_cond.notify_one();
	}

	bool empty()
	{
		return _messages.empty();
	}

	void clear()
	{
		std::lock_guard<std::mutex> uLock(_mutex);
		while (!_messages.empty())
			_messages.pop_back();
	}

private:
	std::mutex _mutex;
	std::condition_variable _cond;
	std::deque<T> _messages;
};
