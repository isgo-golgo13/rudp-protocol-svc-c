cmake_minimum_required(VERSION 3.10)
project(storage_kit)

# Define the library target
add_library(storage_kit SHARED storage.c storage_repository.c)

# Specify include directories for other modules to find the headers
target_include_directories(storage_kit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Optionally link any other necessary libraries (e.g., ScyllaDB or other external dependencies)
# target_link_libraries(storage_kit <external_libs>)
