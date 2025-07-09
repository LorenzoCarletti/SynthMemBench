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
#include <unistd.h>
#include <sys/types.h>
#include "utils.h"

#ifdef PERF_COMPILATION
#include "perf_reader.h"
#endif

void load_array(void* base_ptr, int final_size) {
	// Make sure the data is actually there...
	__asm__ __volatile__(
		"mov x0, %[src];"
		"mov x2, %[target];"
		// Assumes 64 bytes cache line!
		// Though it is not important...
		"L3%=: ldr x3, [x0], #64;"
		"cmp x0, x2;"
		"bne L3%=;" : : [src] "r" ((unsigned long long int)base_ptr), [target] "r" (((unsigned long long int)base_ptr) + final_size) : "x0", "x2", "x3");
}

static int has_user_asked_for_help_message(int argc, char **argv, int arg_to_check) {
	return (argc > arg_to_check) && (argv[arg_to_check][0] == '-') && ((argv[arg_to_check][1] == 'h') || (argv[arg_to_check][1] == 'H')) && (argv[arg_to_check][2] == '\0');
}

int print_help(int argc, char **argv, size_t base_size, int base_iters) {
	int found_help = 0;
	for(int i = 1; i < argc; i++) {
		if(has_user_asked_for_help_message(argc, argv, i)) {
			found_help = 1;
			break;
		}
	}

	if(!found_help)
		return 0;

	printf("Usage: benchmark [footprint (in KBs)] [num_iters]");
	#ifdef PERF_COMPILATION
	printf("[perf_counters_to_measure]");
	#endif
	printf("\n");
	printf("Defaults:\n");
	printf("\tfootprint (in KBs): %llu\n", (long long unsigned int)(base_size / 1024));
	printf("\tnum_iters: %d\n", base_iters);
	#ifdef PERF_COMPILATION
	printf("\tperf_counters_to_measure: ");
	perf_default_events_printer();
	printf("\n");
	#endif
	return 1;
}
