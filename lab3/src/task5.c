#include <unistd.h>
#include <sys/types.h>

int main (int argc, char** argv)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Process not crated");
        return 1;
    }
    if (pid == 0)
    {
        execv("sequentalMinMax", argv);
    }
    wait(NULL);
    return 0;
}