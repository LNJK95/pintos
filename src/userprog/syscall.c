#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/filesys.h"


#include <stdbool.h>
#include "filesys/file.h"
#include "lib/kernel/console.h"

static void syscall_handler (struct intr_frame *);


void syscall_init (void) {

  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler (struct intr_frame *f UNUSED) {

  int *syscall_no = f->esp;
     
  switch(*syscall_no) {
  case(SYS_HALT) : {
    halt();
  }
    break;
  case(SYS_EXIT) : {
    int status = (int)*(syscall_no+1);
    exit(status);
  }
    break;
  case(SYS_READ) : {
    int fd = (int)*(syscall_no+1);
    void *buffer = (void *)*(syscall_no+2);
    unsigned size = (unsigned)*(syscall_no+3);
    f->eax = read(fd, buffer, size);
  }
    break;
  case(SYS_WRITE) : {
    int fd = (int)*(syscall_no+1);
    void *buffer = (void *)*(syscall_no+2);
    unsigned size = (unsigned)*(syscall_no+3);
    f->eax = write(fd, buffer, size);
  }
    break;
  case(SYS_CLOSE) : {
    int fd = (int)*(syscall_no+1);
    close(fd);
  }
    break;
  case(SYS_OPEN) : {
    char *file_name = (char *)*(syscall_no+1);
    f->eax = open(file_name);
  }
    break;
  case(SYS_CREATE) : {
    char *file_name = (char *)*(syscall_no+1);
    unsigned size = (unsigned)*(syscall_no+2);
    f->eax = create(file_name, size);
  }
    break;
  default :
    break;
  }
}

static void halt(void) {
  power_off();
}

static void exit(int status) {
  int kernel_status = status;
  thread_exit();
}

static int write(int fd, const void *buffer, unsigned size) {
  int written_bytes = 0;
  if (fd == 1) {
    while (size > 100) {
      putbuf(buffer+written_bytes, 100);
      size = size-100;
      written_bytes = written_bytes+100;
    }
    putbuf(buffer+written_bytes, size);
    written_bytes = written_bytes + size;
    return written_bytes;
  }
  else if (1<fd && fd<130 && thread_current()->fd_opened[fd-2] != NULL) {
    while (size > 100) {
      written_bytes = written_bytes + file_write(thread_current()->fd_opened[fd-2], buffer, 100);
      size = size-100;
    }
    written_bytes = written_bytes + file_write(thread_current()->fd_opened[fd-2], buffer, size);
    return written_bytes;
  }
  else {
    return -1;
  }
}

static int read(int fd, void *buffer, unsigned size) {
  int read_bytes = 0;
  if (fd == 0) {
    int i = 0;
    while (i < size) {
      buffer = buffer + input_getc();
      i++;
    }
    return size;
  }
  else if (1<fd && fd<130 && thread_current()->fd_opened[fd-2] != NULL) {
    if (size > 100) {
      read_bytes = read_bytes + file_read(thread_current()->fd_opened[fd-2], buffer, 100);
      size = size - 100;
    }
    read_bytes = read_bytes + file_read(thread_current()->fd_opened[fd-2], buffer, size);
    return read_bytes;
  }
  else {
    return -1;
  }
}

static void close(int fd) {
  if (1<fd && fd<130) {
    file_close(thread_current()->fd_opened[fd-2]);
    thread_current()->fd_opened[fd-2]= NULL;
  }
  else {
    exit(-1);
  }
}

static int open(const char *file_name) {
  int i = 2;
  while (i < 130) {
    if (thread_current()->fd_opened[i-2] == NULL) {
      thread_current()->fd_opened[i-2] = filesys_open(file_name);
      if (thread_current()->fd_opened[i-2] == NULL) {
        file_close(thread_current()->fd_opened[i-2]);
	return -1;
      }
      return i;
    }
    i++;
  }
  return -1;
}

static bool create(const char *file_name, unsigned initial_size) {
  return filesys_create(file_name, initial_size);
}

