
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "p3150026-p3150117-p3150148-res1.h"



int totalIncome, transactionCounter, totalWaitingTime, totalServiceTime, *seats, seatCounter, tilefonites;
unsigned int seedGl;


//mutexes
pthread_mutex_t lock, totInc, trCount, totWaitT, totServT, plan, pr;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;




void Outputs(int customers){


	for (int i = 0; i < N_seat; i++) {
		if(seats[i]==-1){
			printf("Seat %d / Empty\n", i+1);	
		}
		else{ 
			printf("Seat %d / Customer %d\n", i+1, *(seats + i));
		}
	}


	printf("\nTotal Income: %d euros.\n", totalIncome);

	printf("\nAverage waiting time: %f.\n", (double)totalWaitingTime/(double)customers);

	printf("\nAverage service time: %f.\n", (double)totalServiceTime/(double)customers);

	printf("\n");


}





void *Reservation(void *threadId) {

	struct timespec start,mid,end;
	int start_time, mid_time, end_time;
	clock_gettime(CLOCK_REALTIME,&start); 
	start_time = start.tv_sec;
	int *tid;
	tid = (int *)threadId;
	int rc;
	//printf("I am the customer with number %d!\n", *tid);

	int randomSeats = rand_r(&seedGl) % (N_seatHigh - N_seatLow + 1) + N_seatLow;
	int randomTime = rand_r(&seedGl) % (t_seatHigh - t_seatLow + 1) + t_seatLow;
	int randomSuc = rand_r(&seedGl) % 100;

	rc = pthread_mutex_lock(&lock);
	
	while(tilefonites==0){

		//printf("Customer %d, no available tilefonitis. Blocked...\n",*tid);
		rc = pthread_cond_wait(&cond,&lock);
	
	}

	clock_gettime(CLOCK_REALTIME,&mid);
	mid_time = mid.tv_sec;

		
	//printf("Customer %d getting service\n",*tid);
	tilefonites--;
	rc = pthread_mutex_unlock(&lock);

	rc = pthread_mutex_lock(&totWaitT);
	totalWaitingTime += (mid_time-start_time);
	rc = pthread_mutex_unlock(&totWaitT);
	
	sleep(randomTime);

	clock_gettime(CLOCK_REALTIME,&end);
	end_time = end.tv_sec;


	rc = pthread_mutex_lock(&totServT);
	totalServiceTime += (end_time-start_time);
	rc = pthread_mutex_unlock(&totServT);


	rc = pthread_mutex_lock(&plan);

	bool tmp = false;

	if (seatCounter == N_seat){
		printf("Πελάτης %d:\nΗ κράτηση ματαιώθηκε γιατί το θέατρο είναι γεμάτο.\n", *tid);
		tmp = true;
	}

	else if (seatCounter + randomSeats > N_seat){
		printf("Πελάτης %d:\nΗ κράτηση ματαιώθηκε γιατί δεν υπάρχουν διαθέσιμες θέσεις.\n", *tid);
		tmp = true;
	}

	else if (randomSuc > P_success-1){
		printf("Πελάτης %d:\nΗ κράτηση ματαιώθηκε γιατί η συναλλαγή με την πιστωτική κάρτα δεν έγινε αποδεκτή.\n", *tid);
		tmp = true;
	}

	if (tmp){

		rc = pthread_mutex_unlock(&plan);				
		rc = pthread_mutex_lock(&lock);
		//printf("Customer %d got service\n",*tid);
		tilefonites++;
		rc = pthread_cond_signal(&cond);
		rc = pthread_mutex_unlock(&lock);
		pthread_exit(tid);
	}
	
	for (int i=seatCounter; i<seatCounter + randomSeats ; i++){
		seats[i]=*tid;			
	}
	
	int tmp1 = seatCounter+1;
	seatCounter += randomSeats;
	int tmp2 = seatCounter;
	rc = pthread_mutex_unlock(&plan);


	rc = pthread_mutex_lock(&totInc);
	totalIncome += randomSeats*C_seat;
	rc = pthread_mutex_unlock(&totInc);

	rc = pthread_mutex_lock(&trCount);
	int count = transactionCounter;
	transactionCounter++;
	rc = pthread_mutex_unlock(&trCount);
	
	rc = pthread_mutex_lock(&pr);
	printf("Πελάτης %d:\nΗ κράτηση ολοκληρώθηκε επιτυχώς.\nΟ αριθμός συναλλαγής είναι <%d>", *tid, count);
	printf(", οι θέσεις σας είναι οι <%d",tmp1);
	for(int i=tmp1+1;i<=tmp2;i++){
		printf(", %d",i);
	}
	printf("> και το κόστος της συναλλαγής είναι <%d> ευρώ.\n",randomSeats*C_seat);
	rc = pthread_mutex_unlock(&pr);

	 
	rc = pthread_mutex_lock(&lock);
	//printf("Customer %d got service\n",*tid);
	tilefonites++;
	rc = pthread_cond_signal(&cond);
	rc = pthread_mutex_unlock(&lock);


	pthread_exit(tid);
}





int main(int argc, char **argv) { 
	
	int N_cust, seed;
	totalIncome = 0, transactionCounter = 0, totalWaitingTime = 0, totalServiceTime = 0, seatCounter = 0;
	tilefonites = N_tel;	


	if (argc != 3) {
		printf("ERROR: Provide two arguments.\n");
		return -1;
	}


	N_cust = atoi(argv[1]);
	seed = atoi(argv[2]);

	
	if (N_cust < 0) {
		printf("ERROR: the number of threads to run should be a positive number. Current number given %d.\n", N_cust);
		exit(-1);
	}
	

	printf("Customers: %d, Seed: %d.\n", N_cust, seed);
	
	seedGl = seed;	



	seats = (int *) malloc(sizeof(int) * N_seat);
	

	if (seats == NULL) {
		printf("ERROR: Malloc failed not enough memory!\n");
		return -1;
	}

	
	//arxikopoihsh twn timwn tou pinaka me -1
	for (int i = 0; i < N_seat; i++) {
		seats[i] = -1;
	}




	pthread_t *threads;

	threads = malloc(N_cust * sizeof(pthread_t));
	if (threads == NULL) {
		printf("NOT ENOUGH MEMORY!\n");
		return -1;
	}

	pthread_mutex_init(&lock,NULL);
	pthread_mutex_init(&totInc,NULL);
	pthread_mutex_init(&trCount,NULL);
	pthread_mutex_init(&totWaitT,NULL);
	pthread_mutex_init(&totServT,NULL);
	pthread_mutex_init(&plan,NULL);
	pthread_mutex_init(&pr,NULL);


	int rc;
   	int threadCount;
	int countArray[N_cust];
   	for(threadCount = 0; threadCount < N_cust; threadCount++) {
    	//printf("Main: creating thread %d\n", threadCount+1);
		countArray[threadCount] = threadCount + 1;
    	rc = pthread_create(&threads[threadCount], NULL, Reservation, &countArray[threadCount]);


    	if (rc != 0) {
    		printf("ERROR: return code from pthread_create() is %d\n", rc);
       		exit(-1);
       	}
   	}


	void *status;

	for (threadCount = 0; threadCount < N_cust; threadCount++) {
		rc = pthread_join(threads[threadCount], &status);
		
		if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}

		//printf("Main: Thread %d returned %d as status code.\n", countArray[threadCount], (*(int *)status));
	}
	
	

	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&totInc);
	pthread_mutex_destroy(&trCount);
	pthread_mutex_destroy(&totWaitT);
	pthread_mutex_destroy(&totServT);
	pthread_mutex_destroy(&plan);
	pthread_mutex_destroy(&pr);

	
	pthread_cond_destroy(&cond);



	Outputs(N_cust);


    	free(seats);
	free(threads);

	return 0;
	
}



