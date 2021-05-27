// Source: https://elinux.org/RPi_GPIO_Code_Samples#C
// Python alternative: https://spellfoundry.com/docs/getting-the-sleepy-pi-to-shutdown-the-raspberry-pi/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define GPIO_PIN_SHUTDOWN 24
#define GPIO_PIN_RUNNING 25

#define BCM2708_PERI_BASE        0x20000000 /* 0x3F000000 for RPi 2,3 */
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

void setup_io();

int main(int argc, char **argv) {
    // Set up gpio pointer for direct register access
    setup_io();

    // Set GPIO pin 24 to input, 25 to output
    INP_GPIO(GPIO_PIN_SHUTDOWN);
    INP_GPIO(GPIO_PIN_RUNNING); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(GPIO_PIN_RUNNING);

    GPIO_SET = 1<<GPIO_PIN_RUNNING;
    printf("[Info] Telling Sleepy Pi RPi is running on pin 25\n");

    char shutdown_command[13];
    strcpy(shutdown_command, "poweroff -nf");

    // Shutdown on signal
    while (1) {
        if (GET_GPIO(GPIO_PIN_SHUTDOWN)) {
            printf("[Info] Sleepy Pi requesting RPi to shutdown on pin 24\n");
            system(shutdown_command);
            break;
        }
        sleep(0.5);
    }

    return 0;
}

//
// Set up a memory regions to access GPIO
//
void setup_io() {
    /* open /dev/mem */
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        exit(-1);
    }

    /* mmap GPIO */
    gpio_map = mmap(
        NULL,                   //Any adddress in our space will do
        BLOCK_SIZE,             //Map length
        PROT_READ|PROT_WRITE,   //Enable reading & writting to mapped memory
        MAP_SHARED,             //Shared with other processes
        mem_fd,                 //File to map
        GPIO_BASE               //Offset to GPIO peripheral
    );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (gpio_map == MAP_FAILED) {
        printf("mmap error %d\n", (int)gpio_map);//errno also set!
        exit(-1);
    }

    // Always use volatile pointer!
    gpio = (volatile unsigned *)gpio_map;
}
