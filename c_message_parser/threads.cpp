#include <thread>
#include <condition_variable>
#include <mutex>
#include <pthread.h>

std::mutex m;
std::condition_variable cv;
bool your_turn = false;

void 
thread_body(const char* print, uint8_t &counter, bool (*conditional)(void))
{
    // Unique lock on shared mutex in global space
    std::unique_lock<std::mutex> lock(m);
    // Condition variable waits on boolean condition to return true
    cv.wait(lock, [conditional]{return conditional();});
    /* enter: critical section */
    printf("%s", print);
    // thread a's turn
    your_turn = !your_turn;
    ++counter;
    /* exit: critical section */
    // Release the lock on the mutex
    lock.unlock();
    // Notify any waiting threads
    cv.notify_one();
}

bool
your_turn_check(void)
{
    return your_turn;
}

bool
my_turn_check(void)
{
    return !your_turn;
}

void
thread_a(void)
{
    uint8_t counter = 0;
    while (counter < UINT8_MAX)
    {
        thread_body("a ", counter, my_turn_check);
    }
}

void 
thread_b(void)
{
    uint8_t counter = 0;
    while (counter < UINT8_MAX)
    {
        thread_body("b ", counter, your_turn_check);
    }
    printf("\n");
}

int
main(void)
{
    // Initialize threads
    std::thread a_runnable(thread_a);
    std::thread b_runnable(thread_b);
    sched_param s;

    // Set the sheduling priorities of each thread
    s.sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;   
    if (pthread_setschedparam(a_runnable.native_handle(), SCHED_FIFO, &s))
        perror("Failed to set the priority of thread a!\n");
    s.sched_priority = sched_get_priority_min(SCHED_FIFO);   
    if (pthread_setschedparam(b_runnable.native_handle(), SCHED_FIFO, &s))
        perror("Failed to set the priority of thread b!\n");

    // Join back a & b threads
    a_runnable.join();
    b_runnable.join();
}