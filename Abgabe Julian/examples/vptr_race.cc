#include <thread>
#include <mutex>

class A
{
public:
    A() : done_(false) {}
    virtual void F() { printf("A::F\n"); }
    void Done()
    {
        std::unique_lock<std::mutex> lk(m_);
        done_ = true;
        cv_.notify_one();
    }
    virtual ~A()
    {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [this]
                 { return done_; });
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    bool done_;
};

class B : public A
{
public:
    virtual void F() { printf("B::F\n"); }
    virtual ~B() {}
};

int main()
{
    A *a = new B;
    std::thread t1([a]
                   {a->F(); a->Done(); });
    std::thread t2([a]
                   { delete a; });
    t1.join();
    t2.join();
}