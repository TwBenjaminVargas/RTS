/*
 * Reloj Digital en Consola
 * DESCRIPCIÓN: Muestra la fecha y hora actual actualizándose en la misma línea.
 * * NOTAS TÉCNICAS:
 * - Usa '\r' para sobreescribir la línea actual.
 * - Usa secuencias ANSI para el color amarillo.
 * - Requiere fflush() para forzar la salida del buffer sin usar saltos de línea.
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main()
{
    time_t time_unix_epoch;
    struct tm *format_time;

    const char *AMARILLO = "\033[33m";
    const char *RESET = "\033[0m";

    while(1)
    {
        time(&time_unix_epoch);
        format_time = localtime(&time_unix_epoch);
        printf("\r%02d/%02d/%d %s%02d:%02d:%02d%s",
            format_time->tm_mday, 
            format_time->tm_mon + 1,
            format_time->tm_year + 1900,
            AMARILLO,
            format_time->tm_hour, 
            format_time->tm_min, 
            format_time->tm_sec,
            RESET);
        fflush(stdout);
        usleep(100000);
    }
    return 0;
}