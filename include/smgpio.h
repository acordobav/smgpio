#include <stdio.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>

#include <time.h>
#include <pthread.h>

#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

// sysFds:
//	Map a file descriptor from the /sys/class/gpio/gpioX/value
static int sysFds [64] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
} ;

/**
 * Function to export a GPIO
 * gpio: number of the gpio to export
**/
void exportGpio(int gpio);

/**
 * Function to unexport a GPIO
 * gpio: number of the gpio to unexport
**/
void unexportGpio(int gpio);

/**
 * Function to set if a gpio must be input or output
 * pin: number of the gpio
 * mode: INPUT or OUTPUT
**/
void pinMode(int pin, int mode);

/**
 * Function to write the pin state
 * pin: GPIO number
 * value: HIGH or LOW
**/
void digitalWrite(int pin, int value);

/**
 * Function to read the pin state
 * pin: GPIO number
**/
int digitalRead(int pin);

/**
 * Function to get the path to the [name] file of a pin
**/
char *getPinFileName(int pin, char *name);

/**
 * Function to get the file descriptor related to a pin
 * pin: gpio number
 * name: filename inside the gpio folder
 * return: file descriptor
**/
int openPinFile(int pin, char *name);


/**
 * Creates a thread that generates a blink of a specific 
 * duration, every interval of time
 * pin: gpio number of the pin
 * freq: interval of time between blinks
 * duration: how low should be the blink
 * returns: thread id
**/
pthread_t blink(int pin, int freq, int duration);

typedef struct blinkData
{
  int pin;
  int freq; 
  int duration;
} BlinkData;

void* blink_aux(void* blink_data);