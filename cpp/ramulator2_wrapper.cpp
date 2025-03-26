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

  bool enqueue_success = false;

  if (is_write) {
    enqueue_success = frontend->receive_external_requests(
        1, addr, 0, [this](Ramulator::Request &req) {
          this->out_queue.push(req.addr);
          // std::cout << "Enqueue write request" << std::endl;
          --num_outstanding_writes;
        });
  } else {
    enqueue_success = frontend->receive_external_requests(
        0, addr, 0, [this](Ramulator::Request &req) {
          // std::cout << "Enqueue read request" << std::endl;
          --num_outstanding_reads;
        });
  }

  // TODO: Figure out what needs to be added here
  if (enqueue_success) {
    // std::cout << "Request enqueued successfully" << std::endl;
    outgoing_reqs++;
  } else {
    // std::cout << "Request enqueue failed (queue full)" << std::endl;
  }

  return enqueue_success;
}

void RamulatorWrapper::tick() {
  tCK++;
  frontend->tick();
  memory_system->tick();
}

bool RamulatorWrapper::is_finished() {
  return frontend->is_finished();
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
    return false;
  }
  auto sim_wrapper = (RamulatorWrapper *)sim;
  return sim_wrapper->send_request(addr, is_write);
}

void ramulator_tick(void *sim) {
  auto sim_wrapper = (RamulatorWrapper *)sim;
  sim_wrapper->tick();
}

bool ramulator_is_finished(void *sim) {
  auto sim_wrapper = (RamulatorWrapper *)sim;
  if (!sim_wrapper) {
    std::cerr << "Simulator instance is null" << std::endl;
    return false; // Treat as not finished if invalid
  }

  return sim_wrapper->is_finished();
}

float ramulator_get_cycle(void *sim) {
  auto sim_wrapper = (RamulatorWrapper *)sim;
  if (!sim_wrapper) {
    return 0;
  }

  return sim_wrapper->get_cycle();
}
