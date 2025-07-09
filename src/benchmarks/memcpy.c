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

#define ITER 200
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

	void* src_base = NULL;
	unsigned char* src = NULL;
	void* src2_base = NULL;
	unsigned char* src2 = NULL;
	void* dst_base = NULL;
	unsigned char* dst = NULL;

	int retval = create_memory_region(final_size, &src_base, (void**)&src, WISHED_ALIGNMENT);
	if(retval < 0)
		return retval;

	retval = create_memory_region(final_size, &src2_base, (void**)&src2, WISHED_ALIGNMENT);
	if(retval < 0)
		return retval;

	retval = create_memory_region(final_size, &dst_base, (void**)&dst, WISHED_ALIGNMENT);
	if(retval < 0)
		return retval;

	// Pre-set them to avoid page faults
	memset(src, 0x11111110, final_size);
	memset(src2, 0x13333331, final_size);
	memset(dst, 0x22222222, final_size);

	load_array(dst, final_size);

	int iter = 0;
	#ifdef PERF_COMPILATION
	perf_prepare(FIND_CPU_FOR_ME);
	perf_purge();
	#endif
	clock_gettime(CLOCK_REALTIME, &t1);

	while(CAN_CONTINUE_ITERS(iter, num_iters)) {
		unsigned char *source = src;
		unsigned char *dest = dst;

		// Swap between the two srcs to keep changing the values written
		if(iter & 1)
			source=src2;

		__asm__ __volatile__(
			"mov x0, %[src];"
			"mov x1, %[dst];"
			"mov x2, %[target];"
			// This achieves 50-50 load store.
			// The extra ldrs are omitted due to
			// not causing any extra traffic on L2/DRAM.
			// They could be restored, but care must be taken
			// in making sure that the registers used are not
			// the same for every ldr.
			// It is important that the stores aren't posted in the
			// assembly, else the performance suffers
			// Assumes 64 bytes cache line!
			"L1%=: ldr x3, [x0], #64;"
			"str x3, [x1];"
			"str x3, [x1,#8];"
			"str x3, [x1,#16];"
			"str x3, [x1,#24];"
			"str x3, [x1,#32];"
			"str x3, [x1,#40];"
			"str x3, [x1,#48];"
			"str x3, [x1,#56];"
			// Assumes 64 bytes cache line!
			"add x1, x1,#64;"
			#if THROTTLE > 0
			"mov x3, %[value];"
			"L2%=: subs x3, x3, #1;"
			"bne L2%=;"
			#endif
			"cmp x1, x2;"
			"bne L1%=;" : : [dst] "r" ((unsigned long long int)dest), [src] "r" ((unsigned long long int)source), [target] "r" (((unsigned long long int)dest) + final_size), [value] "r" (THROTTLE) : "x0", "x1", "x2", "x3");

		iter++;
	}

	clock_gettime(CLOCK_REALTIME, &t2);
	#ifdef PERF_COMPILATION
	perf_cleanup();
	#endif

	printf("%.8f\n", ((t2.tv_sec-t1.tv_sec)+(t2.tv_nsec-t1.tv_nsec)/1000000000.0)*1);
	#ifdef PERF_COMPILATION
	perf_print_results(num_iters);
	#endif

	destroy_memory_region(src_base);
	destroy_memory_region(src2_base);
	destroy_memory_region(dst_base);

	return 0;
}
