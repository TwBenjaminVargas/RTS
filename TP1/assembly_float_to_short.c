#include <stdio.h>

int main() {
    float f = 15.75f;
    short s;

    // __asm__ indica que entraremos en terreno de assembler
    // volatile evita que el compilador optimice y borre el código
    __asm__ volatile (
        "cvttss2si %1, %%eax \n\t" // Convierte float a entero de 32 bits (EAX)
        "movw %%ax, %0       \n\t" // Mueve los 16 bits bajos de EAX a nuestra variable 's'
        : "=r" (s)                 // Salida: la variable 's'
        : "m" (f)                  // Entrada: la variable 'f' en memoria
        : "%eax"                   // Registro que usamos (se ensucia)
    );

    printf("Float: %f -> Short en Assembler: %d\n", f, s);

    return 0;
}