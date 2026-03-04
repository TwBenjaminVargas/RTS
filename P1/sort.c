/*
 * Ordenador de arreglos numericos
 * Descripción: 
 *   Programa que recibe por línea de comandos una bandera ('a' para orden ascendente, 
 *   'd' para orden descendente) y una cadena de dígitos. 
 *   Convierte la cadena en un arreglo dinámico de enteros, lo ordena según la bandera
 *   usando bubble sort y luego imprime el resultado.
 *
 * Consideraciones:
 *   - Termina el programa con exit(1) si los parámetros son inválidos o falla la memoria.
 *   - El código no valida argc, por lo que si no se pasan suficientes argumentos puede fallar.
 *   - La función parse_int_array hace realloc en cada dígito, lo cual es ineficiente para cadenas largas.
 */
#include <stdio.h>
#include <stdlib.h>

int* parse_int_array(char*,int*);
void print_int_array(int *, int);
void bubble_sort_ascending(int *, int);
void bubble_sort_descending(int *, int);
void swap(int *, int*);

int main(int argc, char* argv[])
{
    char f = **(argv+1);
    int *arr, length=0;
    arr = parse_int_array(*(argv+2),&length);
    switch (f)
    {
    case 'a':
        bubble_sort_ascending(arr,length);
        break;
    case 'd':
        bubble_sort_descending(arr,length);
        break;
    default:
        printf("Invalid flag: %c\n",f);
        return 1;
    }
    print_int_array(arr,length);
    free(arr);
    return 0;
}

int* parse_int_array(char* str, int *length)
{
    if(*str == '\0') exit(1);
    int *arr = malloc(sizeof(int)), *tmp;
    if (arr == NULL)
        exit(1);
    int i;
    for(i=0;*(str+i) != '\0' && *(str+i) >= 48 && *(str+i) <= 57;i++)
    {
        *(arr+i) = *(str+i) - '0';
        if(*(str+i+1) != '\0')
        {
            tmp = realloc(arr,(i+2)*sizeof(int));
            if(!tmp)
            {
                free(arr);
                exit(1);    
            }
            arr=tmp;
        }    

    }
    if(*(str+i) >= 48 && *(str+i) <= 57)
    {
        free(arr);
        exit(1);
    }
    *length = i;
    return arr;
}

void swap(int *a, int*b)
{
    int tmp = *a;
    *a = *b;
    *b= tmp;
}
void bubble_sort_descending(int *arr, int length)
{
    int swapped;
    for(int i=0; i < length-1; i++)
    {
        swapped =0;

        for(int j=length-1;j>i;j--)
            if(*(arr+j) > *(arr+j-1))
            {
                swap((arr+j),(arr+j-1));
                swapped=1;
            }
        if(!swapped)
            break;
    }
}
void bubble_sort_ascending(int *arr, int length)
{
    int swapped;
    for(int i=0; i < length-1; i++)
    {
        swapped =0;

        for(int j=0;j<length-1-i;j++)
            if(*(arr+j) > *(arr+j+1))
            {
                swap((arr+j),(arr+j+1));
                swapped = 1;
            }
                
        if(!swapped)
            break;
    }
        
}


void print_int_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", *(arr+i));
    }
    printf("\n");
}