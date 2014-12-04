/* 
 * read.h
 *  read head file
 *  jgfntu@163.com
 */

#ifndef __READ_FILE_H
#define __READ_FILE_H

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

ssize_t read_all(int fd, void *buf, size_t *len);
ssize_t write_all(int fd, void *buf, size_t *len);

#endif
