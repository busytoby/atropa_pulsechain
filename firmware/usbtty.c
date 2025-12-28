#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>

struct termios tty;
char tx_buffer[141];
char rx_buffer[141];

int rl() {
  fd_set fds;
  struct timeval tv;
  size_t i = 0;
  int retval;
  int c;

  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);

  tv.tv_sec = 0;
  tv.tv_usec = 10000;

  retval = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);

  if (retval == -1) {
    perror("select()");
    return -1;
  } else if (retval) {
    memset(tx_buffer, '\0', 140);
    while(i < 140 && (c = fgetc(stdin)) != EOF && c != '\n')
      tx_buffer[i++] = (char)c;
    tx_buffer[i] = '\n';
    return i;
  }
  return 0;
}

int main() {
  int fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
  size_t bytesread;
  if(fd <0) return -1;

  fcntl(fd, F_SETFL, 0);

  tcgetattr(fd, &tty);
  cfmakeraw(&tty);

  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 1;

  tcsetattr(fd, TCSANOW, &tty);
  tcflush(fd, TCIFLUSH);

  while(1) {
    if((bytesread = rl()) > 0) {
      write(fd, tx_buffer, strlen(tx_buffer));
      //printf("%s\n", tx_buffer);
      fflush(stdout);
      usleep(300000);
    }

    memset(rx_buffer, '\0', 140);
    ssize_t n = read(fd, rx_buffer, 140);

    if(n > 0) {
      rx_buffer[140] = '\0';
      printf("%s", rx_buffer);
      fflush(stdout);
    } else if(n <= 0)
      usleep(100000);
  }

  close(fd);
  return 0;
}
