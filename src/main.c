#include <smgpio.h>

#define output_pin_1 26
#define output_pin_2 6
#define input_pin 16

int main() {
    // Test the function to establish the mode
    pinMode(output_pin_1, OUTPUT);
    pinMode(output_pin_2, OUTPUT);
    pinMode(input_pin, INPUT);

    // Blnk with 1 second frecuency, during 5 seconds
    blink(output_pin_1, 1, 5);
    sleep(5);

    // Test the digital write
    digitalWrite(output_pin_2, LOW);
    digitalWrite(output_pin_2, HIGH);

    // Test the digital read
    int value = digitalRead(input_pin);
    printf("Value read on GPIO 16: %d\n", value);

    // Test the unexport
    unexportGpio(output_pin_1);
    unexportGpio(output_pin_2);
    unexportGpio(input_pin);

    printf("Library smgpio works as expected!\n");

    return 0;
}