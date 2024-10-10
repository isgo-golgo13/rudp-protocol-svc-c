cmake_minimum_required(VERSION 3.10)
project(rudp_project_c)

# Add subdirectories (build the shared libraries first)
add_subdirectory(storage-kit)
add_subdirectory(rudp-protocol-kit)

# Add the client executable
add_executable(client_executable client/client.c)

# Link the client executable to the rudp_protocol_kit (and transitively storage_kit)
target_link_libraries(client_executable PRIVATE rudp_protocol_kit)

# Add the server executable
add_executable(server_executable server/server.c)

# Link the server executable to the rudp_protocol_kit (and transitively storage_kit)
target_link_libraries(server_executable PRIVATE rudp_protocol_kit)
