////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Matthew Bedder (bedder@gmail.com)
//
// This code has been released under the MIT license. See the LICENSE file in
// the project root for more information
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <thread>
#include <mutex>
#include <ostream>
#include <queue>
#include <string>

namespace helper {

////////////////////////////////////////////////////////////////////////////////
// ThreadPool
//   This class allows you to define a pool of N threads that can be used to run
//   functions operations stored in a queue. This is based off Jakob Progsch's
//   C++11 Thread Pool (accessible here: http://progsch.net/wordpress/?p=81) but
//   with changes made for readability, logging, and to allow us to wait for all
//   threads to complete before terminating.
//
//   Usage of this class should be fairly self-explanatory, as follows:
//       ThreadPool thread_pool(NUM_THREADS, &std::cerr);
//       for (int i=0 ; i<20 ; i++)
//         thread_pool.addTask([i]() { fnc_with_side_effect(i); });
//       thread_pool.waitUntilComplete(true);
//
//   Note the dependency on ThreadPoolWorker.h
////////////////////////////////////////////////////////////////////////////////
class ThreadPool {
public:
  friend class ThreadPoolWorker;

public:
  explicit ThreadPool(size_t nThreads = 4,
                      std::ostream* logStream = nullptr);
  ~ThreadPool();

  template <typename F>
  void addTask(F function);
  void waitUntilComplete(bool allowNewTasks = true,
                         bool abandonTasks  = false,
                         bool terminatePool = false);
  void reset();
  void respawn();

private:
  std::ostream* log_;
  std::vector<std::thread> workers_ = {};
  std::vector<char>        working_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mt_;
  std::condition_variable cv_;
  bool allowNewTasks_ = true;   // Is the ThreadPool accepting new tasks?
  bool terminate_     = false;  // Should the ThreadPool terminate when it can?

  void logMessage(const std::string& msg);
};

////////////////////////////////////////////////////////////////////////////////
// ThreadPoolWorker
//   An auxiliary class used for thread pooling. This is based off Jakob
//   Progsch's C++11 Worker Class (accessible here: http://progsch.net/wordpress/?p=81)
//   but with changes made for readability, logging, and to allow us to wait for
//   all threads to complete before terminating.
//
//   This class is not intended to be used separately from ThreadPool - for
//   guidance on how to use the ThreadPool class, please look at ThreadPool.h
////////////////////////////////////////////////////////////////////////////////
class ThreadPoolWorker {
public:
  explicit ThreadPoolWorker(ThreadPool& tp, size_t id);
  void operator()();

private:
  ThreadPool& tp_;
  size_t      id_;
};

////////////////////////////////////////////////////////////////////////////////
ThreadPool::ThreadPool(size_t nThreads, std::ostream* logStream)
: log_(logStream) {
  workers_.reserve(nThreads);
  working_.reserve(nThreads);
  for (unsigned int i = 0; i < nThreads; i++) {
    working_.push_back(false);
    workers_.push_back(std::thread(ThreadPoolWorker(*this, i)));
  }
}

////////////////////////////////////////////////////////////////////////////////
ThreadPool::~ThreadPool() {
  waitUntilComplete(false, true, true);
}

////////////////////////////////////////////////////////////////////////////////
template <typename F>
void ThreadPool::addTask(F function) {
  if (!allowNewTasks_) {
    logMessage("ERROR in ThreadPool::addTask(F): "
               "Attempting to add task to a stopped thread pool.");
    return;
  }
  {
    std::lock_guard<std::mutex> guard(mt_);
    if (!allowNewTasks_) {
      logMessage("ERROR in ThreadPool::addTask(F): "
                 "Attempting to add task to a stopped thread pool.");
      return;
    }
    tasks_.push(std::function<void()>(function));
  }
  cv_.notify_one();
}

////////////////////////////////////////////////////////////////////////////////
// template <typename F>
// void ThreadPool::addTask(F function)
// // Declared in the header file to allow for template matching

////////////////////////////////////////////////////////////////////////////////
void ThreadPool::waitUntilComplete(bool allowNewTasks,
                                   bool abandonTasks,
                                   bool terminatePool) {
  if (allowNewTasks && (abandonTasks || terminatePool))
    throw std::logic_error("ThreadPool::waitUntilComplete : "
                           "allowNewTasks is incompatible with abandonTasks "
                           "and terminatePool.");

  allowNewTasks_ = allowNewTasks;

  if (abandonTasks) {
    // We're abandoning tasks (and not allowing new ones), so ditch the task queue
    std::lock_guard<std::mutex> guard(mt_);
    while (!tasks_.empty())
      tasks_.pop();
  } else {
    // Wait until all tasks have been completed
    while (1) {
      {
        std::lock_guard<std::mutex> guard(mt_);
        if (tasks_.empty()) {
          // At this point the task queue is empty, but tasks may still be running
          allowNewTasks_ = false;
          break;
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  if (terminatePool) {
    // There are no more tasks on the queue, so notify each thread and join
    terminate_ = true;
    cv_.notify_all();
    for (auto& w : workers_) {
      if (w.joinable())
        w.join();
    }
  } else {
    // Wait for each running process to terminate
    while (std::any_of(working_.cbegin(), working_.cend(), [](const char&b) { return b; })) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    // We're done, so allow new tasks again
    allowNewTasks_ = true;
  }
}


////////////////////////////////////////////////////////////////////////////////
void ThreadPool::logMessage(const std::string& msg) {
  if (log_ == nullptr)
    return;
  (*log_) << msg.c_str();
}

////////////////////////////////////////////////////////////////////////////////
ThreadPoolWorker::ThreadPoolWorker(ThreadPool& tp, size_t id)
: tp_(tp),
  id_(id) {
}

////////////////////////////////////////////////////////////////////////////////
void ThreadPoolWorker::operator()() {
  while (1) {
    std::function<void()> currentTask;

    { // START scope for unique_lock
      std::unique_lock<std::mutex> lock(tp_.mt_);
      // Wait while the thread pool is accepting tasks but there are none
      if (tp_.tasks_.empty()) {
        tp_.working_[id_] = false;
        tp_.cv_.wait(lock);
      }
      // If we're not allowing new tasks and there are none left, exit
      if (tp_.terminate_ && tp_.tasks_.empty()) {
        // End the function so the thread can terminate
        return;
      }
      currentTask = tp_.tasks_.front();
      tp_.tasks_.pop();
    } // END scope of unique_lock
    
    try {
      tp_.working_[id_] = true;
      currentTask();
    } catch (const std::exception& e) {
      // Eat the exception, but log with the parent ThreadPool
      tp_.logMessage("ERROR in ThreadPoolWorker[" + std::to_string(id_) + "] "
                     + "task. Original message is as follows:\n\t"
                     + std::string(e.what()) + "\n"
                     + "Ignoring exception and continuing.\n");
    }
  }
}

}  // namespace helper
