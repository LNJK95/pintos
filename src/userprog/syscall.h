#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <stdbool.h>

struct fd_struct;

void syscall_init (void);

void halt(void);

void exit(int status);

bool create(const char *file, unsigned initial_size);

int open(const char *file);

void close(int fd);

int read(int fd, void *buffer, unsigned size);

int write(int fd, const void *buffer, unsigned size);

#endif /* userprog/syscall.h */
