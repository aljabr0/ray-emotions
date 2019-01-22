#include "internal.h"

void rayem_init(){
	g_thread_init(NULL);
	g_type_init();
	rayem_gettickms();
	__rayem_sys_init();
}
