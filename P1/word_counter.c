/*
    Contador de Palabras
    Descripción:
        Este programa permite contar la cantidad de palabras a partir de:

            1) Un texto ingresado por parámetro.
            2) Un archivo de texto.

        La selección del modo se realiza mediante un flag:

            i  -> cuenta palabras en un string pasado por consola
            f  -> cuenta palabras dentro de un archivo

    Uso:

        ./programa i "texto a analizar"
        ./programa f archivo.txt

    Definición de palabra:
        Se considera palabra a cualquier secuencia de caracteres
        separada por el carácter espacio (ASCII 32).

    Consideraciones:
        - Solo el carácter espacio (' ') es tratado como separador.
        - No se contemplan tabs, saltos de línea ni otros espacios.
        - Si el archivo no puede abrirse, la función retorna -1.

*/
#include <stdio.h>
#include <stdlib.h>

int count_words(char*);
int count_words_file(char*);

int main (int argc, char *argv[])
{
    char f = **(argv + 1);
    int words = 0;
    switch (f)
    {
    case 'i':
        words = count_words(*(argv+2));
        break;
    case 'f':
        words = count_words_file(*(argv+2));
        break;
    default:
        printf("Invalid flag %c\n", f);
        return 1;
    }   
    printf("%d\n", words);
    return 0;
}

int count_words(char* str)
{
    int w = 0,p=32,i,c;
    for(i = 0; *(str + i) != '\0'; i++)
    {
        c= *(str + i);
        if(c == 32 && p != c)
                w++;
        p = c;
    }
    if (p!=32)
        w++;
    return w;
    
}

int count_words_file(char *filename)
{
    FILE *fp = fopen(filename,"r");
    if(!fp) return -1;

    int c,w=0,p=32;
    while((c= fgetc(fp))!= EOF)
    {
        if(c == 32 && p != c)
                w++;
        p=c;
    }
    if (p != 32)
        w++;

    fclose(fp);
    return w;

}
