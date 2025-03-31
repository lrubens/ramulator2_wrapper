fn main() {
    // Tell cargo to invalidate the built crate whenever the wrapper changes
    // let dst = cmake::Config::new(".").build();
    // println!("cargo:rustc-link-search=native={}", dst.display());

    println!("cargo:rerun-if-changed=cpp/ramulator2_wrapper.cpp");
    println!("cargo:rerun-if-changed=cpp/ramulator2_wrapper.h");

    let lib_path = "ext/ramulator2/";
    println!("cargo:rustc-env=LD_LIBRARY_PATH={}", lib_path);
    println!("cargo:rustc-link-search=native={}", lib_path);
    
    // Link to the Ramulator shared library
    println!("cargo:rustc-link-lib=ramulator");

    
    println!("cargo:rustc-link-lib=stdc++");
    println!("cargo:rustc-link-lib=yaml-cpp");
    println!("cargo:rustc-link-lib=spdlog");
    println!("cargo:rustc-link-lib=fmt");

    println!("cargo:lib-path={}", lib_path);
    
    // Build the C++ wrapper
    let mut builder = cc::Build::new();
    builder.include("ext/ramulator2/src");
    builder.include("ext/spdlog/include");
    builder.include("ext/yaml-cpp/include");
    builder.include("ext/argparse/include");
    builder
        .cpp(true)
        .file("cpp/ramulator2_wrapper.cpp")
        .flag("-std=c++20") 
        .flag("-O2") 
        .compile("ramulator2_wrapper");
    
    // Generate bindings only if the "generate-bindings" feature is enabled
}