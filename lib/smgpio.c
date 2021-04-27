#include "smgpio.h"

void exportGpio(int gpio)
{
    if ((gpio < 0) || (gpio > 63))
    {
        perror("GPIO number must be 0-63");
        exit(1);
    }

    // Checks if the gpio was already exported
    char *pinFolder = getPinFileName(gpio, "");
    if (!(access(pinFolder, F_OK) == 0))
    {
        // Opening the /sys/class/gpio/export file
        int fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd < 0)
        {
            perror("Unable to open /sys/class/gpio/export");
            exit(1);
        }

        // Conversion of the gpio number into string
        char pin[3];
        sprintf(pin, "%d", gpio);

        // Export the pin by writing to /sys/class/gpio/export
        if (write(fd, pin, strlen(pin)) < 0)
        {
            perror("Error writing to /sys/class/gpio/export");
            exit(1);
        }

        close(fd);
    }
}

void unexportGpio(int gpio)
{
    if ((gpio < 0) || (gpio > 63))
    {
        perror("GPIO number must be 0-63");
        exit(1);
    }

    // Opening the /sys/class/gpio/unexport file
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1)
    {
        perror("Unable to open /sys/class/gpio/unexport");
        exit(1);
    }

    // Conversion of the gpio number into string
    char pin[3];
    sprintf(pin, "%d", gpio);

    // Unexport the pin by writing to /sys/class/gpio/unexport
    if (write(fd, pin, strlen(pin)) < 0)
    {
        perror("Error writing to /sys/class/gpio/unexport");
        exit(1);
    }

    close(fd);
}

char *getPinFileName(int pin, char *name)
{
    // Conversion of the gpio number into string
    char gpio[3];
    sprintf(gpio, "%d", pin);

    // The gpio folder name is obtained
    const char *gpioString = "gpio";
    char *gpioFolderName = malloc(strlen(gpioString) + strlen(gpio) + 1);
    strcpy(gpioFolderName, gpioString);
    strcat(gpioFolderName, gpio);

    // The gpio file is obtained
    const char *path1 = "/sys/class/gpio/";
    char *gpioFile = malloc(
        strlen(path1) + strlen(gpioFolderName) + strlen(name) + 1);
    strcpy(gpioFile, path1);
    strcat(gpioFile, gpioFolderName);
    strcat(gpioFile, name);

    free(gpioFolderName);

    return gpioFile;
}

int openPinFile(int pin, char *name)
{
    // Get the file name
    char *gpioFile = getPinFileName(pin, name);

    // Open the file
    int fd = open(gpioFile, O_RDWR);
    if (fd == -1)
    {
        perror("Unable to open the gpio file");
        exit(1);
    }

    free(gpioFile);

    return fd;
}

void pinMode(int pin, int mode)
{
    // Checks the desired mode for the pin
    if (mode != INPUT && mode != OUTPUT)
    {
        perror("Pin mode must be INPUT or OUTPUT");
        exit(1);
    }

    // Exports the pin
    exportGpio(pin);

    // Get the file descriptor of the mode file
    int fd = openPinFile(pin, "/direction");

    // Writing the pin mode to the file
    char *pinMode = "out";
    if (!mode)
        pinMode = "in";
    if (write(fd, pinMode, strlen(pinMode)) < 0)
    {
        perror("Unable to set pin mode");
        exit(1);
    }

    close(fd);
}

void digitalWrite(int pin, int value)
{
    if ((pin < 0) || (pin > 63))
    {
        perror("Pin number must be 0-63");
        exit(1);
    }

    if ((value != LOW) && (value != HIGH))
    {
        perror("Pin value must be LOW or HIGH");
        exit(1);
    }

    // Get the file descriptor of the value file
    if (sysFds[pin] == -1)
    {
        int fd = openPinFile(pin, "/value");
        sysFds[pin] = fd;
    }
    else
    {
        lseek(sysFds[pin], 0L, SEEK_SET);
    }

    // Conversion of the value into string
    char svalue[2];
    sprintf(svalue, "%d", value);

    // Writing the value
    if (write(sysFds[pin], svalue, strlen(svalue)) < 0)
    {
        perror("Unable to set the pin value");
        exit(1);
    }
}

int digitalRead(int pin)
{
    if ((pin < 0) || (pin > 63))
    {
        perror("Pin number must be 0-63");
        exit(1);
    }

    // Get the file descriptor of the value file
    if (sysFds[pin] == -1)
    {
        int fd = openPinFile(pin, "/value");
        sysFds[pin] = fd;
    }
    else
    {
        lseek(sysFds[pin], 0L, SEEK_SET);
    }

    char value[1];
    if (read(sysFds[pin], value, 1) < 0)
    {
        perror("Unable to read the pin value");
        exit(1);
    }

    int ivalue = atoi(value);

    return ivalue;
}

int gpioISR(int pin, int mode, void (*function)(void))
{
    pthread_t threadId;
    char fName[64];
    char pinS[8];
    int count, i;
    char c;

    if ((pin < 0) || (pin > 63))
    {
        perror("Pin number must be 0-63");
        exit(1);
    }

    // The edge is configured
    setEdge(pin, mode);

    // Get the file descriptor of the value file
    if (sysFds[pin] == -1)
    {
        int fd = openPinFile(pin, "/value");
        sysFds[pin] = fd;
    }

    // Clear any initial pending interrupt

    ioctl(sysFds[pin], FIONREAD, &count);
    for (i = 0; i < count; ++i)
        read(sysFds[pin], &c, 1);

    isrFunctions[pin] = function;

    pthread_mutex_lock(&pinMutex);
    pinPass = pin;
    pthread_create(&threadId, NULL, interruptHandler, NULL);
    while (pinPass != -1)
        delay(1);
    pthread_mutex_unlock(&pinMutex);

    return 0;
}

static void *interruptHandler(void *arg)
{
    int myPin;

    myPin = pinPass;
    pinPass = -1;

    for (;;)
        if (waitForInterrupt(myPin, -1) > 0)
            isrFunctions[myPin]();

    return NULL;
}

int waitForInterrupt(int pin, int mS)
{
    int fd, x;
    uint8_t c;
    struct pollfd polls;

    if ((fd = sysFds[pin]) == -1)
        return -2;

    // Setup poll structure

    polls.fd = fd;
    polls.events = POLLPRI | POLLERR;

    // Wait for it ...

    x = poll(&polls, 1, mS);

    if (x > 0)
    {
        lseek(fd, 0, SEEK_SET); // Rewind
        (void)read(fd, &c, 1);  // Read & clear
    }

    return x;
}

void delay(unsigned int howLong)
{
    struct timespec sleeper, dummy;

    sleeper.tv_sec = (time_t)(howLong / 1000);
    sleeper.tv_nsec = (long)(howLong % 1000) * 1000000;

    nanosleep(&sleeper, &dummy);
}

void setEdge(int pin, int mode)
{
    if ((pin < 0) || (pin > 63))
    {
        perror("Pin number must be 0-63");
        exit(1);
    }

    if ((mode != INT_EDGE_BOTH) &&
        (mode != INT_EDGE_RISING) &&
        (mode != INT_EDGE_FALLING))
    {
        perror("Invalid mode: Edge should be rising, falling or both");
        exit(1);
    }

    // The edge file is obtained
    int fd = openPinFile(pin, "/edge");

    // Writing the edge value
    char* smode = "none\n";
    if(mode == INT_EDGE_FALLING) smode = "falling\n";
    else if(mode == INT_EDGE_RISING) smode = "rising\n";
    else if(mode == INT_EDGE_BOTH) smode = "both\n";

    if (write(fd, smode, strlen(smode)) < 0)
    {
        perror("Unable to set the edge value");
        exit(1);
    }

    close(fd);
}   