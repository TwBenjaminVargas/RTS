#include "dsp.h"

int main(void)
{
    int j, pol;
    struct sched_param sp;

    pol = SCHED_RR; // SCHED_OTHER, SCHED_FIFO, SCHED_RR

    sp.sched_priority = 99;

    if (sched_setscheduler(0, pol, &sp) == -1)
        printf("error sched_setscheduler\n");


	gpioInitialise();

	if((myFd_AD = spiOpen(AD_SPI_CHAN,SPI_SPEED,SPI_MODE)) < 0){
		fprintf(stderr, "no se puede abrir el bus SPI: %s\n", strerror (errno));
		exit(EXIT_FAILURE);
	}
	
	if((myFd_DA = spiOpen(DA_SPI_CHAN,SPI_SPEED,SPI_MODE)) < 0){
		fprintf(stderr, "no se puede abrir el bus SPI: %s\n", strerror (errno));
		exit(EXIT_FAILURE);
	}	

	tickConfig(TS);
	
	for(;;)
		pause();
	
  return 0;
}


void tickHandler(int sig, siginfo_t *si, void *uc)
{
	unsigned short input;

	input = AD74XX_GetRegisterValue(myFd_AD);

	// procesamiento de muestras de entrada

	DAC121S101_SendRegisterValue(input, myFd_DA);

	return;
}


void tickConfig(unsigned int sampling_period){
    
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    struct sigaction sa;

    sa.sa_flags = 0;
    sa.sa_sigaction = tickHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1)
        printf("error sigaction");

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
        printf("error timer_create");

    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = sampling_period;

    if (timer_settime(timerid, 0, &its, NULL) == -1)
        printf("error timer_settime");
    
    return; 
}
