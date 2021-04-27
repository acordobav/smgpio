#include <smgpio.h>

void test(void) {
    printf("Interrupted!\n");
}

int main() {
    // Test the function to established the mode
    pinMode(22, OUTPUT);
    pinMode(16, INPUT);

    // Test the digital write
    digitalWrite(22, LOW);
    digitalWrite(22, HIGH);

    // Test the digital read
    int value = digitalRead(22);

    if(value != HIGH) {
        perror("Error on digital read/write");
        exit(1);
    }

    // Test the ISR functionality
    gpioISR(16, INT_EDGE_RISING, &test);

    // Test the unexport
    unexportGpio(16);
    unexportGpio(22);

    printf("Library sm-gpio works as expected!\n");

    return 0;
}