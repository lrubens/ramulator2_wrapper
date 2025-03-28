// ramulator2-rs/wrapper.h
#ifndef RamulatorWrapper_H
#define RamulatorWrapper_H

// Include only standard C headers in the wrapper.h
// The C++ headers will be included in the .cpp file
#include <cstdint>
#include <iostream>
#include <map>
#include <unordered_map>
#include <queue>
#include <set>
#include <stdint.h>
#include <tuple>
#include <vector>

#include "base/base.h"
#include "base/config.h"
#include "base/exception.h"
#include "base/request.h"
#include "frontend/frontend.h"
#include "memory_system/memory_system.h"
#include "mutex"
#include "atomic"

#ifdef __cplusplus
extern "C" {
#endif

class RamulatorWrapper {
public:
  explicit RamulatorWrapper(const char *config);
  ~RamulatorWrapper();
  bool send_request(uint64_t addr, bool is_write);
  void tick();
  // double get_statistic(const char *name);
  bool is_finished();
  uint64_t get_cycle() { return tCK; }
  bool return_available() const { return !out_queue.empty(); }
  uint64_t pop() {
    uint64_t addr = out_queue.front();
    out_queue.pop();
    return addr;
  }

  bool empty() const {
    return out_queue.empty() && in_queue[0].empty() && outgoing_reqs == 0;
  }

private:
  // uint64_t tCK = 0.0;
  // uint64_t num_outstanding_reads = 0;
  // uint64_t num_outstanding_writes = 0;
  std::atomic<int> tCK{0};
  std::atomic<int> num_outstanding_reads{0};
  std::atomic<int> num_outstanding_writes{0};
  std::atomic<int> outgoing_reqs{0};
  std::string config_path;
  bool initialized;
  Ramulator::IFrontEnd *frontend;
  Ramulator::IMemorySystem *memory_system;
  // TODO: Might not be needed
  std::vector<std::queue<std::pair<uint64_t, bool>>> in_queue;
  std::queue<uint64_t> out_queue;
  std::mutex frontend_mutex;
  // std::unordered_map<uint64_t, std::deque<PacketPtr>> outstandingReads;
  // std::unordered_map<uint64_t, std::deque<PacketPtr>> outstandingWrites;
};

// Create a new ramulator simulator instance with the given config file
void *ramulator_new(const char *config_path);

void *ramulator_new_with_preset(const char *config_content);

// Free a ramulator simulator instance
void ramulator_free(void *sim);

// Send a memory request to the simulator
// addr: physical address
// type: request type (read/write)
// return: success or error code
bool ramulator_send_request(void *sim, uint64_t addr, bool is_write);

// Advance the simulation by one cycle
void ramulator_tick(void *sim);

uint64_t ramulator_pop(void *sim);

// Check if the simulation has finished processing all requests
bool ramulator_is_finished(void *sim);

// Get the current cycle count
float ramulator_get_cycle(void *sim);

bool ramulator_ret_available(void *sim);

#ifdef __cplusplus
}
#endif

#endif // RamulatorWrapper_H