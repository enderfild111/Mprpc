#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <optional>
template<typename T>
class SafeQueue : public UnableCopy
{
public:
    SafeQueue() = default;
    ~SafeQueue() = default;
    void push(const T& msg)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(msg);  
        }
        cond_var_.notify_one();
    }
    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock,[this]{return !queue_.empty();});
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
    // 尝试非阻塞拿出队头元素
    std::optional<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(queue_.empty())
        {
            return std::nullopt;
        }
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool Empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
};
