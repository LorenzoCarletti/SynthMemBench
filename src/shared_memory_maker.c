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
#include "shared_memory_maker.h"

// Wished alignment must be a power of 2 for this code to work
// Could be expanded to support more sizes, in theory...
static void* force_align_area(void* base_ptr, const int wished_alignment) {
	return (void*)((((uintptr_t)base_ptr) + wished_alignment - 1) & (~(wished_alignment - 1)));
}

int create_memory_region(const int size, void** out_ptr_real, void** out_ptr_usable, const int wished_alignment) {
	*out_ptr_real = malloc(size + wished_alignment);
	if((*out_ptr_real) == NULL) {
		printf("MALLOC ERROR!\n");
		return -1;
	}
	*out_ptr_usable = force_align_area(*out_ptr_real, wished_alignment);
	return 0;
}

void destroy_memory_region(void* out_ptr_real) {
	free(out_ptr_real);
}
