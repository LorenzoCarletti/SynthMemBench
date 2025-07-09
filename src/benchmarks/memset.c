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

#define WISHED_ALIGNMENT (2*1024*1024)

#ifndef THROTTLE
#define THROTTLE 0
#endif

#define ITER 300
#define BASE_ARRAY_SIZE (16*1024*1024)

int main(int argc, char **argv) {
	if(print_help(argc, argv, BASE_ARRAY_SIZE, ITER))
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

	void* dst_base = NULL;
	unsigned char* dst = NULL;

	int retval = create_memory_region(final_size, &dst_base, (void**)&dst, WISHED_ALIGNMENT);
	if(retval < 0)
		return retval;

	memset(dst, 0x22222222, final_size);

	load_array(dst, final_size);

	int iter = 0;
	#ifdef PERF_COMPILATION
	perf_prepare(FIND_CPU_FOR_ME);
	perf_purge();
	#endif
	clock_gettime(CLOCK_REALTIME, &t1);

	while(CAN_CONTINUE_ITERS(iter, num_iters)) {
		__asm__ __volatile__(
			// Full cache line fill.
			// It is important that the stores aren't posted in the
			// assembly, else the performance suffers
			"mov x0, %[dst];"
			"L1: str %[data], [x0];"
			"str %[data], [x0,#8];"
			"str %[data], [x0,#16];"
			"str %[data], [x0,#24];"
			"str %[data], [x0,#32];"
			"str %[data], [x0,#40];"
			"str %[data], [x0,#48];"
			"str %[data], [x0,#56];"
			// Assumes 64 bytes cache line!
			"add x0, x0,#64;"
			#if THROTTLE > 0
			// Memset can be very easily throttled...
			// If more precision is needed, use
			// nops instead of a loop
			"mov x3, %[value];"
			"L2%=: subs x3, x3, #1;"
			"bne L2%=;"
			#endif
			"cmp x0, %[target];"
			"bne L1;" : : [dst] "r" ((unsigned long long int)dst), [data] "r" (iter), [target] "r" (((unsigned long long int)dst) + final_size), [value] "r" (THROTTLE) : "x0", "x3");

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

	destroy_memory_region(dst_base);

	return 0;
}
