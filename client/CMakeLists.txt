# Build binary
add_executable(Chat_Client main.c)

# Link necessary libraries
target_link_libraries(Chat_Client PRIVATE PkgConfig::GTK4)

# Include headers for client
target_include_directories(Chat_Client PRIVATE ${GTK4_INCLUDE_DIRS})

