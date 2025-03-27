# Build binary
add_executable(Chat_Server main.c)

# Link necessary libraries
target_link_libraries(Chat_Server PRIVATE PkgConfig::GTK4)

# Include headers for common libs
target_include_directories(Chat_Server PRIVATE ${GTK4_INCLUDE_DIRS})

