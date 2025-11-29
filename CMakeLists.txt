cmake_minimum_required(VERSION 3.20.0)

# Find Zephyr package
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Project name
project(zephyr_threads_demo)

# Add source files
# full demo  for real-time primitive concepts
# target_sources(app PRIVATE src/main.c)

# Advanced concepts
# Reader-Writer demo
#target_sources(app PRIVATE src/demo_reader_writer.c)

# thread pool demo
target_sources(app PRIVATE src/demo_thread_pool.c)
