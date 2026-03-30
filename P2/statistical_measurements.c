#define _DEFAULT_SOURCE
#include <stdio.h>
#include <pigpio.h>
#include <signal.h>


#define LED_PIN       24
#define BUTTON_PIN    23
#define DEBOUNCE_TIME 200
#define MAX_SAMPLES   100 


uint32_t latencies[MAX_SAMPLES];
int sampleCount = 0;
volatile sig_atomic_t keepRunning = 1;

void signalHandler(int signum) {
    keepRunning = 0;
}
// Posible mejora: No usar funciones tan complejas en un ISR
void interrupt_handler(int gpio, int level, uint32_t tick_hardware) {
    
    uint32_t tick_software = gpioTick();
    
    
    gpioWrite(LED_PIN, !level);
    
    
    if (level == 0 && sampleCount < MAX_SAMPLES) {
        
        uint32_t latency = tick_software - tick_hardware;
        
        latencies[sampleCount] = latency;
        sampleCount++;

        printf("\rPulse %d/%d | Latency: %u us    ", sampleCount, MAX_SAMPLES, latency);
        fflush(stdout);
    }
}

int main() {
    signal(SIGINT, signalHandler);
    
    if (gpioInitialise() < 0) {
        printf("Error: Failed to initialise pigpio\n");
        return 1;
    }

    gpioSetMode(LED_PIN, PI_OUTPUT);
    gpioSetMode(BUTTON_PIN, PI_INPUT);
    //gpioSetPullUpDown(BUTTON_PIN, PI_PUD_DOWN);

    // Interrupt configuration with hardware debounce
    gpioSetISRFunc(BUTTON_PIN, EITHER_EDGE, DEBOUNCE_TIME, interrupt_handler);

    printf("System ready. Please perform at least 20 pulses.\n");
    printf("Press Ctrl+C to stop and view the statistical analysis.\n\n");

    while (keepRunning) {
        gpioDelay(100000); // 100ms sleep to save CPU
    }

    // STATISTICAL ANALYSIS
    if (sampleCount > 0) {
        uint32_t minLatency = UINT32_MAX;
        uint32_t maxLatency = 0;
        double sumLatency = 0;

        printf("\n\n--- TABULATED RESULTS ---\n");
        printf("ID\tLatency (us)\n");
        printf("------------------------\n");

        for (int i = 0; i < sampleCount; i++) {
            printf("%d\t%u\n", i + 1, latencies[i]);
            
            if (latencies[i] < minLatency) minLatency = latencies[i];
            if (latencies[i] > maxLatency) maxLatency = latencies[i];
            sumLatency += latencies[i];
        }

        double averageLatency = sumLatency / sampleCount;
        uint32_t jitter = maxLatency - minLatency;

        printf("------------------------\n");
        printf("Minimum Latency:   %u us\n", minLatency);
        printf("Maximum Latency:   %u us\n", maxLatency);
        printf("Average Latency:   %.2f us\n", averageLatency);
        printf("Jitter (Variation): %u us\n", jitter);
        printf("Total Samples:     %d\n", sampleCount);
        
        if (sampleCount < 20) {
            printf("\nNOTE: At least 20 samples are recommended for a solid analysis.\n");
        }
    } else {
        printf("\nNo pulses were recorded.\n");
    }

    gpioWrite(LED_PIN, 0);
    gpioTerminate();
    printf("Program terminated correctly.\n");
    return 0;
}