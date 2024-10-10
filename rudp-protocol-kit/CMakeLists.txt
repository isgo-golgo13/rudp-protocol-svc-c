cmake_minimum_required(VERSION 3.10)
project(rudp_protocol_kit)

# Define the library target
add_library(rudp_protocol_kit SHARED rudp_protocol.c)

# Include storage-kit as a dependency (since it's required by rudp_protocol_kit)
target_link_libraries(rudp_protocol_kit PRIVATE storage_kit)

# Specify include directories for other modules to find the headers
target_include_directories(rudp_protocol_kit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Optionally link any other necessary libraries
# target_link_libraries(rudp_protocol_kit <external_libs>)
