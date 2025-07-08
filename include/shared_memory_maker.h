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

#ifndef __SHARED_MEM_MAKR_H__
#define __SHARED_MEM_MAKR_H__

// Expects wished alignment to be a power of 2!
int create_memory_region(const int size, void** out_ptr_real, void** out_ptr_usable, const int wished_alignment);
void destroy_memory_region(void* out_ptr_real);

#endif
