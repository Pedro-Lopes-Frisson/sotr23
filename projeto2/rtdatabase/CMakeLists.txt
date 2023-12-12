cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Set the source files that make up the library
set(MOVAVG_SRCS rtdb.h rtdb.c)

# Set the library type (can be static or dynamic )
SET (LIB_TYPE STATIC)

# Create the library
add_library (rtdatabase ${LIB_TYPE} ${MOVAVG_SRCS})