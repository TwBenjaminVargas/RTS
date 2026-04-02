#include <stdio.h>

int main() {
    // float (32 bits) a short (16 bits) ---
    float real;
    short entero_corto;

    printf("1) CONVERSION FLOAT A SHORT\n");
    printf("Ingrese un numero decimal (float): ");
    scanf("%f", &real);

    // Realizamos el cast (conversion explicita)
    entero_corto = (short)real;

    printf("Valor float ingresado: %f\n", real);
    printf("Valor short resultante: %d\n", entero_corto);
    
    // Nota: Si ingresas 40000.5, el short fallará porque su máximo es 32767.
    
    printf("\n------------------------------------\n\n");

    // --- PARTE 2: short a float ---
    short origen;
    float destino;

    printf("2) CONVERSION SHORT A FLOAT\n");
    printf("Ingrese un numero entero corto (-32768 a 32767): ");
    scanf("%hd", &origen); // %hd es el formato para short

    // Conversion en sentido contrario
    destino = (float)origen;

    printf("Valor short ingresado: %d\n", origen);
    printf("Valor float resultante: %f\n", destino);

    return 0;
}