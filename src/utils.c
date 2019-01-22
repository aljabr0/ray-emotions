#include "internal.h"

void print_pointer_gfunc(gpointer data,gpointer user_data){
	fprintf((FILE *)user_data,"%p\n",data);
}
