cmake_minimum_required(VERSION 3.3.2)

project(synthetic_benchmarks)

set(DO_PERF_COMPILATION TRUE CACHE BOOL "Specifies whether to enable perf events monitoring")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -ftree-vectorize -lm")
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)

set(EXTRA_INCLUDE_DIRS "")
set(SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BENCHMARKS_FOLDER ${SRC_FOLDER}/benchmarks)
set(INFINITE_COMPILE_FLAG "INFINITE_RUN")

if(${DO_PERF_COMPILATION})
	set(PERF_COMPILE_FLAG "PERF_COMPILATION")
	set(PERF_LIBRARY_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/libs/perf)
	set(LIBCAP_LIBRARY_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/libs/libcap)
	set(PERF_LIBRARY ${PERF_LIBRARY_FOLDER}/lib/libperf.a)
	set(LIBCAP_LIBRARY ${LIBCAP_LIBRARY_FOLDER}/libcap.a)
	list(APPEND EXTRA_INCLUDE_DIRS ${PERF_LIBRARY_FOLDER}/include)
	set(PERF_READER_SRC ${SRC_FOLDER}/perf_reader.c)
else()
	set(PERF_COMPILE_FLAG "")
	set(PERF_LIBRARY "")
	set(LIBCAP_LIBRARY "")
	set(PERF_READER_SRC "")
endif()

set(SHARED_MEMORY_MAKER_SRC ${SRC_FOLDER}/shared_memory_maker.c)
set(UTILS_SRC ${SRC_FOLDER}/utils.c)

link_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${EXTRA_INCLUDE_DIRS})

#######################
# Executables - base
#######################

add_executable(
	read_miss
	${BENCHMARKS_FOLDER}/read_miss.c
	${PERF_READER_SRC}
	${SHARED_MEMORY_MAKER_SRC}
	${UTILS_SRC}
)

add_executable(
	memcpy
	${BENCHMARKS_FOLDER}/memcpy.c
	${PERF_READER_SRC}
	${SHARED_MEMORY_MAKER_SRC}
	${UTILS_SRC}
)

add_executable(
	write_miss
	${BENCHMARKS_FOLDER}/write_miss.c
	${PERF_READER_SRC}
	${SHARED_MEMORY_MAKER_SRC}
	${UTILS_SRC}
)

add_executable(
	memset
	${BENCHMARKS_FOLDER}/memset.c
	${PERF_READER_SRC}
	${SHARED_MEMORY_MAKER_SRC}
	${UTILS_SRC}
)

#######################
# Executables - infinite
#######################

add_executable(
	read_miss_infinite
	${BENCHMARKS_FOLDER}/read_miss.c
	${SHARED_MEMORY_MAKER_SRC}
	${UTILS_SRC}
)

add_executable(
	memcpy_infinite
	${BENCHMARKS_FOLDER}/memcpy.c
	${SHARED_MEMORY_MAKER_SRC}
	${UTILS_SRC}
)

add_executable(
	write_miss_infinite
	${BENCHMARKS_FOLDER}/write_miss.c
	${SHARED_MEMORY_MAKER_SRC}
	${UTILS_SRC}
)

add_executable(
	memset_infinite
	${BENCHMARKS_FOLDER}/memset.c
	${SHARED_MEMORY_MAKER_SRC}
	${UTILS_SRC}
)

#######################
# Definitions - base
#######################

target_compile_definitions(
	read_miss
	PUBLIC
	${PERF_COMPILE_FLAG}
)

target_compile_definitions(
	memcpy
	PUBLIC
	${PERF_COMPILE_FLAG}
)

target_compile_definitions(
	write_miss
	PUBLIC
	${PERF_COMPILE_FLAG}
)

target_compile_definitions(
	memset
	PUBLIC
	${PERF_COMPILE_FLAG}
)

#######################
# Definitions - infinite
#######################

target_compile_definitions(
	read_miss_infinite
	PUBLIC
	${INFINITE_COMPILE_FLAG}
)

target_compile_definitions(
	memcpy_infinite
	PUBLIC
	${INFINITE_COMPILE_FLAG}
)

target_compile_definitions(
	write_miss_infinite
	PUBLIC
	${INFINITE_COMPILE_FLAG}
)

target_compile_definitions(
	memset_infinite
	PUBLIC
	${INFINITE_COMPILE_FLAG}
)

#######################
# Linking - base
#######################

target_link_libraries(
	read_miss
	${PERF_LIBRARY}
	${LIBCAP_LIBRARY}
)

target_link_libraries(
	memcpy
	${PERF_LIBRARY}
	${LIBCAP_LIBRARY}
)

target_link_libraries(
	write_miss
	${PERF_LIBRARY}
	${LIBCAP_LIBRARY}
)

target_link_libraries(
	memset
	${PERF_LIBRARY}
	${LIBCAP_LIBRARY}
)
