////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Matthew Bedder (bedder@gmail.com)
//
// This code has been released under the MIT license. See the LICENSE file in
// the project root for more information
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>
#include "ThreadPool.h"

uint64_t fib(int n) {
  if (n == 0)
    return 0;
  uint64_t prev = 0, curr = 1;
  for (int i=1 ; i<n ; i++) {
    std::swap(prev, curr);
    curr += prev;
  }
  return curr;
}

int main(int argc, char **argv) {
  helper::ThreadPool pool(4, &std::cerr);
  std::mutex mt;
  
  //
  // First example - queue up ten tasks, then wait for them all to complete
  //
  std::cout << "=====================================================\n"
               "ThreadPool example 1 : run until completion [fib(10)]\n"
               "=====================================================\n";
  for (int i=0 ; i<=10 ; i+=1) {
    pool.addTask([i, &mt]() {
      uint64_t res = fib(i);
      std::lock_guard<std::mutex> guard(mt);
      std::cout << "fib(" << i << ") : " << res << "\n";
    });
  }
  pool.waitUntilComplete(false,   // We don't want other threads to add new tasks while we're waiting
                         false);  // We want to be able to reuse the thread pool afterwards
  
  //
  // Second example - queue up ten tasks, then abandon any tasks not already started
  //
  std::cout << "\n"
               "===================================\n"
               "ThreadPool example 2 : run for 50ms\n"
               "===================================\n";
  for (int i=0 ; i<=60 ; i+=1) {
    pool.addTask([i, &mt]() {
      uint64_t res = fib(i);
      std::lock_guard<std::mutex> guard(mt);
      std::cout << "fib(" << i << ") : " << res << "\n";
    });
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  pool.waitUntilComplete(false, // We don't want other threads to add new tasks while we're waiting
                         true); // We want the thread pool to be unusable afterwards
}