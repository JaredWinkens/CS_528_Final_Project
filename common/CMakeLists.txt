# Build library
add_library(common STATIC main.c)

# Link necessary libraries
target_link_libraries(common PRIVATE PkgConfig::GTK4)

# Include headers for common libs
target_include_directories(common PRIVATE ${GTK4_INCLUDE_DIRS})

