#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/filesys.h"


//include bools
#include <stdbool.h>
#include "filesys/file.h"
#include "lib/kernel/console.h"

static void syscall_handler (struct intr_frame *);


void syscall_init (void) {

  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler (struct intr_frame *f UNUSED) {

  int *syscall_no = f->esp;
  void *buffer;
  char *file_name;
  unsigned size;
  int status;
  int fd;
     
  switch(*syscall_no) {
  case(SYS_HALT) :
    halt();
    break;
  case(SYS_EXIT) :
    status = (int)*(syscall_no+1);
    exit(status);
    break;
  case(SYS_READ) :
    fd = (int)*(syscall_no+1);
    buffer = (void *)*(syscall_no+2);
    size = (unsigned)*(syscall_no+3);
    f->eax = read(fd, buffer, size);
    break;
  case(SYS_WRITE) :
    fd = (int)*(syscall_no+1);
    buffer = (void *)*(syscall_no+2);
    size = (unsigned)*(syscall_no+3);
    f->eax = write(fd, buffer, size);
    break;
  case(SYS_CLOSE) :
    fd = (int)*(syscall_no+1);
    close(fd);
    break;
  case(SYS_OPEN) :
    file_name = (char *)*(syscall_no+1);
    f->eax = open(file_name);
    break;
  case(SYS_CREATE) :
    file_name = (char *)*(syscall_no+1);
    size = (unsigned)*(syscall_no+2);
    f->eax = create(file_name, size);
    break;
  default :
    break;
  }
}

void halt(void) {
  power_off();
}

void exit(int status) {
  int kernel_status = status;
  thread_exit();
}

int write(int fd, const void *buffer, unsigned size) {
  int written_bytes = 0;
  void *buffer_pointer = buffer;
  if (fd == 1) {
    while (size > 100) {
      putbuf(buffer_pointer, 100);
      buffer_pointer = buffer_pointer + 100;
      size = size-100;
      written_bytes = written_bytes+5;
    }
    putbuf(buffer_pointer, size);
    written_bytes = written_bytes + size;
    return written_bytes;
  }
  else if (1<fd && fd<131 && thread_current()->fd_opened[fd] != NULL) {
    while (size > 100) {
      written_bytes = written_bytes + file_write(thread_current()->fd_opened[fd], buffer, 100);
      size = size-100;
    }
    written_bytes = written_bytes + file_write(thread_current()->fd_opened[fd], buffer, size);
    return written_bytes;
  }
  else {
    return -1;
  }
}

int read(int fd, void *buffer, unsigned size) {
  int read_bytes = 0;
  void *buffer_pointer = buffer;
  if (fd == 0) {
    while (size > 100) {
      input_getc(buffer_pointer, 100);
      buffer_pointer = buffer_pointer + 100;
      size = size - 100;
      read_bytes = read_bytes + 100;
    }
    input_getc(buffer_pointer, size);
    read_bytes = read_bytes + size;
    return read_bytes;
  }
  else if (1<fd && fd<131 && thread_current()->fd_opened[fd] != NULL) {
    if (size > 100) {
      read_bytes = read_bytes + file_read(thread_current()->fd_opened[fd], buffer, 100);
      size = size - 100;
    }
    read_bytes = read_bytes + file_read(thread_current()->fd_opened[fd], buffer, size);
    return read_bytes;
  }
  else {
    return -1;
  }
}

void close(int fd) {
  if (1<fd && fd<131) {
    thread_current()->fd_opened[fd] = NULL;
  }
  else {
    printf("Tried to close a fd out of bounds\n");
    exit(-1);
  }
}

int open(const char *file_name) {
  if (filesys_open(file_name) == NULL) {
    return -1;
  }
  int i = 2;
  while (i < 131) {
    if (thread_current()->fd_opened[i] == NULL) {
      thread_current()->fd_opened[i] = filesys_open(file_name);
      return i;
    }
    i++;
  }
  return -1;
}

bool create(const char *file_name, unsigned initial_size) {
  return filesys_create(file_name, initial_size);
}

