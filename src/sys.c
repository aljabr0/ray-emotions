#include "internal.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/resource.h>

#define SYSFS_DEVCPU	"/sys/devices/system/cpu"
#define STAT			"/proc/stat"
#define MAX_PF_NAME		1024

int __rayem_detected_cpus=1;

static int get_sys_cpu_nr(void){
	DIR *dir;
	struct dirent *drd;
	struct stat buf;
	char line[MAX_PF_NAME];
	int proc_nr=0;

	/* Open relevant /sys directory */
	if((dir=opendir(SYSFS_DEVCPU))==NULL)return 0;

	/* Get current file entry */
	while((drd=readdir(dir))!=NULL){
		if (!strncmp(drd->d_name,"cpu",3) && isdigit(drd->d_name[3])){
			snprintf(line,MAX_PF_NAME,"%s/%s",SYSFS_DEVCPU,drd->d_name);
			line[MAX_PF_NAME - 1]='\0';
			if(stat(line,&buf)<0)continue;
			if(S_ISDIR(buf.st_mode)){
				proc_nr++;
			}
		}
	}

	/* Close directory */
	closedir(dir);

	return proc_nr;
}

static int get_proc_cpu_nr(void){
	FILE *fp;
	char line[16];
	int num_proc,proc_nr=-1;

	if((fp=fopen(STAT,"r"))==NULL){
		return -1;
	}

	while(fgets(line,sizeof(line),fp)!=NULL){
		if(strncmp(line,"cpu ",4) && !strncmp(line,"cpu",3)){
			sscanf(line+3,"%d",&num_proc);
			if(num_proc>proc_nr)proc_nr=num_proc;
		}
	}
	fclose(fp);
	return (proc_nr+1);
}

gboolean __rayem_sys_init(){
	int c=rayem_sys_cpu_count();
	if(c<=0){
		fprintf(stderr,"%s error detecting cpu count\n",__func__);
		return FALSE;
	}else{
		fprintf(stderr,"%s detected %d cpus\n",__func__,c);
		__rayem_detected_cpus=c;
		return TRUE;
	}
}

int rayem_sys_cpu_count(){
	int cpu_nr;
	if((cpu_nr=get_sys_cpu_nr())==0){
		cpu_nr=get_proc_cpu_nr();
	}
	return cpu_nr;
}

void rayem_sys_print_mem_statistics(){
	struct rusage usage;
	if(getrusage(RUSAGE_SELF,&usage)){
		fprintf(stderr,"syscall error: getrusage\n");
		return;
	}
	fprintf(stderr,"sys resource usage:\n");
	fprintf(stderr,"\tpage faults: %ld\n",usage.ru_majflt);
	fprintf(stderr,"\tswaps: %ld\n",usage.ru_nswap);
	fprintf(stderr,"\tvoluntary context switches: %ld\n",usage.ru_nvcsw);
	fprintf(stderr,"\tinvoluntary context switches: %ld\n",usage.ru_nivcsw);
}
