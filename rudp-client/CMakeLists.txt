cmake_minimum_required(VERSION 3.10)

# Specify executable target (if additional logic needed for client build)
add_executable(client_executable rudp-socket-client.c)

# Link to rudp_protocol_kit (and storage_kit through it)
target_link_libraries(client_executable PRIVATE rudp_protocol_kit)
