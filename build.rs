// fn main() {
//     let dst = cmake::Config::new(".")
//         .build();
//     let ram = cmake::Config::new("ext/ramulator2")
//         .build();
//     println!("cargo:rustc-link-search=native={}", ram.display());
//     println!("cargo:rustc-link-lib=ramulator");
//     println!("cargo:rustc-link-search=native={}", dst.display());
//     println!("cargo:rustc-link-lib=static=ramulator_wrapper");
//     if cfg!(target_os = "linux") {
//         println!("cargo:rustc-link-lib=stdc++");
//     } else if cfg!(target_os = "macos") {
//         println!("cargo:rustc-link-lib=c++");
//     }

// }

use std::env;
use std::path::PathBuf;

fn main() {
    // Tell cargo to invalidate the built crate whenever the wrapper changes
    println!("cargo:rerun-if-changed=cpp/ramulator2_wrapper.cpp");
    println!("cargo:rerun-if-changed=cpp/ramulator2_wrapper.h");
    
    // First, try to find Ramulator2 in the standard location
    let mut ramulator_found = false;
    
    // Try different possible locations for the Ramulator2 library
    let possible_lib_paths = [
        "/usr/local/lib",
        "/usr/lib",
        "ext/ramulator2/build", // Local ext directory
        "ext/ramulator2/", // Local ext directory
    ];
    
    for lib_path in &possible_lib_paths {
        if PathBuf::from(lib_path).join("libramulator.so").exists() {
            println!("cargo:rustc-link-search={}", lib_path);
            ramulator_found = true;
            break;
        }
    }
    
    if !ramulator_found {
        println!("cargo:warning=Could not find libramulator.so in standard locations. Make sure Ramulator2 is installed properly.");
    }
    
    // Link to the Ramulator shared library
    println!("cargo:rustc-link-lib=ramulator");
    
    // Additional system libraries that might be needed
    println!("cargo:rustc-link-lib=stdc++");
    println!("cargo:rustc-link-lib=yaml-cpp");
    println!("cargo:rustc-link-lib=fmt");
    
    // Build the C++ wrapper
    let mut builder = cc::Build::new();
    
    // Detect possible include paths for Ramulator2 headers
    let possible_include_paths = [
        "/usr/local/include",
        "/usr/include",
        "ext/ramulator2",
    ];
    
    let mut include_paths = Vec::new();
    for path in &possible_include_paths {
        if PathBuf::from(path).exists() {
            include_paths.push(path.to_string());
        }
    }
    
    // Add all found include paths
    for path in &include_paths {
        builder.include(path);
    }
    
    // If we found ramulator2 in ext/, add specific include paths
    if include_paths.iter().any(|p| p.contains("ext/ramulator2")) {
        builder.include("ext/ramulator2/src");
        // builder.include("ext/ramulator2/ramulator2/ext/spdlog/include");
        // builder.include("ext/ramulator2/ramulator2/ext/yaml-cpp/include");
    }
    
    builder
        .cpp(true)
        .file("cpp/ramulator2_wrapper.cpp")
        .flag("-std=c++17") // Make sure we use C++17 as required by Ramulator2
        .compile("ramulator2_wrapper");
    
    // Generate bindings only if the "generate-bindings" feature is enabled
}
