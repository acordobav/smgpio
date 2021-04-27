#include <stdio.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <sys/ioctl.h>

#include <poll.h>
#include <stdint.h>
#include <sys/wait.h>
#include <pthread.h>

#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

#define INT_EDGE_FALLING 0
#define INT_EDGE_RISING 1
#define INT_EDGE_BOTH 2

// sysFds:
//	Map a file descriptor from the /sys/class/gpio/gpioX/value
static int sysFds [64] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
} ;

// ISR Data
static void (*isrFunctions [64])(void) ;

static volatile int pinPass = -1 ;

static pthread_mutex_t pinMutex = PTHREAD_MUTEX_INITIALIZER;

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

int waitForInterrupt(int pin, int mS);
static void *interruptHandler(void *arg);
int gpioISR(int pin, int mode, void (*function)(void));
void delay (unsigned int howLong);


void setEdge(int pin, int mode);