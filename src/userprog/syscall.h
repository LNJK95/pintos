#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <stdbool.h>

struct fd_struct;

void syscall_init (void);

static void halt(void);

static void exit(int status);

static bool create(const char *file, unsigned initial_size);

static int open(const char *file);

static void close(int fd);

static int read(int fd, void *buffer, unsigned size);

static int write(int fd, const void *buffer, unsigned size);

#endif /* userprog/syscall.h */
