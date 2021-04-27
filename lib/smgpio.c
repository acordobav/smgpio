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

pthread_t blink(int pin, int freq, int duration) {
    pinMode(pin, OUTPUT);

    BlinkData* data = malloc(sizeof(*data));
    data->pin = pin;
    data->freq = freq;
    data->duration = duration;

    pthread_t id;
    pthread_create(&id, NULL, blink_aux, data);
    return id;
}


void* blink_aux(void* blink_data) {
    BlinkData* data = blink_data;

    int state = LOW;
    time_t endwait = time(NULL) + data->duration;
    while(time(NULL) < endwait) {
        if (state == LOW)
            state = HIGH;
        else
            state = LOW;

        digitalWrite(data->pin, state);
        sleep(data->freq);
    }

    free(blink_data);
}