#define _GNU_SOURCE
#include <stdio.h> /* Input/Output */
#include <pthread.h> 
#include <sched.h>
#include <errno.h> /* Errors */
#include <stdlib.h> /* General Utilities */
#include <unistd.h> /* Symbolic Constants */
#include <sys/types.h>
#include <time.h>
#include <string.h>

//pthread_barrier
pthread_barrier_t barrier;

typedef struct{
	pthread_t thread_id;
	int thread_num;
	int sched_policy;
	int sched_priority;
	double time_wait;
}thread_info_t;	

void *thread_func(void *arg){

	thread_info_t *data;
	data = (thread_info_t *)arg;
	
	//set cpu affinity
	int cpu_id = 0;
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(cpu_id,&set);
	pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),&set);
	
	//pthread_barrier wait
	pthread_barrier_wait(&barrier);
	
	for(int i=0;i<3;i++){
		printf("Thread %d is running\n",data->thread_num);
		clock_t start_t = clock(); // now
		while((double)(clock()-start_t) / CLOCKS_PER_SEC < data->time_wait){
		}
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int opt;
	int num_threads = 0;
	char* policies = NULL;
	char* priorities = NULL;
	double time_wait = 0.0;
	
	/*parse program arguments*/
	while((opt =getopt(argc,argv,"n:t:s:p:")) !=-1){
		switch(opt){
			case 'n':
				char * endptr;
				num_threads = strtol(optarg,&endptr,10);
				if(*endptr!='\0'){
					printf("error trans\n");
				}
				break;
			case 't':
				time_wait = atof(optarg);
				break;
			case 's':
				policies = optarg;
				break;
			case 'p':
				priorities = optarg;
				break;
			case '?':
				printf("Unknown option: %c\n",optopt);
				exit(EXIT_FAILURE);
		}
	}
	
	/*create num_threads worker threads*/
	thread_info_t *thread_info = (thread_info_t *)malloc(num_threads*sizeof(thread_info_t));
	memset(thread_info,0,sizeof(thread_info));
	
	//substr
	char *delim = ",";
	char *dubstr_policies = strdup(policies);
	char *dubstr_priorities = strdup(priorities);
	char *saveptr_policies = NULL;
	char *substr_policies = NULL;
	char *saveptr_priorities = NULL;
	char *substr_priorities = NULL;
	int rc;
	void *status;
	//pthread_barrier init
	pthread_barrier_init(&barrier,NULL,num_threads);
	
	for(int i = 0; i<num_threads;i++){
	
		substr_policies =(i!=0)?strtok_r(NULL,delim,&saveptr_policies):strtok_r(dubstr_policies,delim,&saveptr_policies);
		substr_priorities =(i!=0)?strtok_r(NULL,delim,&saveptr_priorities):strtok_r(dubstr_priorities,delim,&saveptr_priorities);
		
		//set thread info
		thread_info[i].thread_num=i;
		thread_info[i].time_wait = time_wait;
		thread_info[i].sched_policy = (strcmp(substr_policies,"FIFO")==0)? SCHED_FIFO:SCHED_OTHER;
		thread_info[i].sched_priority = atoi(substr_priorities);
		//printf argument
		//printf("usage  -n<%d> -t<%lf> -s<%d> -p<%d> \n",thread_info[i].thread_num,thread_info[i].time_wait,thread_info[i].sched_policy,thread_info[i].sched_priority);
		if(thread_info[i].sched_policy == 0){
			thread_info[i].sched_priority = 0;
		}
		// pthread_attr and init
		pthread_attr_t attr;
		pthread_attr_init(&attr);		
		//SET scheduler policy
		pthread_attr_setschedpolicy(&attr,thread_info[i].sched_policy);
		//SET sched_param
		struct sched_param sp;
		sp.sched_priority = thread_info[i].sched_priority;
		//sched_param write in attr
		pthread_attr_setschedparam(&attr,&sp);
		//thread create
		rc = pthread_create(&thread_info[i].thread_id,&attr,thread_func,&thread_info[i]);
		if(rc!=0){
			printf("Error:unable to create thread %d\n",i);
			exit(-1);
		}
		//dynamic set scheduler policy
		pthread_setschedparam(thread_info[i].thread_id,thread_info[i].sched_policy,&sp);
		// pthread attr destroy
		pthread_attr_destroy(&attr);
	}
	
	for(int j = 0;j<num_threads;j++){
		rc = pthread_join(thread_info[j].thread_id,&status);
		if(rc){
			printf("Error:unable to join returns %d\n",rc);
			exit(-1);
		}
	}
	//pthread_barrier destroy
	pthread_barrier_destroy(&barrier);
	free(thread_info);
	pthread_exit(NULL);
	return 0;
}

