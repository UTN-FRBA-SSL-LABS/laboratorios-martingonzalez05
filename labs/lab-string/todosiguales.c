#include <stdio.h>
#include "String.h"

/*
 * todosiguales — imprime 1 si todos los argumentos son iguales, 0 si no.
 *
 * Uso: ./todosiguales hola hola hola  →  1
 *      ./todosiguales hola mundo      →  0
 *
 * Pista: compara cada argumento contra argv[1] usando AreEqual.
 *        Iterá con puntero (char **arg), no con indice entero.
 */

int main(int argc, char *argv[]) {
    (void)argc;

    /* Si no hay argumentos adicionales (o hay 1 solo), se consideran todos iguales */
    if (argv[1] == NULL) {
        printf("1\n");
        return 0;
    }

    /* Compara cada argumento a partir de argv[2] contra el primero (argv[1]) */
    for (char **arg = argv + 2; *arg != NULL; arg++) {
        if (!AreEqual(*arg, argv[1])) {
            printf("0\n");
            return 0;
        }
    }

    printf("1\n");
    return 0;
}
