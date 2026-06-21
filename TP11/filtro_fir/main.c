#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "lp400_tfilter.h"
#include "test_signal.h"

#define NS 4000
#define FS 8000
#define PI 3.1415926

//#define NOISE
#define SIGNAL

int pseudo_noise_seq(void);


int main()
{
	long n, i;
	float fir_out;
	float fir_input[N_FIR+1] = {0.0};

	FILE *fir_out_f;
	fir_out_f = fopen("fir_out.txt", "w");

	srand(time(NULL));

	for (n = 0; n < NS; n++)
	{
		#if defined (NOISE)
		fir_input[0] = pseudo_noise_seq();
		#elif defined (SIGNAL)
		fir_input[0] = test_signal[n];
		#endif
		
		fir_out = 0;
		for(i = N_FIR-1; i >= 0; i--)
		{
    		    fir_out += (h[i] * fir_input[i]);
    		    fir_input[i+1] = fir_input[i];
		}

		fprintf(fir_out_f, "\n %10f %10f", (float)n/FS, fir_out);
	}

	fclose(fir_out_f);
}

int pseudo_noise_seq()  
{   
   int prnseq;  
    
   prnseq = -8000 + rand()%16001;

   return prnseq;   
}
