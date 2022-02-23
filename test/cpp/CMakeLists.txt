
# Tests need to be added as executables first
add_executable(cpptest main.cpp)

# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(cpptest PRIVATE openjph)

#C++ 14
target_compile_features(cpptest PUBLIC cxx_std_14)

