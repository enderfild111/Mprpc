#include <sys/socket.h>
#include <unistd.h>
class unique_fd
{
public:
    explicit unique_fd(int fd = -1) : fd_(fd) {}

    ~unique_fd(){reset();}

    int get() const noexcept { return fd_; }

    unique_fd(const unique_fd&) = delete;
    unique_fd& operator=(const unique_fd&) = delete;

    unique_fd(unique_fd&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    unique_fd& operator=(unique_fd&& other) noexcept
    {
        if(this != &other)
        {
            reset();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
    explicit operator bool() const noexcept { return fd_ != -1; }
    
    int release() noexcept
    {
        int old_fd = fd_;
        fd_ = -1;
        return old_fd;
    }
private:
    int fd_;
    void reset() noexcept
    {
        if(fd_ != -1)
        {
            close(fd_);
            fd_ = -1;
        }
    }
};