#include "internal.h"

//TODO merge with libdspserver

#define MAX_CACHED_PRIMES	16
static int prime_table[MAX_CACHED_PRIMES];

static gboolean is_prime(int number){
	gboolean p=TRUE;
	int i;
	for(i=2;i<number;i++){
		if(number%i==0){
			p=FALSE;
			break;
		}
	}
	if(number==2){
		p=FALSE;
	}
	return p;
}

static int find_prime(int index){
	int prime=g_atomic_int_get(&prime_table[index]);
	if(prime>0)return prime;
	if(index<1)return -1;
	prime=1;
	int found=1;
	while(found!=index){
		prime+=2;
		if(is_prime(prime)){
			found++;
		}
	}
	g_atomic_int_set(&prime_table[index],prime);
	return prime;
}

inline double rayem_prand_sequ_get_number(int index,int count,int dimension){
	return drand48();
}

double rayem_hammersley_sequ_get_number(int index,int count,int dimension){
	g_assert(index>=0 && dimension>=1);
	if(dimension==1){
		return (double)(index+1)/(double)count;
	}
	return rayem_halton_sequ_get_number(index,count,dimension-1);
}

double rayem_halton_sequ_get_number(int index,int count,int dimension){
	index++;
	int base=find_prime(dimension);
	if(base==1){
		base++;//the first dimension uses base 2.
	}
	double remainder;
	double output=0.0;
	double fraction=1.0/(double)base;
	int N1=0;
	int copyOfIndex=index;
	if(base>=2 && index>=1){
		while(copyOfIndex>0){
			N1=(copyOfIndex/base);
			remainder=copyOfIndex%base;
			output+=fraction*remainder;
			copyOfIndex=(int)(copyOfIndex/base);
			fraction/=(double)base;
		}
		return output;
	}else{
		g_assert_not_reached();
		return 0.0;
	}
}

inline double rayem_sequgen_get_number1(int index,int count,int dimension){
	return ((rayem_halton_sequ_get_number(index,count,dimension)-0.5)/0.5);
}

void rayem_sequgen_get_v3d(int index,int count,vector3dp v){
	v->x=rayem_sequgen_get_number1(index,count,1);
	v->y=rayem_sequgen_get_number1(index,count,2);
	v->z=rayem_sequgen_get_number1(index,count,3);
}
void rayem_sequgen_get_2v3d(int index,int count,vector3dp v1,vector3dp v2){
	v1->x=rayem_sequgen_get_number1(index,count,1);
	v1->y=rayem_sequgen_get_number1(index,count,2);
	v1->z=rayem_sequgen_get_number1(index,count,3);
	v2->x=rayem_sequgen_get_number1(index,count,4);
	v2->y=rayem_sequgen_get_number1(index,count,5);
	v2->z=rayem_sequgen_get_number1(index,count,6);
}
