#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

#include <stdlib.h> // for prototypes of malloc() and free()

void _exit(int a) { while(1); }
int _close(int file) {return 0;}
//char **environ; /* pointer to array of char * strings that define the current environment variables */
int execve(char *name, char **argv, char **env) {return 0;}
int fork() {return 0;}
int _fstat(int file, struct stat *st) {return 0;}
int _getpid() {return 0;}
int _isatty(int file) {return 0;}
int _kill(int pid, int sig) {return 0;}
int link(char *old, char *new) {return 0;}
int _lseek(int file, int ptr, int dir) {return 0;}
int open(const char *name, int flags, ...) {return 0;}
int _read(int file, char *ptr, int len) {return 0;}
caddr_t _sbrk(int incr) { return (char *)0;}
int stat(const char *file, struct stat *st) {return 0;}
//clock_t times(struct tms *buf) ;
int unlink(char *name) {return 0;}
int wait(int *status) {return 0;}
int _write(int file, char *ptr, int len) {return 0;}
//int gettimeofday(struct timeval *p, struct timezone *z) {return 0;}



