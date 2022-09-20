#include <iostream>
#include <stack>
#include <shared_mutex>
#include <random>
//--------------------------------------------------------------
// Simple multithreaded reader-writer synchronization. 
// Respectful to waiting writers, so they skip the line asap.
// In C++14 can be replaced with shared_timed_mutex/shared_lock.
//--------------------------------------------------------------
class ReaderWriter
{
    std::mutex mutexMonitor;
    std::condition_variable conditionSignal;
    int numReaders = 0;
    int numWritersWaiting = 0;
    bool isWriterActive = false;
    void startReading() {
        std::unique_lock<std::mutex> locker(mutexMonitor);
        conditionSignal.wait(locker, [this]() {return numWritersWaiting <= 0 && !isWriterActive; });
        ++numReaders;
    }
    void stopReading() {
        std::unique_lock<std::mutex> locker(mutexMonitor);
        if (!--numReaders)
        {
            conditionSignal.notify_all();
        }
    }
    void startWriting() {
        std::unique_lock<std::mutex> locker(mutexMonitor);
        ++numWritersWaiting;
        conditionSignal.wait(locker, [this]() {return numReaders <= 0 && !isWriterActive; });
        --numWritersWaiting;
        isWriterActive = true;
    }
    void stopWriting() {
        std::unique_lock<std::mutex> locker(mutexMonitor);
        isWriterActive = false;
        conditionSignal.notify_all();
    }
    using RWMember = void (ReaderWriter::*)();
    class RWLock
    {
        ReaderWriter* p;
        RWMember stop;
    public:
        explicit RWLock(ReaderWriter& s, RWMember start, RWMember stop_) :p(std::addressof(s)),stop(stop_)
        {
            (p->*start)();
        }
        ~RWLock()
        {
            (p->*stop)();
        }
        RWLock(const RWLock&) = delete;
        RWLock& operator=(const RWLock&) = delete;
        RWLock& operator=(RWLock&&) noexcept = delete;
        RWLock(RWLock&&) noexcept = delete;
    };
public:
    struct ReadLock : public RWLock
    {
        explicit ReadLock(ReaderWriter& s) :RWLock(s, &ReaderWriter::startReading, &ReaderWriter::stopReading) {}
    };
    struct WriteLock : public RWLock
    {
        explicit WriteLock(ReaderWriter& s) :RWLock(s, &ReaderWriter::startWriting, &ReaderWriter::stopWriting) {}
    };
};
//--------------------------------------------------------------
// Let's test both implementation 
// by reading/writing random number sequences...
//--------------------------------------------------------------
using namespace std::literals::chrono_literals;

int main()
{
    std::stack<int> buffer;

    bool use_cplusplus_2014;
    ReaderWriter rwMonitor;
    std::shared_timed_mutex rwMonitor2014;
    
    auto seed = std::random_device{}();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> uni(10, 90);
    auto randomMillis = [&uni, &rng]() {return uni(rng);};

    auto writer = [&buffer, &rwMonitor, &rwMonitor2014, &use_cplusplus_2014, &randomMillis]() {
        for (int i = 1; i < 50; ++i) {
            {
                if (use_cplusplus_2014) 
                {
                    std::lock_guard<std::shared_timed_mutex> lock{ rwMonitor2014 };
                    buffer.push(i);
                }
                else
                {
                    ReaderWriter::WriteLock lock{ rwMonitor };
                    buffer.push(i);
                }
            }
            std::this_thread::sleep_for(randomMillis()*1ms);
        }
    };
    auto reader = [&buffer, &rwMonitor, &rwMonitor2014, &use_cplusplus_2014, &randomMillis]() {
        for (int i = 1; i < 100; ++i) {
            int x = -1;
            if (use_cplusplus_2014)
            {
                std::shared_lock<std::shared_timed_mutex> lock{ rwMonitor2014 };
                if (!buffer.empty()) { x = buffer.top(); buffer.pop(); }
            }
            else
            {
                ReaderWriter::ReadLock lock{ rwMonitor };
                if (!buffer.empty()) { x = buffer.top(); buffer.pop(); }
            }
            if (x >= 0) std::cout << x << std::endl;
            std::this_thread::sleep_for(randomMillis() * 1ms);
        }
    };
    use_cplusplus_2014 = false;
    std::cout << "Test custom ReaderWriter locks..." << std::endl;
    std::thread threadWriter(writer);
    std::thread threadReader(reader);
    threadWriter.join();
    threadReader.join();

    use_cplusplus_2014 = true;
    rng.seed(seed); //use same randome sequence
    std::cout << "Test C++14 ReaderWriter locks..." << std::endl;
    std::thread threadWriter14(writer);
    std::thread threadReader14(reader);
    threadWriter14.join();
    threadReader14.join();
}
