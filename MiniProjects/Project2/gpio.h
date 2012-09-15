/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio);

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio);

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value);

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value);

/****************************************************************
 * gpio_set_edge
 ****************************************************************/
int gpio_set_edge(unsigned int gpio, char *edge);

/****************************************************************
 * gpio_fd_open
 ****************************************************************/
int gpio_fd_open(unsigned int gpio);

/****************************************************************
 * gpio_fd_close
 ****************************************************************/
int gpio_fd_close(int fd);
