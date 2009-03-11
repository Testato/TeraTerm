/*
 * wait4all.h
 *
 */
#ifndef _WAIT4ALL_H_
#define _WAIT4ALL_H_

int open_macro_shmem(void);
void close_macro_shmem(void);
int register_macro_window(HWND hwnd);
int unregister_macro_window(HWND hwnd);
void put_macro_1byte(BYTE b);
int read_macro_1byte(LPBYTE b);

#endif

