//   Copyright 2025 Lorenzo Carletti (lorenzo.carletti@unimore.it) Gianluca Brilli (gianluca.brilli@unimore.it)
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef PERF_COMPILATION
#include "perf_reader.h"
#endif
#include "shared_memory_maker.h"
#include "utils.h"

// Where THROTTLE is defined
#include "additional.h"

#define CACHE_LINE_SIZE 64

#define WISHED_ALIGNMENT (2*1024*1024)

#ifndef THROTTLE
#define THROTTLE 0
#endif

#define ITER 100
#define BASE_ARRAY_SIZE (16*1024*1024)

struct elem {
	struct elem* next;
	#if CACHE_LINE_SIZE > 4
	uint8_t dummy[CACHE_LINE_SIZE - sizeof(struct elem*)];
	#endif
};

static int get_next_elem_index(int curr_index, int count, struct elem* array, int sequential) {
	if(sequential)
		return curr_index + 1;

	int index = rand() % count;
	while((index == curr_index) && (array[index].next != NULL)) // Find first free that isn't you
		index = (index + 1) % count;
	return index;
}

static void prepare(struct elem* array, unsigned size, int sequential) {
	int count = size / sizeof(struct elem);
	memset(array, 0, size);

	int index = 0;
	for (int i = 0; i < count - 1; i++) {
		int next_index = get_next_elem_index(index, count, array, sequential);
		array[index].next = &array[next_index];
		index = next_index;
	}
	array[index].next = &array[0];
}

int main(int argc, char **argv) {
	if(print_help(argc, argv))
		return 0;

	int final_size = BASE_ARRAY_SIZE;
	int num_iters = ITER;

	#ifdef PERF_COMPILATION
	perf_arg_reader(argc, argv, 3);
	#endif

	if(argc > 1)
		final_size = atoi(argv[1]) * 1024;

	if(argc > 2)
		num_iters = atoi(argv[2]);

	struct timespec t1, t2;

	void* base_array = NULL;
	struct elem* array = NULL;
	int retval = create_memory_region(final_size, &base_array, (void**)&array, WISHED_ALIGNMENT);
	if (retval < 0)
		return retval;

	prepare(array, final_size, 1);

	int iter = 0;
	#ifdef PERF_COMPILATION
	perf_prepare(FIND_CPU_FOR_ME);
	perf_purge();
	#endif
	clock_gettime(CLOCK_REALTIME, &t1);

	struct elem* start = &array[0];

	while (CAN_CONTINUE_ITERS(iter, num_iters)) {
		__asm__ __volatile__(
			"mov x0, %[target];"
			"L1: ldr x0, [x0];"
			#if THROTTLE > 0
			"mov x3, %[value];"
			"L2: subs x3, x3, #1 ;"
			"bne L2;"
			#endif
			"cmp x0, %[target];"
			"bne L1;" :: [target] "r" (start), [value] "r" (THROTTLE) : "x0", "x3" );
		iter++;
	}

	clock_gettime(CLOCK_REALTIME, &t2);
	#ifdef PERF_COMPILATION
	perf_cleanup();
	#endif

	printf("%f\n", ((t2.tv_sec-t1.tv_sec)+(t2.tv_nsec-t1.tv_nsec)/1000000000.0)*1);
	#ifdef PERF_COMPILATION
	perf_print_results(num_iters);
	#endif

	destroy_memory_region(base_array);

	return 0;
}

