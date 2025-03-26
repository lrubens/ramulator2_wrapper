#include "ramulator2_wrapper.h"
#include <iostream>

int main() {
  // Path to the Ramulator configuration file
  std::string config_path =
      "/home/rubensl/Documents/repos/ramulator2_wrapper/hbm.yaml";

  try {
    // Instantiate the Ramulator wrapper
    void *ramu = ramulator_new(config_path.c_str());
    // RamulatorWrapper *ramulator = (RamulatorWrapper *)ramu;

    // Send multiple read requests in a loop
    for (uint64_t address = 0x100; address < 0x2000000; address += 0x100) {
      // ramulator->sendRequest(address, true); // Read requests
      while (!ramulator_send_request(ramu, address, false)) {
        ramulator_tick(ramu);
      }
    }

    // Send a write request
    // ramulator.sendRequest(0x2000, false); // Write request to address 0x2000

    // Save the current state/data
    // ramulator.saveData("memory_state.txt");
    std::cout << "Found cycle: " << ramulator_get_cycle(ramu) << std::endl;

    // Run the simulation for 1000 cycles
    // for(uint64_t i = 0; i < 1000; i+=1){
    // ramulator_tick(ramu);
    // }
    // std::cout << "Found cycle: " << ramulator_get_cycle(ramu) << std::endl;
    // delete ramu;

  } catch (const std::exception &e) {
    std::cerr << "An error occurred: " << e.what() << std::endl;
  }

  return 0;
}