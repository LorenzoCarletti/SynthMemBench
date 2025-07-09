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
#include <stdlib.h>
#include <perf/utilities.h>
#include "perf_reader.h"

// Adapted from: https://github.com/AlexGustafsson/perf/blob/main/examples/full/harness.c

// Hardware limit for our case
#define MAX_PERF_EVENTS_CPU 6

extern int getcpu (unsigned int *, unsigned int *) __THROW;

// This structure is returned each time the main measurement is read.
// It contains the result of all the measurements, taken simultaneously.
// The order of the measurements is unknown - used the ID.
typedef struct {
	uint64_t recorded_values;
	struct {
		uint64_t value;
		uint64_t id;
	} values[MAX_PERF_EVENTS_CPU + 1];
} measurement_t;
// Why is this not in the main library...?

// L2D_CACHE_REFILL, L2D_CACHE_WB, "Attributable Performance Impact Event Counts every
// cycle there is a stall in the Wr stage because of a load miss",
// "Data Write operation that stalls the pipeline because the store buffer is full",
// LD_RETIRED, ST_RETIRED
static int counter_ids[MAX_PERF_EVENTS_CPU] = { 0x17, 0x18, 0xE7, 0xC7, 6, 7 };
static uint64_t counter_file_ids[MAX_PERF_EVENTS_CPU];
static uint64_t counter_last_values[MAX_PERF_EVENTS_CPU] = { 0 };
static perf_measurement_t* events_measurement[MAX_PERF_EVENTS_CPU + 1];
static measurement_t events_result;

static void assert_support() {
	// Print the kernel version
	int major, minor, patch;
	int status = perf_get_kernel_version(&major, &minor, &patch);
	if (status < 0) {
		perf_print_error(status);
		exit(EXIT_FAILURE);
	}

	// Exit if the API is unsupported
	status = perf_is_supported();
	if (status < 0) {
		perf_print_error(status);
		exit(EXIT_FAILURE);
	} else if (status == 0) {
		fprintf(stderr, "error: perf not supported\n");
		exit(EXIT_FAILURE);
	}
}

static void prepare_measurement(int description, perf_measurement_t *measurement, perf_measurement_t *parent_measurement) {
	int status = perf_has_sufficient_privilege(measurement);
	if (status < 0) {
		perf_print_error(status);
		exit(EXIT_FAILURE);
	} else if (status == 0) {
		fprintf(stderr, "error: unprivileged user\n");
		exit(EXIT_FAILURE);
	}

	int support_status = perf_event_is_supported(measurement);
	if (support_status < 0) {
		perf_print_error(support_status);
		exit(EXIT_FAILURE);
	} else if (support_status == 0) {
		fprintf(stderr, "warning: event %d not supported\n", description);
		return;
	}

	int group = parent_measurement == NULL ? -1 : parent_measurement->file_descriptor;

	status = perf_open_measurement(measurement, group, 0);
	if (status < 0) {
		perf_print_error(status);
		exit(EXIT_FAILURE);
	}
}

void perf_prepare(unsigned int curr_cpu) {
	int i = 0;
	int curr_node = 0;
	if(curr_cpu == FIND_CPU_FOR_ME) {
		int err = getcpu(&curr_cpu, &curr_node);
		if(err < 0) {
			fprintf(stderr, "ERROR CPU GET!\n");
			return;
		}
	}

	// Fail if the perf API is unsupported
	assert_support();
	for(i = 0; i < (MAX_PERF_EVENTS_CPU + 1); i++)
		events_measurement[i] = NULL;

	// Create a dummy measurement (measures nothing) to act as a group leader
	events_measurement[0] = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_DUMMY, 0, curr_cpu);
	prepare_measurement(PERF_COUNT_SW_DUMMY, events_measurement[0], NULL);

	for(i = 0; i < MAX_PERF_EVENTS_CPU; i++) {
		if(counter_ids[i] != -1) {
			events_measurement[i + 1] = perf_create_measurement(PERF_TYPE_RAW, counter_ids[i], 0, curr_cpu);
			events_measurement[i + 1]->attribute.exclude_kernel = 1;
			prepare_measurement(counter_ids[i], events_measurement[i + 1], events_measurement[0]);
			counter_file_ids[i] = events_measurement[i + 1]->id;
		}
	}
	perf_start_measurement(events_measurement[0]);
}

void perf_purge() {
	perf_stop_measurement(events_measurement[0]);
	perf_read_measurement(events_measurement[0], &events_result, sizeof(measurement_t));
	measurement_t *measurements = &events_result;
	for (uint64_t j = 0; j < measurements->recorded_values; j++) {
		for (int k = 0; k < MAX_PERF_EVENTS_CPU; k++) {
			if (measurements->values[j].id == counter_file_ids[k]) {
				counter_last_values[k] = measurements->values[j].value;
				break;
			}
		}
	}
	perf_start_measurement(events_measurement[0]);
}

void perf_print_results(int num_iters) {
	measurement_t *measurements = &events_result;
	uint64_t values[MAX_PERF_EVENTS_CPU + 1] = {0};

	for (uint64_t j = 0; j < measurements->recorded_values; j++) {
		for (int k = 0; k < MAX_PERF_EVENTS_CPU; k++) {
			if (measurements->values[j].id == counter_file_ids[k]) {
				values[k] = measurements->values[j].value - counter_last_values[k];
				break;
			}
		}
	}

	printf("TOTAL EVENTS:\n\t");
	for (int k = 0; k < MAX_PERF_EVENTS_CPU; k++) {
		// Ignore the results from the dummy counter
		printf("Event %d: %lu, ", counter_ids[k], values[k]);
	}

	printf("\nPER ITERATION EVENTS:\n\t");
	for (int k = 0; k < MAX_PERF_EVENTS_CPU; k++) {
		// Ignore the results from the dummy counter
		printf("Event %d: %f, ", counter_ids[k], ((float)values[k]) / num_iters);
	}
	printf("\n");
}

void perf_cleanup() {
	int i = 0;
	perf_stop_measurement(events_measurement[0]);
	perf_read_measurement(events_measurement[0], &events_result, sizeof(measurement_t));
	for(i = 0; i < MAX_PERF_EVENTS_CPU; i++) {
		if(events_measurement[i] != NULL) {
			perf_close_measurement(events_measurement[i]);
			free((void*)events_measurement[i]);
			events_measurement[i] = NULL;
		}
	}
	if(events_measurement[0] != NULL) {
		perf_close_measurement(events_measurement[0]);
		free((void*)events_measurement[0]);
		events_measurement[0] = NULL;
	}
}

void perf_arg_reader(int argc, char** argv, int start_index_arg_counter_ids) {
	for(int i = 0; ((i + start_index_arg_counter_ids) < argc) && (i < MAX_PERF_EVENTS_CPU); i++) {
		int curr_event_id = atoi(argv[i + start_index_arg_counter_ids]);
		counter_ids[i] = curr_event_id;
	}
}

void perf_default_events_printer() {
	for (int k = 0; k < MAX_PERF_EVENTS_CPU; k++)
		printf("%d ", counter_ids[k]);
}
