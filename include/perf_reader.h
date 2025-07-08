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

#ifndef __PERF_READER_H__
#define __PERF_READER_H__

#define FIND_CPU_FOR_ME -2

void perf_prepare(unsigned int curr_cpu);
void perf_purge(void);
void perf_print_results(int num_iters);
void perf_cleanup(void);
void perf_arg_reader(int argc, char** argv, int start_index_arg_counter_ids);

#endif
