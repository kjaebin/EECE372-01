#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>        // needed for ioctl()
#include <asm/unistd.h>       // needed for perf_event syscall
#include <linux/perf_event.h> // needed for perf_event
#include <linux/hw_breakpoint.h> // needed for perf_event

int cnts_tick(int *fd) {
	int i, ret;
	char error = 0;
	struct perf_event_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.type = PERF_TYPE_HARDWARE;
	attr.size = sizeof(struct perf_event_attr);
	attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;

	printf("%lu %lu %lu %lu %lu %lu %lu %lu\n",
	PERF_TYPE_HARDWARE,
	PERF_FORMAT_TOTAL_TIME_ENABLED,
	PERF_FORMAT_TOTAL_TIME_RUNNING,
	PERF_COUNT_HW_CPU_CYCLES,
	PERF_COUNT_HW_INSTRUCTIONS,
	PERF_COUNT_HW_CACHE_MISSES,
	PERF_COUNT_HW_CACHE_REFERENCES,
	__NR_perf_event_open
	);

	attr.config = PERF_COUNT_HW_CPU_CYCLES;
	fd[0] = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
	attr.config = PERF_COUNT_HW_INSTRUCTIONS;
	fd[1] = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
	attr.config = PERF_COUNT_HW_CACHE_MISSES;
	fd[2] = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
	attr.config = PERF_COUNT_HW_CACHE_REFERENCES;
	fd[3] = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
	
	error |= (fd[0] == -1); 
	error |= (fd[1] == -1); 
	error |= (fd[2] == -1); 
	error |= (fd[3] == -1); 
	if (error) {perror("cannot open perf_counters"); exit(0);}

	for (i = 0; i < 4; i++) {
		ret = ioctl(fd[i], PERF_EVENT_IOC_RESET);
		if (ret == -1) return -1;
	}
	return 0;
}

int cnts_tock(int *fd, unsigned long long cnts[][3]) {
	int i, ret;

	for (i = 0; i < 4; i++) {
		ret = read(fd[i], cnts[i], sizeof(cnts[i]));
		if (ret != 24) return -1;
	}

	for (i = 0; i < 4; i++) close(fd[i]);

	return 0;
}

void cnts_print(unsigned long long cnts[][3]){

	float cycles, instructions;
	float cacheMisses, cacheRefs;
	cycles       = (float)cnts[0][0] * cnts[0][1] / cnts[0][2];
	instructions = (float)cnts[1][0] * cnts[1][1] / cnts[1][2];
	cacheMisses  = (float)cnts[2][0] * cnts[2][1] / cnts[2][2];
	cacheRefs    = (float)cnts[3][0] * cnts[3][1] / cnts[3][2];

	printf("[perf_event] cycles = %llu \n",
			(unsigned long long)cycles);
	printf("[perf_event] instructions = %llu (CPI=%0.2f)\n",
			(unsigned long long)instructions,
			cycles / instructions);
	printf("[perf_event] misses = %llu, references = %llu (miss rate=%0.4f)\n",
			(unsigned long long)cacheMisses,
			(unsigned long long)cacheRefs,
			cacheMisses / cacheRefs);


	return;
}
