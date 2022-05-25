
add_executable(openjphjs jslib.cpp)

target_link_libraries(openjphjs PRIVATE openjphsimd)
target_compile_options(openjphjs PRIVATE -DOJPH_ENABLE_WASM_SIMD -msimd128)
target_compile_features(openjphjs PUBLIC cxx_std_11)
set_target_properties(
    openjphjs 
    PROPERTIES 
    LINK_FLAGS "\
      -O3 \
      -s WASM=1 \
      --bind \
      -s DISABLE_EXCEPTION_CATCHING=1 \
      -s ASSERTIONS=0 \
      -s NO_EXIT_RUNTIME=1 \
      -s MALLOC=emmalloc \
      -s ALLOW_MEMORY_GROWTH=1 \
      -s TOTAL_MEMORY=536870912 \
      -s FILESYSTEM=0 \
      -s EXPORTED_FUNCTIONS=[] \
      -s EXPORTED_RUNTIME_METHODS=[ccall] \
   ")
