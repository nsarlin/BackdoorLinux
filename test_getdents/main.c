#define _GNU_SOURCE
#include <dirent.h>     /* Defines DT_* constants */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct linux_dirent64 {
    unsigned long long d_ino;
    signed long long   d_off;
    unsigned short     d_reclen;
	unsigned char      d_type;
    char               d_name[0];
};

#define BUF_SIZE 1024

int
main(int argc, char *argv[])
{
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent64 *d;
    int bpos;
    char d_type;

   fd = open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
    if (fd == -1)
        handle_error("open");

   for ( ; ; ) {
        nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
        if (nread == -1)
            handle_error("getdents");

       if (nread == 0)
            break;

       printf("--------------- nread=%d ---------------\n", nread);
        printf("i-node#  file type  d_reclen  d_off   d_name\n");
        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent64 *) (buf + bpos);
            printf("%8ld  ", d->d_ino);
            d_type = d->d_type;
            printf("%-10s ", (d_type == DT_REG) ?  "regular" :
                             (d_type == DT_DIR) ?  "directory" :
                             (d_type == DT_FIFO) ? "FIFO" :
                             (d_type == DT_SOCK) ? "socket" :
                             (d_type == DT_LNK) ?  "symlink" :
                             (d_type == DT_BLK) ?  "block dev" :
                             (d_type == DT_CHR) ?  "char dev" : "???");
            printf("%4d %10lld  %s\n", d->d_reclen,
                    (long long) d->d_off, d->d_name);
            bpos += d->d_reclen;
        }
    }

   exit(EXIT_SUCCESS);
}
