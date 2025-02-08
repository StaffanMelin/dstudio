// INSPIRED: https://www.kernel.org/doc/Documentation/i2c/dev-interface */
extern "C" {
  #include <linux/i2c-dev.h>
  #include <i2c/smbus.h>
}

#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/ioctl.h>

int main()
{
  int adapter_nr = 1;
  char filename[20];
  int file;

  snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);

  file = open(filename, O_RDWR);

  int addr = 0x4D;

  if (ioctl(file, I2C_SLAVE, addr) < 0) {
    exit(1);
  }

  __u8 reg = 0x10;
  __s32 res;
  char buf[10];

  while (res >=0)
  {
    res = i2c_smbus_read_word_data(file, reg);
    std::cout << res << "\n";
  }
}
