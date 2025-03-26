cmake_minimum_required(VERSION 3.16)
project(AI_Chat C CXX)

# Set C Standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall")

# Export for IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Get required GTK Packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

# Build binary
add_executable(
  AI_Chat
  hello-world-gtk.c
)

# Link necessary libraries
target_link_libraries(AI_Chat PRIVATE PkgConfig::GTK4)
