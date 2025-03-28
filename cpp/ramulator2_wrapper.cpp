// ramulator2-rs/ramulator_wrapper.cpp
#include "ramulator2_wrapper.h"

// C++ standard library includes
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

RamulatorWrapper::RamulatorWrapper(const char *config_path)
    : config_path(config_path) {
  if (!config_path) {
    throw std::invalid_argument("Invalid argument");
  }

  std::cout << "Loading config file: " << config_path << std::endl;

  // Parse configuration
  auto config = Ramulator::Config::parse_config_file(config_path, {});
  // auto test = Ramulator::Config::parse_content(config_path, {});

  // Create frontend with the configuration
  // frontend = std::make_unique<Ramulator::Frontend>(*(config));
  frontend = Ramulator::Factory::create_frontend(config);
  memory_system = Ramulator::Factory::create_memory_system(config);

  frontend->connect_memory_system(memory_system);
  memory_system->connect_frontend(frontend);

  initialized = true;
}

RamulatorWrapper::~RamulatorWrapper() {
  if (initialized) {
    frontend->finalize();
    memory_system->finalize();
  }
}

bool RamulatorWrapper::send_request(uint64_t addr, bool is_write) {
  if (!initialized) {
    throw std::runtime_error("Ramulator instance not initialized");
  }

  std::lock_guard<std::mutex> lock(frontend_mutex);

  bool enqueue_success = false;

  if (is_write) {
    enqueue_success = frontend->receive_external_requests(
        1, addr, 0, [this](Ramulator::Request &req) {
          this->out_queue.push(req.addr);
          // int chan_id = req.addr_vec[0];
          std::atomic_fetch_sub(&num_outstanding_writes, 1);
        });

    if (enqueue_success) {
      // std::cout << "Request enqueued successfully" << std::endl;
      // auto &packet = outstandingReads.find(addr)->second;
      std::atomic_fetch_add(&num_outstanding_writes, 1);
    } else {
      // std::cout << "Request enqueue failed (queue full)" << std::endl;
    }
  } else {
    enqueue_success = frontend->receive_external_requests(
        0, addr, 0, [this](Ramulator::Request &req) {
          this->out_queue.push(req.addr);
          std::atomic_fetch_sub(&num_outstanding_reads, 1);
        });
    if (enqueue_success) {
      // std::cout << "Request enqueued successfully" << std::endl;
      // auto &packet = outstandingReads.find(addr)->second;
      std::atomic_fetch_add(&num_outstanding_reads, 1);
    } else {
      // std::cout << "Request enqueue failed (queue full)" << std::endl;
    }
  }

  return enqueue_success;
}

void RamulatorWrapper::tick() {
  std::atomic_fetch_add(&tCK, 1);
  frontend->tick();
  memory_system->tick();
}

bool RamulatorWrapper::is_finished() {
  return frontend->is_finished();
  // return num_outstanding_reads == 0 && num_outstanding_writes == 0 && frontend->is_finished();
  // return outgoing_reqs == 0 && frontend->is_finished();
}

void *ramulator_new(const char *config_path) {
  auto sim = new RamulatorWrapper(config_path);
  return sim;
}

void ramulator_free(void *sim) {
  auto sim_wrapper = (RamulatorWrapper *)sim;
  if (sim_wrapper) {
    delete sim_wrapper;
  }
}

bool ramulator_send_request(void *sim, uint64_t addr, bool is_write) {
  if (!sim) {
    std::cerr << "Simulator instance is null" << std::endl;
    return false;
  }
  auto sim_wrapper = (RamulatorWrapper *)sim;
  return sim_wrapper->send_request(addr, is_write);
}

void ramulator_tick(void *sim) {
  auto sim_wrapper = (RamulatorWrapper *)sim;
  if (!sim_wrapper) {
    std::cerr << "Error: Simulator instance is null" << std::endl;
    return;
  }
  sim_wrapper->tick();
}

uint64_t ramulator_pop(void *sim) {
  auto sim_wrapper = (RamulatorWrapper *)sim;
  if (!sim_wrapper) {
    std::cerr << "Error: Simulator instance is null" << std::endl;
    return 0;
  }
  return sim_wrapper->pop();
}

bool ramulator_is_finished(void *sim) {
  auto sim_wrapper = (RamulatorWrapper *)sim;
  if (!sim_wrapper) {
    std::cerr << "Error: Simulator instance is null" << std::endl;
    return false; // Treat as not finished if invalid
  }

  return sim_wrapper->is_finished();
}

float ramulator_get_cycle(void *sim) {
  auto sim_wrapper = (RamulatorWrapper *)sim;
  if (!sim_wrapper) {
    std::cerr << "Error: Simulator instance is null" << std::endl;
    return 0;
  }

  return sim_wrapper->get_cycle();
}

bool ramulator_ret_available(void *sim) {
  auto sim_wrapper = (RamulatorWrapper *)sim;
  if (!sim_wrapper) {
    std::cerr << "Error: Simulator instance is null" << std::endl;
    return false;
  }

  return sim_wrapper->return_available();
}
