use std::ffi::CString;
use std::panic;

extern "C" {
    // Opaque simulator struct from the C++ wrapper, as declared in the header.
    pub fn ramulator_new(config_path: *const std::os::raw::c_char) -> *mut libc::c_void;
    pub fn ramulator_free(sim: *mut libc::c_void);
    pub fn ramulator_send_request(sim: *mut libc::c_void, addr: u64, is_write: bool) -> bool;
    pub fn ramulator_tick(sim: *mut libc::c_void) -> std::os::raw::c_int;
    pub fn ramulator_is_finished(sim: *mut libc::c_void) -> std::os::raw::c_int;
    pub fn ramulator_get_statistic(
        sim: *mut libc::c_void,
        name: *const std::os::raw::c_char,
        value: *mut f64,
    ) -> std::os::raw::c_int;
    pub fn ramulator_get_cycle(sim: *mut libc::c_void) -> u64;
}

/// Memory request representation for Ramulator
#[derive(Debug, Clone)]
pub struct MemoryRequest {
    /// Physical address for the request
    pub address: u64,
    /// Whether this is a write request (true) or read request (false)
    pub is_write: bool,
    /// Size of the request in bytes
    pub size: usize,
}

/// Safe Rust wrapper for Ramulator2 memory system
pub struct RamulatorWrapper {
    sim: *mut libc::c_void,
}

impl RamulatorWrapper {
    pub fn new(config_path: &str) -> Self {
        let config_c_str = CString::new(config_path).unwrap();

        let sim = unsafe { ramulator_new(config_c_str.as_ptr()) };

        if sim.is_null() {
            panic!("Failed to create Ramulator system");
        }

        RamulatorWrapper { sim }
    }

    /// Send a memory request to Ramulator
    pub fn send_request(&self, request: MemoryRequest) -> bool {
        unsafe { ramulator_send_request(self.sim, request.address, request.is_write) }
    }

    pub fn tick(&self) {
        unsafe { ramulator_tick(self.sim) };
    }

    /// Get the current simulation cycle.
    pub fn get_cycle(&self) -> u64 {
        unsafe { ramulator_get_cycle(self.sim) }
    }

    pub fn ramulator_free(&self) {
        unsafe {
            ramulator_free(self.sim);
        }
    }
}

impl Drop for RamulatorWrapper {
    fn drop(&mut self) {
        unsafe {
            ramulator_free(self.sim);
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{MemoryRequest, RamulatorWrapper};

    #[test]
    fn ffi_test() -> Result<(), Box<dyn std::error::Error>> {
        // Initialize Ramulator with a config file
        let ramulator = RamulatorWrapper::new("hbm.yaml");

        // Send the request
        for i in 0..1000000 {
            let read_request = MemoryRequest {
                address: i,
                is_write: false,
                size: 64, // 64 bytes (cache line size)
            };
            while !ramulator.send_request(read_request.clone()) {
                ramulator.tick();
            }
        }

        println!("Elapsed cycles: {}", ramulator.get_cycle());

        Ok(())
    }
}
