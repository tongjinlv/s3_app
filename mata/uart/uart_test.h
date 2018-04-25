#ifndef __UART_TEST_H
#define __UART_TEST_H

int open_port(int fd);
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);
void ble_send(void *arg);
void ble_recv(void *arg);

#endif

