# smgpio
Biblioteca dinámica escrita en C para el control de los GPIO de un Raspberry Pi

## Utilización biblioteca smgpio

Para utilizar un gpio es necesario establecerlo como de entrada o salida, esto se logra utilizando la función pinMode(int pin, int mode), donde pin es el número del gpio y el modo es INPUT para entrada o OUTPUT para salida.

Si un pin está configurado como OUTPUT, la función digitalWrite(int pin, int value) permite realizar la lectura digital,  pin es el número de gpio y value debe ser HIGH o LOW.

Si un pin está configurado como INPUT, la función digitalRead(int pin) permite leer el valor que se está recibiendo, pin es el número de gpio. Esta función retorna un 1 para alto y un 0 para bajo.

Si se desea crear un blink, la función blink(int pin, int freq, int duration) crea un nuevo hilo que utiliza el gpio con número pin, y realiza un blink cada freq segundos durante duration segundos.
