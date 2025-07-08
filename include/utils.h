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

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**************************** Type Definitions ******************************/
typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define PAGESIZE 4*1024

typedef struct{
	u64 phys_addr;
	u64 virt_addr;
} addr_t;

#ifdef INFINITE_RUN
#define CAN_CONTINUE_ITERS(x, y) (1)
#else
#define CAN_CONTINUE_ITERS(x, y) ((x) < (y))
#endif

void load_array(void* base_ptr, int final_size);
int print_help(int argc, char **argv);

#endif
