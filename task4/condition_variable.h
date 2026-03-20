#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include <limits>


void FutexWait(void* value, int expectedValue) {
   syscall(SYS_futex, value, FUTEX_WAIT_PRIVATE, expectedValue, nullptr, nullptr, 0);
}


void FutexWake(void* value, int count) {
   syscall(SYS_futex, value, FUTEX_WAKE_PRIVATE, count, nullptr, nullptr, 0);
}


class ConditionVariable {
private:
   std::atomic<int> m_sequence;
   std::atomic<int> m_waiters;

public:
   ConditionVariable() : m_sequence(0), m_waiters(0) {}
  
   ConditionVariable(const ConditionVariable&) = delete;
   ConditionVariable& operator=(const ConditionVariable&) = delete;
  
   void wait(std::unique_lock<std::mutex>& lock) {
       int seq = m_sequence.load(std::memory_order_relaxed);
       m_waiters.fetch_add(1, std::memory_order_relaxed);
      
       lock.unlock();
      
       while (true) {
           int current_seq = m_sequence.load(std::memory_order_acquire);
           if (seq != current_seq) {
               break;
           }
           FutexWait(&m_sequence, seq);
       }
      
       m_waiters.fetch_sub(1, std::memory_order_relaxed);
       lock.lock();
   }
  
   template<typename Predicate>
   void wait(std::unique_lock<std::mutex>& lock, Predicate pred) {
       while (!pred()) {
           wait(lock);
       }
   }
  
   void notify_one() noexcept {
       m_sequence.fetch_add(1, std::memory_order_release);
      
       if (m_waiters.load(std::memory_order_relaxed) > 0) {
           FutexWake(&m_sequence, 1);
       }
   }
  
   void notify_all() noexcept {
       m_sequence.fetch_add(1, std::memory_order_release);
      
       if (m_waiters.load(std::memory_order_relaxed) > 0) {
           FutexWake(&m_sequence, std::numeric_limits<int>::max());
       }
   }
};

