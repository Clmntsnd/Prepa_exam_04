#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void perr(char *s)
{
    while(*s)
        write(1, s++, 1);
}

int cd (char **av, int i)
{
    if (i != 2)
        return(perr("error: cd: bad arguments\n"), 1);
    else if (chdir(av[1]) == -1)
        return(perr("error: cd: cannot change directory to "), perr(av[1]), perr("\n"), 1);
    return (0);
}

int exec(char **av, char **env, int i)
{
    int status;
    int fd[2];
    int pip = (av[i] && !strcmp(av[i], "|")); // exist if av[i] is "|"

    if (pipe(fd) == -1)
        return (perr("error: fatal\n"), 1);
    int pid = fork();
    if (!pid)
    {
        av[i] = 0;
        if (pip && (dup2(fd[1] , 1) == -1 || close(fd[0]) || close(fd[1])))
            return (perr("error: fatal\n"), 1);
        execve(*av, av, env);
        return (perr("error: cannot execute "), perr(*av), perr("\n"), 1);
    }
    waitpid(pid, &status, 0);
    if (pip && (dup2(fd[0] , 0) == -1 || close(fd[0]) || close(fd[1])))
        return (perr("error: fatal\n"), 1);
    return WIFEXITED(status) && WEXITSTATUS(status);
}

int main(int ac, char **av, char **env)
{
    (void)ac;
    int i = 0;
    int status = 0;

    while(*av && *(av + 1))
    {
        ++av;
        i = 0;
        while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
            i++;
        if (!strcmp(*av, "cd"))
            status = cd(av, i);
        else if (i)
            status = exec(av, env, i);
        av += i;
    }
    return status;
}