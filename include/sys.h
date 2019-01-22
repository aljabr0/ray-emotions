#ifndef SYS_H_
#define SYS_H_

#include "internal.h"

extern int __rayem_detected_cpus;
#define rayem_detected_cpus	((const int)__rayem_detected_cpus)

int rayem_sys_cpu_count();
void rayem_sys_print_mem_statistics();

gboolean __rayem_sys_init();

#endif /* SYS_H_ */
