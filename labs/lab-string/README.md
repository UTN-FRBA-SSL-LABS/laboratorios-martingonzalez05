# Laboratorio: Strings en C

---

## Verificación y calificación

Desde la raíz del repositorio ejecutá:

```bash
make grade LAB=lab-string
```

Si ya estás dentro de esta carpeta también podés usar `make test`. Ambos
comandos ejecutan los mismos checks y muestran el puntaje sin necesidad de hacer
push. La verificación oficial se ejecuta cuando cambia `labs/lab-string/`; la
nota queda en el resumen y en los artefactos de GitHub Actions.

No modifiques `.github/`, `scripts/` ni `test_local.sh`: son infraestructura
docente y el workflow restaura automáticamente su versión oficial.

---

## Antes de empezar

Para trabajar local necesitás GCC y Make:
- **Linux:** `sudo apt install gcc make`
- **macOS:** `xcode-select --install`
- **Windows:** WSL con Ubuntu recomendado

Sin instalar nada: **GitHub Codespaces** → botón verde `<> Code` → pestaña `Codespaces`.

Verificá que el proyecto compila desde el estado inicial:

```bash
make test
make all
```

Deberías ver que todo compila sin errores.

---

## Restricciones — leer antes de arrancar

Aplican a **todo** el laboratorio sin excepción:

| Restricción | Ejemplo ❌ | Ejemplo ✅ |
|---|---|---|
| No usar `<string.h>` | `strlen(s)` | `GetLength(s)` |
| No usar `<stdlib.h>` para conversión | `atoi(s)` | `ToInteger(s)` |
| Usar `const` donde el parámetro no se modifica | `int f(char *s)` | `int f(const char *s)` |
| Tests con literales, no variables | `char *s="x"; assert(f(s)==1)` | `assert(f("x")==1)` |
| Tests sin salida a stdout | `printf("ok\n")` en test | solo `assert(...)` |
| Iterar con puntero en Parte III | `for (int i=1; i<argc; i++)` | `for (char **arg=argv+1; ...)` |

---

## Parte I — Análisis Comparativo: C vs Python

Esta sección compara cómo cada lenguaje trata el tipo string. Leé cada punto antes de arrancar con el código — entender estas diferencias es el contexto del laboratorio.

---

### 1. ¿El tipo string es parte del lenguaje en algún nivel?

**C:** No existe un tipo `string` en el lenguaje. Un string es un arreglo de `char` con una convención: termina en el carácter nulo `'\0'`. El lenguaje provee literales de cadena (`"hola"`) que son objetos reales almacenados en memoria estática de solo lectura, y permite inicializar arreglos con ellos — pero no hay un tipo string propiamente dicho ni operadores que operen sobre él.

**Python:** El tipo string `str` es nativo del lenguaje con soporte sintáctico de primera clase: literales con comillas simples o dobles, triple-quoted strings, f-strings, y operadores `+`, `*`, `in`, entre otros.

---

### 2. ¿El tipo string es parte de la biblioteca?

**C:** `<string.h>` provee funciones para operar sobre strings (`strlen`, `strcpy`, `strcmp`, etc.), pero el tipo string en sí no pertenece a ninguna biblioteca — es solo la convención `char *` terminada en `'\0'`. En este laboratorio no usamos `<string.h>`: implementamos las operaciones nosotros.

**Python:** El tipo `str` está definido en el runtime de Python. Sus métodos (`upper()`, `split()`, `find()`, etc.) son parte del objeto mismo, no de una biblioteca separada que haya que importar.

---

### 3. ¿Qué alfabeto usa?

**C:** Cada `char` ocupa 1 byte y representa un valor entre 0 y 255. El estándar solo garantiza ASCII (0–127). UTF-8 se puede almacenar como bytes, pero el lenguaje no tiene awareness de codificación: `strlen` cuenta bytes, no caracteres.

**Python:** Desde Python 3, `str` usa Unicode. Cada carácter es un code point Unicode (puede representar cualquier símbolo del mundo). Internamente Python elige entre UTF-8, UCS-2 o UCS-4 según el contenido, pero esto es transparente al programador.

---

### 4. ¿Cómo se resuelve la alocación de memoria?

Antes de responder, conviene tener claro dónde puede vivir la memoria en un programa C:

- **Stack (pila):** cada vez que se llama a una función, el sistema reserva automáticamente un bloque de memoria llamado *stack frame* para sus variables locales. Cuando la función retorna, ese frame se destruye y toda la memoria que contenía deja de ser válida. Es rápido pero tiene vida útil limitada.
- **Heap (montículo):** memoria de larga duración que el programador reserva explícitamente con `malloc` y libera con `free`. Vive más allá del retorno de las funciones, pero el programador es responsable de liberarla.
- **Memoria estática:** zona reservada al inicio del programa para literales y variables globales. Vive toda la ejecución.

**C:** Manual. El programador decide dónde vive el string:
- En el **stack**: `char s[] = "hola"` — válido solo mientras el stack frame de la función existe.
- En memoria **estática**: un literal `"hola"` — vive toda la ejecución, pero es de solo lectura.
- En el **heap**: `malloc(n)` — el programador debe liberar con `free` cuando ya no lo necesite.

**Python:** Automática. El garbage collector maneja la vida útil de cada objeto string: cuando ninguna variable referencia un string, su memoria se libera sola. El programador nunca llama a `malloc` ni `free`.

---

### 5. ¿El tipo tiene mutabilidad o es inmutable?

Un tipo es **mutable** si se pueden cambiar sus contenidos después de crearlo. Es **inmutable** si cualquier "cambio" produce un nuevo valor en lugar de modificar el original.

**C:** Depende de dónde vive el string:
- `char s[] = "hola"` — copia los caracteres al stack. Es **mutable**: `s[0] = 'H'` funciona y modifica el contenido en el lugar.
- `char *s = "hola"` — apunta al literal en memoria estática de solo lectura. Es **inmutable de facto**: `s[0] = 'H'` compila pero produce comportamiento indefinido en ejecución (generalmente un crash).

La distinción entre estas dos formas es sutil y fuente de bugs comunes en C.

**Python:** Los strings son siempre **inmutables**. No existe ninguna operación que modifique un string en el lugar. `s = s.upper()` no toca el string original — crea uno nuevo y reasigna la variable. Esto elimina toda una clase de bugs, a costa de que operaciones como concatenación en un loop sean menos eficientes.

---

### 6. ¿El tipo es un *first class citizen*?

Un tipo es *first class citizen* (ciudadano de primera clase) en un lenguaje si puede usarse en todos los contextos donde se usa cualquier otro valor: asignarse a variables, pasarse como argumento, retornarse desde funciones, compararse, almacenarse en colecciones. Un entero en C es un ejemplo clásico de first class citizen: podés hacer `int a = b`, `if (a == b)`, `return a`, sin restricciones. La pregunta es si los strings tienen ese mismo estatus.

**C:** No. Un string no se puede asignar con `=` (excepto en la inicialización de un arreglo), ni comparar con `==` (compara las direcciones de memoria, no el contenido), ni copiar con `=`. Para cada operación hay que usar funciones (`strcpy`, `strcmp`) o, como en este laboratorio, implementarlas.

**Python:** Sí. Un string se comporta exactamente igual que un entero o cualquier otro valor: `s = t`, `s == t`, `return s`, `lista.append(s)` funcionan sin restricciones ni funciones auxiliares.

---

### 7. ¿Cuál es la mecánica para ese tipo cuando se pasa como argumento?

**C:** Se pasa un **puntero** (`const char *`). La función recibe la dirección del primer carácter — no una copia del contenido. Si la función declara el parámetro `const`, el compilador impide modificaciones. Si no lo declara, puede modificar el original.

**Python:** Se pasa una **referencia** al objeto. Como los strings son inmutables, no hay riesgo de modificación accidental: cualquier "modificación" dentro de la función crea un nuevo objeto local.

---

### 8. ¿Y cuando son retornados por una función?

**C:** Se retorna un puntero. El programador debe garantizar que la memoria siga siendo válida: retornar un puntero a una variable local del stack es un bug clásico (dangling pointer). La solución habitual es alocar en el heap con `malloc` y documentar que el llamador debe liberar.

**Python:** Se retorna una referencia. El garbage collector mantiene el objeto vivo mientras haya al menos una referencia activa. No hay riesgo de dangling pointers.

---

### 9. ¿Qué nivel de soporte tiene para ASCII, Unicode y UTF-8?

**C:** Soporte nativo solo para ASCII. UTF-8 se puede almacenar como bytes pero las funciones de `<string.h>` operan byte a byte, sin ninguna conciencia de codificación: `strlen("ñ")` devuelve 2, no 1. Para Unicode completo se necesitan bibliotecas externas (ICU) o el tipo `wchar_t` con `<wchar.h>`.

**Python:** Soporte nativo completo para Unicode desde Python 3. Los literales son Unicode por defecto. La E/S usa UTF-8 por defecto. `len("ñ")` devuelve 1. `encode()` y `decode()` permiten convertir explícitamente entre strings y bytes en cualquier codificación.

---

### Conclusión

C trata los strings como una convención sobre memoria cruda: son punteros a secuencias de bytes terminadas en `'\0'`. El programador tiene control total — y responsabilidad total — sobre la memoria, la codificación y las operaciones. Python trata los strings como objetos de alto nivel: inmutables, Unicode, con gestión automática de memoria y una biblioteca rica integrada. La diferencia no es solo de comodidad: refleja filosofías distintas sobre dónde debe vivir la complejidad. En C, el programador construye las abstracciones; en Python, el lenguaje las provee. Este laboratorio implementa desde cero algunas de esas abstracciones para entender qué hay debajo.

---

## Parte II — Biblioteca String

### Especificación matemática

Esta sección describe qué debe hacer cada función. Usala como referencia antes
de leer o modificar el código.

#### Notación

| Símbolo | Significado |
|---|---|
| `s`, `t` | cadena (tipo String) |
| `c` | carácter (tipo Char) |
| `ε` | cadena vacía `""` |
| `\|s\|` | longitud de la cadena `s` |
| `s[i]` | i-ésimo carácter de `s` (base 0) |
| `s[1..]` | subcadena de `s` desde la posición 1 hasta el final |
| `𝔹` | Boolean = {0, 1} |
| `ℕ₀` | naturales incluyendo el 0 |

#### IsEmpty

**Signatura:** `IsEmpty : String → 𝔹`

**Especificación:**

```text
IsEmpty(s) = 1   si s = ε
IsEmpty(s) = 0   si s ≠ ε
```

#### GetLength

**Signatura:** `GetLength : String → ℕ₀`

**Especificación:**

```text
GetLength(ε) = 0
GetLength(s) = 1 + GetLength(s[1..])   si s ≠ ε
```

#### AreEqual

**Signatura:** `AreEqual : String × String → 𝔹`

**Especificación:**

```text
AreEqual(ε, ε) = 1
AreEqual(ε, t) = 0                                          si t ≠ ε
AreEqual(s, ε) = 0                                          si s ≠ ε
AreEqual(s, t) = (s[0] = t[0]) ∧ AreEqual(s[1..], t[1..])  si s ≠ ε ∧ t ≠ ε
```

Dos cadenas son iguales si y solo si tienen la misma longitud y los mismos
caracteres en el mismo orden.

#### AreDecimalDigits

**Signatura:** `AreDecimalDigits : String → 𝔹`

**Especificación:**

```text
AreDecimalDigits(ε) = 0
AreDecimalDigits(s) = (s[0] ∈ {'0'..'9'}) ∧ AreDecimalDigits(s[1..])   si |s| > 1
AreDecimalDigits(s) = (s[0] ∈ {'0'..'9'})                              si |s| = 1
```

La cadena vacía no es un conjunto válido de dígitos: se requiere al menos un
carácter.

#### Contains

**Signatura:** `Contains : String × Char → 𝔹`

**Especificación:**

```text
Contains(ε, c) = 0
Contains(s, c) = 1                    si s[0] = c
Contains(s, c) = Contains(s[1..], c)  si s[0] ≠ c
```

Devuelve 1 si el carácter `c` aparece al menos una vez en `s`.

---

### ¿Cómo funcionan los tests?

Los tests están en `StringTest.c` y usan `assert`:

- Si el assert **pasa**: silencio.
- Si el assert **falla**: el programa imprime la línea que falló y aborta.
- Al final, si todo pasó: `./StringTest` termina sin salida y con exit code 0.

Para correr los tests:

```bash
make test
```

---

### Función 0: `IsEmpty` — ya implementada, leerla

Abrí `String.c`. `IsEmpty` ya está implementada y es la referencia de estilo:

```c
int IsEmpty(const char *s) {
    return *s == '\0';
}
```

`*s` desreferencia el puntero: accede al primer carácter. Un string en C termina siempre con el carácter nulo `'\0'`. Si el primero ya es `'\0'`, la cadena está vacía.

Los tests de `IsEmpty` ya están activos en `StringTest.c`. Corré `make test` y verificá que pasan.

**P1** — `IsEmpty` podría haberse escrito también como `return s[0] == '\0'`. ¿Son equivalentes? ¿Por qué?

> R:No son equivalentes,si se pasa s = NULL (dirección 0x0), tanto *s como s[0] intentan desreferenciar un puntero nulo, provocando un Segmentation Fault.

---

### Función 1: `GetLength`

#### Tests

Abrí `StringTest.c` y **descomentá** el bloque de `GetLength`:

```c
/* ── GetLength ──────────────────────────────────────────────────────────── */
assert(GetLength("") == 0);
assert(GetLength("a") == 1);
assert(GetLength("hola") == 4);
assert(GetLength("hola mundo") == 10);
```

Corré los tests:

```bash
make test
```

Van a fallar porque `GetLength` todavía devuelve `-1`. Eso es lo esperado.

#### Especificación

Antes de implementar, revisá en este README la especificación matemática de
`GetLength` y asegurate de entender el caso base y el caso recursivo.

#### Implementación

`GetLength` es el candidato para la implementación **recursiva**. Abrí `String.c`, encontrá `GetLength` y **reemplazá** todo el cuerpo de la función con:

```c
int GetLength(const char *s) {
    if (IsEmpty(s))
        return 0;
    return 1 + GetLength(s + 1);
}
```

Corré los tests:

```bash
make test
```

**P2** — ¿Qué hace `s + 1`? ¿Por qué avanza al siguiente carácter y no al siguiente byte?

> R:s + 1 realiza aritmética de punteros. Avanza la dirección de memoria hacia la siguiente posición del tipo referenciado (char). Como el tipo char en C ocupa exactamente 1 byte por definición (sizeof(char) == 1), avanzar un carácter y avanzar un byte coinciden físicamente, pero conceptualmente avanza un elemento del tipo char apuntado por

**P3** — Si llamaras a `GetLength(NULL)`, ¿qué pasaría? ¿Por qué las precondiciones del contrato dicen `s != NULL`?

> R:Provocaría un fallo de segmentación (Segmentation Fault / Access Violation) y el programa se colgaría. Esto sucede porque GetLength(NULL) pasa el puntero nulo a IsEmpty(s), la cual intenta desreferenciarlo mediante *s (acceder a la dirección de memoria 0x0). Las precondiciones del contrato exigen s != NULL para deslindar a la función de realizar validaciones defensivas internas y garantizar que solo opere sobre punteros a cadenas válidas

```
GETLENGTH_PASA=
```
_(escribí SI cuando todos los tests de GetLength pasen)_

---

### Función 2: `AreEqual`

#### Tests

Los tests de `AreEqual` ya están activos en `StringTest.c`. Corré `make test` — algunos van a fallar.

#### El bug

Abrí `String.c` y mirá la implementación de `AreEqual`:

```c
int AreEqual(const char *s1, const char *s2) {
    while (!IsEmpty(s1) && !IsEmpty(s2)) {
        if (*s1 != *s2)
            return 0;
        s1++;
        s2++;
    }
    return 1;  /* <-- acá está el bug */
}
```

El `while` termina cuando alguna de las dos cadenas llega a `'\0'`. Después devuelve `1` sin verificar nada más.

**P4** — ¿Qué dos casos están mal cubiertos por `return 1`? Describí un ejemplo para cada uno.

> R:El caso 1 seria: s1 = "hola" y s2 = "hola mundo". El bucle while compara los primeros 4 caracteres ('h', 'o', 'l', 'a'), los cuales coinciden. Luego, s1 llega al terminador nulo '\0' (IsEmpty(s1) es verdadero), por lo que el while finaliza. Sin verificar que s2 todavía tiene más caracteres, la función ejecuta return 1;, indicando erróneamente que las cadenas son iguales cuando no lo son. El caso 2 seria: s1 = "algoritmos" y s2 = "algo". El bucle avanza comparando los primeros 4 caracteres. Al llegar al final de s2, IsEmpty(s2) se vuelve verdadero y corta el while. Como la función finaliza con return 1;, devuelve 1 (verdadero) a pesar de que s1 aún contiene caracteres restantes ("ritmos")

#### Corrección

Reemplazá la última línea de `AreEqual` en `String.c`:

```c
    return IsEmpty(s1) && IsEmpty(s2);
```

Corré los tests:

```bash
make test
```

```
AREEQUAL_PASA=
```
_(escribí SI cuando todos los tests de AreEqual pasen)_

---

### Función 3: `AreDecimalDigits`

#### Tests

Los tests de `AreDecimalDigits` ya están activos en `StringTest.c`. Corré `make test`.

Corré `make test`. Tres tests van a pasar pero uno va a fallar.

#### El bug

Abrí `String.c` y mirá `AreDecimalDigits`:

```c
int AreDecimalDigits(const char *s) {
    if (IsEmpty(s)) return 1;  /* <-- acá está el bug */
    for (const char *p = s; !IsEmpty(p); p++)
        if (*p < '0' || *p > '9')
            return 0;
    return 1;
}
```

**P5** — ¿Por qué la cadena vacía no debería considerarse un conjunto de dígitos decimales? Pensalo desde la especificación matemática.

> R:Para que una cadena represente un valor o secuencia numérica decimal, debe contener al menos un dígito decimal. La cadena vacía (longitud 0) no posee elementos, por lo que no satisface la condición de contener dígitos.

#### Corrección

Corregí el valor de retorno para el caso de cadena vacía en `String.c`.

```bash
make test
```

```
AREDECIMALDIGITS_PASA=SI
```
_(escribí SI cuando todos los tests de AreDecimalDigits pasen)_

---

### Función 4: `Contains`

Esta función la implementás completamente vos.

#### Especificación

Revisá en este README la especificación de `Contains` antes de implementarla.

#### Tests

Escribí al menos 4 tests en `StringTest.c` en el bloque de `Contains`. Incluí al menos un caso borde.

#### Implementación

Implementá `Contains` en `String.c`. No hay guía de estructura — usá lo aprendido de las funciones anteriores.

```bash
make test
```

```
CONTAINS_PASA=
```
_(escribí SI cuando todos los tests de Contains pasen)_

---

## Parte II.b — Módulo Conversion

Antes de implementar, discutí con tu equipo:

> ¿Tiene sentido que `ToInteger` sea parte de la biblioteca `String`?
> ¿O es correcto tenerla en un módulo separado `Conversion`?

**P6** — Conclusión de la discusión:

> R:Es correcto tener ToInt. eger en un módulo separado Conversion respetando el Principio de Responsabilidad Única (SRP) y la cohesión modular. Debe centrarse exclusivamente en la manipulación y consulta de cadenas de caracteres (longitud, comparación, búsqueda, validación de caracteres, etc.).La conversión entre distintas representaciones de datos (de texto a enteros, de enteros a texto, a flotantes, etc.) involucra lógica de parsing numérico, manejo de rangos y desbordamientos.

---

### Función: `ToInteger`

#### Tests

Los tests de `ToInteger` ya están activos en `ConversionTest.c`. Corré `make test` — todos van a fallar.

#### El bug

Abrí `Conversion.c` y mirá `ToInteger`:

```c
int ToInteger(const char *s) {
    int signo     = 1;
    int resultado = 0;
    if (*s == '-') { signo = -1; s++; }
    for (; *s != '\0'; s++)
        resultado = resultado * 10 + (*s - '0');
    return signo;  /* <-- acá está el bug */
}
```

**P7** — El loop acumula correctamente el valor en `resultado`. ¿Qué está mal en el `return`?

> R:La función está retornando solo la variable signo (que siempre vale 1 o -1). Para devolver el número entero completo convertido con el signo correspondiente, debe retornar el producto entre el signo y el valor acumulado: return signo * resultado;.

#### Corrección

Corregí la última línea de `ToInteger` en `Conversion.c`.

```bash
make test
```

**P8** — La expresión `*s - '0'` convierte un carácter dígito al entero correspondiente. ¿Por qué funciona? ¿Qué devuelve `'3' - '0'`?

> R:Funciona porque en el estándar ASCII (y UTF-8), los caracteres numéricos del '0' al '9' están codificados de forma consecutiva y contigua. El carácter '0' tiene el valor entero 48, el '1' el 49, el '2' el 50, y así sucesivamente. Al restar la posición base ('0'), se calcula la distancia numérica exacta entre el carácter analizado y el cero. Devuelve el entero 3. Matemáticamente realiza la resta entre los valores decimales ASCII de ambos caracteres: 51 - 48 = 3.

```
TOINTEGER_PASA=SI
```
_(escribí SI cuando todos los tests de ToInteger pasen)_

---

## Parte III — Programas con argumentos

### Cómo recibir argumentos en C

```c
int main(int argc, char *argv[]) { ... }
```

- `argc`: cantidad total de argumentos (incluye el nombre del programa)
- `argv[0]`: nombre del ejecutable
- `argv[1]` en adelante: los argumentos reales
- `argv[argc]` es siempre `NULL` — se puede usar para iterar sin `argc`

### Referencia: `enlineas` — ya implementado, leerlo

Abrí `enlineas.c`. Está completo y muestra el patrón a seguir:

```c
int main(int argc, char *argv[]) {
    (void)argc;
    for (char **arg = argv + 1; *arg != NULL; arg++)
        printf("%s\n", *arg);
    return 0;
}
```

- `argv + 1` apunta al primer argumento real (saltea `argv[0]`)
- `char **arg` es un puntero a puntero: cada `*arg` es un `char *` (un string)
- `arg++` avanza al siguiente string
- `*arg != NULL` detecta el fin del arreglo

Compilá y probá:

```bash
make enlineas
./enlineas hola mundo foo
```

Salida esperada:
```
hola
mundo
foo
```

**P9** — ¿Por qué `(void)argc` suprime un warning? ¿Cuándo sería necesario usar `argc`?

> R:El compilador genera una advertencia si se declara un parámetro en la firma de una función (como argc en main) y nunca se utiliza dentro de su cuerpo. Al escribir (void)argc;, se realiza un casteo explícito al tipo void sin asignar el resultado a ninguna variable. Esto le indica explícitamente al compilador que la variable fue leída/mencionada de forma intencional y que se conoce su falta de uso en el cuerpo del código, suprimiendo así la advertencia de "parámetro no utilizado". Seria necesaria usar argc en casos donde se deba validar la cantidad exacta o mínima de argumentos pasados por línea de comandos antes de procesarlos (por ejemplo, if (argc < 2) para verificar si el usuario ingresó al menos un argumento), o bien cuando se iteren los argumentos mediante un bucle de índice tradicional (por ejemplo, for (int i = 1; i < argc; i++)) en lugar de usar navegación por punteros con centinela NULL como *arg != NULL.

---

### Programa: `longitudes`

Abrí `longitudes.c`. La estructura del `for` ya está escrita — solo falta completar el `printf`:

```c
for (char **arg = argv + 1; *arg != NULL; arg++)
    printf("%d\n", 0); /* completar: reemplazar 0 */
```

Reemplazá el `0` con la llamada correcta a `GetLength`.

```bash
make longitudes
./longitudes hola mundo
```

Salida esperada:
```
4
5
```

```
LONGITUDES_PASA=SI
```
_(SI o NO)_

---

### Programa: `mayorlongitud`

Abrí `mayorlongitud.c`. La estructura está escrita — falta completar una línea:

```c
char *mayor = argv[1];
for (char **arg = argv + 2; *arg != NULL; arg++)
    if (GetLength(*arg) > GetLength(mayor))
        mayor = NULL; /* completar: ¿qué debería guardarse en mayor? */
printf("%s\n", mayor);
```

Reemplazá el `NULL` con el valor correcto.

```bash
make mayorlongitud
./mayorlongitud uno largo
./mayorlongitud uno dos tres cuatro
```

```
MAYORLONGITUD_PASA=SI
```
_(SI o NO)_

---

### Programa: `todosiguales`

Implementá `todosiguales.c` completo. Usá `AreEqual` de `String.h`.

La pista está en el comentario del archivo: comparar cada argumento contra `argv[1]`.

```bash
make todosiguales
./todosiguales igual igual igual
./todosiguales uno dos
```

```
TODOSIGUALES_PASA=SI
```
_(SI o NO)_

---

### Programa: `suma`

Implementá `suma.c` completo. Usá `ToInteger` de `Conversion.h`.

```bash
make suma
./suma 1 2 3
./suma -5 10
```

```
SUMA_PASA=
```
_(SI o NO)_SI

---

## Preguntas de reflexión

**P10** — `GetLength` es recursiva pero en C una llamada recursiva consume un stack frame. Si llamaras `GetLength` con un string de 1.000.000 de caracteres, ¿qué pasaría? ¿Cómo lo resolverías?

> R:Lo que pasaria es que se produciría un Stack Overflow (desbordamiento de pila) y el programa terminaría abruptamente con un error (Segmentation Fault). Esto ocurre porque cada llamada recursiva apila un nuevo stack frame (que almacena la dirección de retorno y parámetros). Con 1.000.000 de llamadas, se excede el tamaño límite asignado por el sistema operativo para la pila de ejecución de la pila. Se podria resolver reemplanzo la recursividad por un bucle iterativo (como un while o for) que mantenga un acumulador en una sola variable local, utilizando $O(1)$ de espacio en la pila

**P11** — En la Parte III, todos los programas usan `char **arg` para iterar en vez de un índice entero. ¿Qué ventaja tiene este estilo? ¿Cuándo sería preferible usar el índice?

> R: Las ventajas es que es mas efeciente y permite iterar la secuencia reconociendo directamente la terminacion marcada por el estandar argv. Es preferible usar el indice entero Cuando se requiere acceso aleatorio o no secuencial a los argumentos (ej. acceder directamente al argumento 3 sin recorrer los anteriores).   

**P12** — En C, `"hola"` es un literal de tipo `const char *`. Si intentaras modificar un carácter con `s[0] = 'H'`, el comportamiento es indefinido. ¿Por qué? ¿En qué parte de la memoria viven los literales?

> R:Los literales de cadena son inmutables por especificación del lenguaje C. Intentar modificar un literal a través de s[0] = 'H' viola esta restricción, lo que en sistemas operativos modernos provoca una violación de segmento (Segmentation Fault) al intentar escribir en una página de memoria protegida como de solo lectura (read-only). Estos viven en la seccion de memoria de solo lectura(read only data).

---

## Entrega

### Checklist

- [ ] `String.c` con todas las funciones implementadas y tests pasando
- [ ] `Conversion.c` con `ToInteger` corregida y tests pasando
- [ ] Los 5 programas funcionando (`make all`)
- [ ] `make test` pasa localmente
- [ ] Todo pusheado a `main`

### Verificación local

Antes de hacer push, verificá tu puntaje con:

```bash
make test
```

**Flujo recomendado:** hacé commits frecuentes mientras avanzás, usá `make test` para verificar tu progreso, y dejá el push para cuando una parte esté realmente lista.

### Integración continua

Cuando pusheás cambios dentro de `labs/lab-string/`, el workflow central del monorepo ejecuta los mismos checks y valida el puntaje mínimo.

> ⚠️ **Evitá pushes innecesarios.** Cada ejecución consume cómputo en servidores de GitHub — un recurso compartido. `make test` te da el mismo resultado en tu terminal sin costo.

Para ver los resultados:

1. Entrá a tu repositorio en GitHub
2. Hacé click en la pestaña **Actions**
3. Hacé click en la ejecución más reciente → job **Grading · lab-string**
4. Al final del job vas a ver la tabla con el resultado de cada check y el puntaje total
