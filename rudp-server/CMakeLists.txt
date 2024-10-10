cmake_minimum_required(VERSION 3.10)

# Specify executable target (if additional logic needed for server build)
add_executable(server_executable rudp-socket-server.c)

# Link to rudp_protocol_kit (and storage_kit through it)
target_link_libraries(server_executable PRIVATE rudp_protocol_kit)
