#pragma once

#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <algorithm>
#include <queue>

template <typename T>
class QueueFPS : public std::queue<T>
{
public:
    QueueFPS() : counter(0) {}

    void push(const T&& entry)
    {
        std::lock_guard<std::mutex> lock(mutex);

        std::queue<T>::push(std::move(entry));
        counter += 1;
        if (counter == 1)
        {
            // Start counting from a second frame (warmup).
            tm.reset();
            tm.start();
        }
        _cond.notify_one();
    }

    T get()
    {
        std::unique_lock<std::mutex> lock(mutex);
        _cond.wait(lock, [this] { return !this->empty();  });
        T entry = std::move(this->front());
        this->pop();
        return entry;
    }

    float getFPS()
    {
        tm.stop();
        double fps = counter / tm.getTimeSec();
        tm.start();
        return static_cast<float>(fps);
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (!this->empty())
            this->pop();
    }

    unsigned int counter;

private:
    cv::TickMeter tm;
    std::mutex mutex;
    std::condition_variable _cond;
};