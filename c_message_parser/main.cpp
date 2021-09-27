#include <stdio.h>

#include <pthread.h>
#include <semaphore.h>

#include <queue>
#include <vector>

#include <sys/socket.h>

#include  <sys/types.h> 
#include <stdint.h>
#include <unistd.h>
#include <functional>

#include <thread>

#include <mutex>

#define NUM_THREADS 4

using namespace std;

class Chassis
{
public:

    enum Priority
    {
        CST = 1,
        POST_IBIT = 10,
        PHM_RECV = 20,
        IPMC_SEND = 30,
        IPMC_RECV = 30,
    };

    Chassis()
    {
        // Create the chassis
    };

    // Model a request and a response in the same structure
    struct Message
    {
        Message(uint8_t i)
        {
            message = i;
        };
        uint8_t message;
    };


    static bool initialize(Chassis* self)
    {
        
        int ret;
        // Open interfaces
        
        // Setup semaphores
        sem_init(&self->ipmc_done, 0, 0);
        sem_init(&self->signal_send, 0, 0);

        // Setup IPMC mutex
        pthread_mutexattr_t attr;
        ret = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
        printf("pthread_mutexattr_setprotocol %i\n", ret);
        ret = pthread_mutex_init(&self->ipmc_in_use, &attr);
        printf("pthread_mutex_init %i\n", ret);

        // Setup threads
        int rc;
        sched_param s;


        self->postThreadHandle = std::thread(&Chassis::postThread, self);
        s.sched_priority = sched_get_priority_min(SCHED_FIFO) + POST_IBIT;
        
        pthread_setschedparam(self->postThreadHandle.native_handle(), SCHED_FIFO, &s);
        
        self->cstThreadHandle = std::thread(&Chassis::cstThread, self);
        s.sched_priority = sched_get_priority_min(SCHED_FIFO) + CST;
        pthread_setschedparam(self->cstThreadHandle.native_handle(), SCHED_FIFO, &s);
        
        self->phmThreadHandle = std::thread(&Chassis::phmThread, self);
        s.sched_priority = sched_get_priority_min(SCHED_FIFO) + PHM_RECV;
        pthread_setschedparam(self->phmThreadHandle.native_handle(), SCHED_FIFO, &s);   
                
        self->recvThreadHandle = std::thread(&Chassis::recvThread, self);
        s.sched_priority = sched_get_priority_min(SCHED_FIFO) + IPMC_RECV;
        pthread_setschedparam(self->recvThreadHandle.native_handle(), SCHED_FIFO, &s);   
        
        self->sendThreadHandle = std::thread(&Chassis::sendThread, self);
        s.sched_priority = sched_get_priority_min(SCHED_FIFO) + IPMC_SEND;
        pthread_setschedparam(self->sendThreadHandle.native_handle(), SCHED_FIFO, &s);   
        
        // Start threads
        self->recvThreadHandle.join();
        self->sendThreadHandle.join();
        self->postThreadHandle.join();
        self->cstThreadHandle.join();
        self->phmThreadHandle.join();
        
        return true;
    };

    void phmThread()
    {
        printf("phmThread enter\n");
        while (true)
        {
            // block on phm message
            sleep(10);
            printf("PHM thread higher priority Every ten! I have a ton of shit to deque!\n");
            printf("PHM thread waiting for the ipmc\n");
            int ret = pthread_mutex_lock(&ipmc_in_use);
            printf("PHM thread has the IPMC, lock returned %i\n", ret);
            printf("PHM on Thread sent something\n");

            for (int i = 0; i < 5; i++)
            {
                printf("doing tranaction %i/5\n", i+1);
                vector<uint8_t> sendMsg = {5, 10, 15};
                sendQueueLock.lock();
                printf("PHM writing message\n");
                sendThreadQ.push(sendMsg); // <- add CV to notify / wake up send Thread
                sendQueueLock.unlock();
                printf("PHM signaling SEND\n");
                sem_post(&signal_send);
                sem_wait(&ipmc_done);
            }
            printf("PHM thread dropping IPMC\n");
            pthread_mutex_unlock(&ipmc_in_use);
            sched_yield();
        }
        printf("phmThread exit\n");
    };

    void postThread()
    {
        // Hold thread state variable here
        // What module we are iterating on, where we are at.
        
        printf("POST IBIT enter\n");
        // Post ibit tings below  (that only occur once , etc)
        // Create a LUT of all modules and their respective messages. Serialization might still need to be dynamic, including the A/B swap, or generate those too

        // map< Slot , Message > mode/bitresults InterfaceA memory reads
        // map< Slot , Message > mode/bitresults InterfaceB memory reads
        // map< Slot , Message > diganosticMessages memory reads
        // map< Slot , Message > powerRecycleMessages
        // map< Slot , Message > SetIplMessages


        printf("POST Thread\n");
        printf("POST thread waiting for the ipmc\n");
        int ret = pthread_mutex_lock(&ipmc_in_use);
        printf("POST thread has the IPMC, lock returned %i\n", ret);

        for (int i = 0; i < 10; i++)
        {
            vector<uint8_t> sendMsg = {2, 4, 6};
            sendQueueLock.lock();
            printf("POST writing message %i/10\n", i+1);
            sendThreadQ.push(sendMsg); // <- add CV to notify / wake up send Thread
            sendQueueLock.unlock();
            printf("POST signaling SEND\n");
            sem_post(&signal_send);

            // Wake send thread up via notify or optionally have it poll sendQ every X ms 
            // Priority Invert here. Increase priority until we get a response
            printf("POST Thread blocking until ipmc is done\n");
            sem_wait(&ipmc_done); // Wait here until we get a timeout or an actual response
            printf("POST Ipmc was done recv, message queue ready to be processed\n");
            // Revert Priority to normal
            // yield CPU to let high priority thread run. If no other high pri, then this will resume control
            // CST message processing
            // Select next module, generate a message, etc
        }
        pthread_mutex_unlock(&ipmc_in_use); // drop the mutex so another (hi) priority thread can use the ipmc to send
        printf("POST Thread done using ipmc transaction\n");
        sched_yield();
        
        printf("POST IBIT exit\n");
    };

    void cstThread()
    {
        // Hold thread state variable here
        // What module we are iterating on, where we are at.
        
        printf("CST MODE enter\n");
        // Post ibit tings below  (that only occur once , etc)
        // Create a LUT of all modules and their respective messages. Serialization might still need to be dynamic, including the A/B swap, or generate those too

        // map< Slot , Message > mode/bitresults InterfaceA memory reads
        // map< Slot , Message > mode/bitresults InterfaceB memory reads
        // map< Slot , Message > diganosticMessages memory reads
        // map< Slot , Message > powerRecycleMessages
        // map< Slot , Message > SetIplMessages

        while (true)
        {
            
            for (int i = 0; i < 10; i++)
            {
                printf("CST thread waiting for the ipmc\n");
                int ret = pthread_mutex_lock(&ipmc_in_use);
                printf("CST thread has the IPMC, lock returned %i\n", ret);
                vector<uint8_t> sendMsg = {0, 1, 2};
                printf("CST waiting to write message");
                sendQueueLock.lock();
                printf("CST writing message %i/10\n", i+1);
                sendThreadQ.push(sendMsg); // <- add CV to notify / wake up send Thread
                sendQueueLock.unlock();
                printf("CST Thread signaled send\n");
                sem_post(&signal_send);
                // Wake send thread up via notify or optionally have it poll sendQ every X ms 
                // Priority Invert here. Increase priority until we get a response
                printf("CST Thread blocking until ipmc is done\n");
                sem_wait(&ipmc_done); // Wait here until we get a timeout or an actual response
                printf("CST Ipmc was done recv, message queue ready to be processed\n");
                pthread_mutex_unlock(&ipmc_in_use); // drop the mutex so another (hi) priority thread can use the ipmc to send
                printf("CST Thread done using ipmc transaction\n");
                sched_yield();
                // Revert Priority to normal
                // yield CPU to let high priority thread run. If no other high pri, then this will resume control
                // CST message processing
                // Select next module, generate a message, etc
            }
        }
        printf("CST MODE exit\n");

    };

    void recvThread()
    {
        // Attempt connection
        while (true)
        {
            
            // Push some random shit to the queue
            vector<uint8_t> bytes;
            bytes.push_back(0x01);
            bytes.push_back(0x02);
            bytes.push_back(0x03);
            sleep(1); // forever loop
            
            // while (recvThreadQ.size() < 5)
            // {
            //     recvQueueLock.lock();
            //     recvThreadQ.push(bytes);
            //     recvQueueLock.unlock();
            //     sleep(1); // Every 1 second , send some shit, regardless of sender lol.
            // }
            // recv message <-- Virtual method, interface specific operation should be defined
            // Block until response
            // Do some serial parsing to route the message to correct queue: (for example, data obj, subtype)
        }
    };

    // This thread is gaurented to finish and also unlock anyone holding ipmc_done.
    void sendThread()
    {
        while (true)
        {
            
            // Wake up (can be woken with condition variable by any thread that adds things to the queue.
            //  queue queue queue , notify. wait(ipmc_done).
            // dequeu message,
            // send message <-- Virutual method, interface specific operation should be defined
            // Mesage should contain info such as: originating thread, timeout value, retry count if retries specified
            printf("SEND waiting for wake signal\n");
            sem_wait(&signal_send);
            printf("SEND I was woken!\n");

            // Dequeue one, send it
            sendQueueLock.lock();
            if (sendThreadQ.size() > 0)
            {
                printf("the queue size is %i\n", sendThreadQ.size());
                auto msg = sendThreadQ.front();
                if (msg[0] == 0) { printf("SEND: CST wants to send a message\n");}
                if (msg[0] == 2) { printf("SEND: POST wants to send a message\n");}
                send(msg);
                sendThreadQ.pop();
                printf("the queue size after pop is %i\n", sendThreadQ.size());
                // Wait timeout ms
                printf("SEND NOTHING ELSE SHOULD WRITE TO ME\n");
                sleep(5);
                printf("SEND DONE SLEEPING\n");
                // On wakeup, check a recv queue.
                recvQueueLock.lock();
                if (recvThreadQ.size() > 0)
                {
                    printf("SEND got a message or we timed out!, set message buffer for appropriate thread\n");
                }
                recvQueueLock.unlock();
                printf("SEND I'm done!\n");
                sem_post(&ipmc_done);
            }
            sendQueueLock.unlock();
            // check recv queue for response
            // if response not in here, or other responses exist (unsoliticted), set that response timed out flag
            // Update request object with the response data (marshalling can be done by the other threads)

            // signal to thread that we have finished and some other thread can take this lock
        }
    };


    // fill priority based queue // send thread sends per quefilled // responses are enqueued in the respective queues. Callign threads responsible for csy

    int recv(vector<uint8_t>& serialized)
    {
        // Read raw interface
        // Populate response vector
        // Set return code
        return -1;
    }

    int send(vector<uint8_t>& serialized)
    {
        // serialize it here? Could be a method of Message class
        // send it across the interface
        // Set return code
        return -1;
    }


    // Threads
    std::thread phmThreadHandle;
    std::thread sendThreadHandle;
    std::thread recvThreadHandle;
    std::thread postThreadHandle;
    std::thread cstThreadHandle;

    // Message queues
    std::queue< vector<uint8_t> > sendThreadQ; // Queue of outgoing Message objects (prior to serialization)
    std::queue< vector<uint8_t> > recvThreadQ; // Queue of incoming serialized bytes
    std::queue< vector<uint8_t> > fldRecvQ; // Queue of incoming serialized bytes

    // Thread sync
    pthread_mutex_t ipmc_in_use;
    sem_t ipmc_done;
    sem_t signal_send;

    // Mutexes
    std::mutex recvQueueLock;
    std::mutex sendQueueLock;
};

int main() 
{
    Chassis chassis;
    Chassis::initialize(&chassis); // Boot strap
    printf("exit-main\n");
    return 0;
}