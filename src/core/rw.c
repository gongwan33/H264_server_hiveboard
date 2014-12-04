/* 
 * read.c
 *  read data buffer
 *  jgfntu@163.com
 *
 */

#include "rw.h"

/* read all texts */
ssize_t read_all(int fd, void *buf, size_t *len)
{
	size_t nleft;
	ssize_t nread;
	ssize_t total;
	char *ptr;
	ptr = buf;
	nleft = *len;
	total = 0;

	while (nleft > 0) {
		if ((nread = read(fd, ptr, *len)) == -1) {
			perror("readall");
			break;
		}
		if (nread == 0)
			break;
		nleft -= nread;
		ptr += nread;
		total += nread;
		*len = nleft;
	}
	*len = total;
	return (nread == -1) ? -1 : 0;
}

/* wirte all texts */
ssize_t write_all(int fd, void *buf, size_t *len)
{
	size_t nleft;
	ssize_t nwrite;
	ssize_t total;
	char *ptr;
	ptr = buf;
	nleft = *len;
	total = 0;
	while (nleft > 0) {
		if ((nwrite = write(fd, ptr, *len)) == -1) {
			perror("writeall");
			break;
		}
		if (nwrite == 0)
			break;
		nleft -= nwrite;
		ptr += nwrite;
		total += nwrite;
		*len = nleft;
	}
	*len = total;
	return (nwrite == -1) ? -1 : 0;
}



