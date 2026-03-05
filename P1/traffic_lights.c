/*
Simulación simple de un semáforo utilizando hilos, señales y mutex.

El programa ejecuta dos hilos:

1) keyboard_listener
   Escucha la entrada del teclado. Si el usuario presiona 's', genera la
   señal SIGUSR1 para indicar que se solicita un cruce peatonal.

2) ligth_pattern_control
   Controla el ciclo del semáforo (RED → YELLOW → GREEN). Durante el estado
   GREEN revisa si existe una solicitud de parada. Si stop_required es 1,
   reinicia el ciclo desde RED.

La variable global stop_required es compartida entre hilos y se protege con
un mutex para evitar condiciones de carrera.

La señal SIGUSR1 es capturada por handle_stop_signal, que activa la variable
stop_required.

Nota: el algoritmo está intencionalmente más complejo de lo necesario ya que
se realizó como ejercicio práctico para experimentar con hilos, señales y
secciones críticas. El problema podría resolverse con una lógica más simple.
*/

#define RED_TIME_SECONDS 5
#define YELLOW_TIME_SECONDS 2
#define GREEN_TIME_SECONDS 10

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

pthread_mutex_t mutex;
int stop_required = 0;

void* ligth_pattern_control(void* arg)
{
    while (1)
    {
        reboot:
            printf("RED\n");
            sleep(RED_TIME_SECONDS);
            printf("YELLOW\n");
            sleep(YELLOW_TIME_SECONDS);
            printf("GREEN\n");
            sleep(GREEN_TIME_SECONDS/2);
            pthread_mutex_lock(&mutex);
            if(stop_required)
            {
                printf("stop given\n");
                stop_required = 0;
                pthread_mutex_unlock(&mutex);
                goto reboot;
            }
            pthread_mutex_unlock(&mutex);
            sleep(GREEN_TIME_SECONDS/2);

        
    }
    
}

void* keyboard_listener(void* arg)
{
    while(1)
    {
        char c = getchar();
        if(c == 's')
        {
            printf("stop required\n");
            raise(SIGUSR1);

        }
    }
}

void handle_stop_signal(int sig)
{
    pthread_mutex_lock(&mutex);
    stop_required = 1;
    pthread_mutex_unlock(&mutex);    
}

int main()
{
    pthread_t t1,t2;
    
    signal(SIGUSR1, handle_stop_signal);

    printf("Semaforo corriendo, presiona s para cruce peatonal:\n");
    pthread_create(&t1,NULL,keyboard_listener,NULL);
    pthread_create(&t2,NULL,ligth_pattern_control,NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;

}