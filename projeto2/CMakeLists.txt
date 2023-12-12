cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})


project(projeto2)

set(RTDB_FILES src/include/rtdb.h src/rtdb.c)
set(FIFO_FILES src/include/fifo.h src/fifo.c)
set(UART_FILES src/include/uart.h src/uart.c)
set(PROTOCOL_FILES src/include/protocol.h src/protocol.c)
# set(I2C_FILES src/include/i2c.h src/i2c.c)

target_sources(app PRIVATE src/main.c ${RTDB_FILES} ${FIFO_FILES} ${UART_FILES} ${PROTOCOL_FILES})
