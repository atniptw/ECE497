#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include "gpio.h"
#include "i2c.h"

/****************************************************************
 * Constants
 ****************************************************************/
 
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64
/* {"gpio1_6", "gpio1_7", "gpio1_2", "gpio1_3", "gpio1_13", "gpio1_12", "gpio1_15", "gpio1_14"} */

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

/****************************************************************
 * signal_handler
 ****************************************************************/
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}

/****************************************************************
 * int2bin
 ****************************************************************/
int *int2bin(int a, int *buffer, int buf_size) {
  buffer += (buf_size - 1);
  int i;

  for (i = 8; i >= 0; i--) {
      *buffer-- = (a & 1);

      a >>= 1;
  }

  return buffer;
}

/****************************************************************
 * main
 ****************************************************************/
int main(int argc, char **argv)
{
	struct pollfd fdset[2];
	int nfds = 2;
	int gpio_fd, timeout, rc;
	char *buf[MAX_BUF];
  int buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int gpio;
	int len;
  int loops = 0;
  int LEDS[9] = {38, 63, 27, 65, 45, 44, 47, 46, 0};
  int line1_1;
  FILE *fr;

	if (argc < 2) {
		printf("Usage: gpio-int <gpio-pin>\n\n");
		printf("Waits for a change in the GPIO pin voltage level or input on stdin\n");
		exit(-1);
	}

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	gpio = atoi(argv[1]);

	gpio_export(gpio);
	gpio_set_dir(gpio, 0);
	gpio_set_edge(gpio, "both");  // Can be rising, falling or both
	gpio_fd = gpio_fd_open(gpio);

	timeout = POLL_TIMEOUT;
  buffer[8 - 1] = '\0';

  int i;
  for(i = 0; i < 8; i++)
  {
    gpio_export(LEDS[i]);
    gpio_set_dir(LEDS[i], 1);
  }
 
  fr = fopen ("/sys/devices/platform/omap/tsc/ain6", "rt");
  fscanf(fr,"%d",&line1_1);
  printf("%d", line1_1);
  fclose(fr);

  fr = fopen ("/sys/kernel/debug/omap_mux/gpmc_a2", "wt");
  fprintf(fr, "%d", 6);
  fclose(fr);

  fr = fopen ("/sys/class/pwm/ehrpwm.1\:0/run", "wt");
  fprintf(fr, "%d", 1);
  fclose(fr);

  fr = fopen ("/sys/class/pwm/ehrpwm.1\:0/duty_percent", "wt");
  fprintf(fr, "%d", 25);
  fclose(fr);

	while (keepgoing) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = gpio_fd;
		fdset[1].events = POLLPRI;

		rc = poll(fdset, nfds, timeout);      

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
    fr = fopen ("/sys/devices/platform/omap/tsc/ain6", "rt");
    fscanf(fr,"%d",&line1_1);
    printf("%d\n", line1_1);
    fclose(fr);
    fr = fopen ("/sys/class/pwm/ehrpwm.1\:0/duty_percent", "wt");
    fprintf(fr, "%d", 0);
    fclose(fr);
    fr = fopen ("/sys/class/pwm/ehrpwm.1\:0/period_freq", "wt");
    printf("%d\n", line1_1);
    fprintf(fr, "%d", line1_1);
    fclose(fr);
    fr = fopen ("/sys/class/pwm/ehrpwm.1\:0/duty_percent", "wt");
    fprintf(fr, "%d", 25);
    fclose(fr);
      
		if (rc == 0) {
			printf(".");
		}
            
		if (fdset[1].revents & POLLPRI) {
			lseek(fdset[1].fd, 0, SEEK_SET);  // Read from the start of the file
			len = read(fdset[1].fd, buf, MAX_BUF);
			printf("\npoll() GPIO %d interrupt occurred, value=%c, len=%d\n",
				 gpio, buf[0], len);
      int temp = read_i2c(3, 72, 0);
      int2bin(temp, buffer, 8);
      for(i = 0; i < 8; i++)
        {
          gpio_set_value(LEDS[i], buffer[i]);
        }
		}

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}

		fflush(stdout);
	}

  for(i = 0; i < 8; i++)
  {
    printf("Unsetting gpio%d", LEDS[i]);
    gpio_set_value(LEDS[i], 0);
    gpio_unexport(LEDS[i]);
  }
	gpio_fd_close(gpio_fd);
	return 0;
}
