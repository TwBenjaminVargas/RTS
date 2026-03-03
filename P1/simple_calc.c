/*
 * Calculadora simple por línea de comandos.
 *
 * Uso:
 *   ./calc <num1> <num2> <op>
 *
 * Donde:
 *   <num1> y <num2> son números reales válidos.
 *   <op> es un carácter:
 *        'a' → suma
 *        's' → resta
 *        'm' → multiplicación
 *        'd' → división
 *
 * El programa valida que los operandos sean numéricos.
 * Si ocurre un error, termina con código distinto de 0.
 */

#include <stdio.h>
#include <stdlib.h>

double get_val(char*);

int main(int argc, char *argv[])
{
    char* endptr;
    double val1 = get_val(*(argv + 1));
    double val2 = get_val(*(argv + 2));
    double res = 0; 
    char op = **(argv + 3);

    switch (op)
    {
    case 'a':
        res = val1 + val2;     
        break;
    case 'm':
        res= val1 * val2;
        break;
    case 'd':
        res= val1/val2;
        break;
    case 's':
        res= val1 - val2;
        break;
    default:
        printf("Invalid operator\n");
        return 1;
    }

    printf("%.2f\n",res);
    return 0;
}

double get_val(char *str_ptr)
{
    char* endptr;
    double val = strtod(str_ptr, &endptr);
    
    if(str_ptr == endptr || *endptr != '\0')
    {
        printf("Invalid operand: %s\n", str_ptr);
        exit(1);
    }
    
    return val;
}