/* CSCI 4500 Program 1
 * Author: An Vu
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 100
#define MAX_WORDS 16

extern char **environ;

/* Parse $PATH into directories */
typedef struct {
    char *dirs[64];
    int count;
} PathDirs;

void get_path_dirs(PathDirs *pd) {
    char *path = getenv("PATH");
    pd->count = 0;
    char *start = path;
    while (start && *start) {
        char *end = strchr(start, ':');
        if (!end) end = start + strlen(start);
        int len = end - start;
        pd->dirs[pd->count] = malloc(len + 2);
        strncpy(pd->dirs[pd->count], start, len);
        pd->dirs[pd->count][len] = '\0';
        pd->count++;
        start = (*end) ? end + 1 : NULL;
    }
}

char *resolve_path(char *cmd, PathDirs *pd) {
    if (strchr(cmd, '/')) {
        return access(cmd, X_OK) == 0 ? strdup(cmd) : NULL;
    }
    for (int i = 0; i < pd->count; ++i) {
        static char buf[256];
        snprintf(buf, sizeof(buf), "%s/%s", pd->dirs[i], cmd);
        if (access(buf, X_OK) == 0) return strdup(buf);
    }
    return NULL;
}

void print_status(pid_t pid, int status) {
    if (WIFEXITED(status)) {
        dprintf(1, "process %d terminated normally with status %d\n", pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        dprintf(1, "process %d terminated abnormally for reason %d\n", pid, WTERMSIG(status));
    }
}

int main() {
    char buf[MAX_LINE];
    char *words[MAX_WORDS + 1];
    PathDirs pd;
    get_path_dirs(&pd);

    int i = 0;
    while (1) {
        char ch;
        int n = read(0, &ch, 1);
        if (n <= 0) break;
        write(1, &ch, 1);
        if (ch == '\n') {
            buf[i] = '\0';
            /* tokenize */
            int wordc = 0;
            char *tok = strtok(buf, " \t\n");
            while (tok && wordc < MAX_WORDS) {
                words[wordc++] = tok;
                tok = strtok(NULL, " \t\n");
            }
            words[wordc] = NULL;
            
            if (wordc == MAX_WORDS && tok != NULL) {
                dprintf(2, "Too many words\n");
                goto next;
            }

            if (wordc == 0) {
                i = 0; continue; /* empty line */
            }

            int pipe_index = -1;
            for (int j = 0; j < wordc; ++j) {
                if (strcmp(words[j], "|") == 0) {
                    if (pipe_index != -1) {
                        dprintf(1, "Too many pipes\n");
                        goto next;
                    }
                    pipe_index = j;
                }
            }

            /* Split args */
            char *left_argv[MAX_WORDS + 1] = {0};
            char *right_argv[MAX_WORDS + 1] = {0};

            if (pipe_index == -1) {
                for (int j = 0; j < wordc; ++j) left_argv[j] = words[j];
            } else {
                if (pipe_index == 0 || pipe_index == wordc - 1) {
                    dprintf(1, "Malformed pipe\n");
                    goto next;
                }
                for (int j = 0; j < pipe_index; ++j) left_argv[j] = words[j];
                for (int j = pipe_index + 1; j < wordc; ++j) right_argv[j - pipe_index - 1] = words[j];
            }

            /* Resolve paths */
            char *left_path = resolve_path(left_argv[0], &pd);
            if (!left_path) {
                dprintf(1, "%s: not found\n", left_argv[0]);
                goto next;
            }
            char *right_path = NULL;
            if (pipe_index != -1) {
                right_path = resolve_path(right_argv[0], &pd);
                if (!right_path) {
                    dprintf(1, "%s: not found\n", right_argv[0]);
                    free(left_path);
                    goto next;
                }
            }

            if (pipe_index == -1) {
                pid_t pid = fork();
                if (pid == 0) {
                    execve(left_path, left_argv, environ);
                    dprintf(1, "%s: exec failed\n", left_path);
                    _exit(127);
                }
                int status;
                waitpid(pid, &status, 0);
                print_status(pid, status);
            } else {
                int p[2];
                pipe(p);

                pid_t pid1 = fork();
                if (pid1 == 0) {
                    close(1); dup(p[1]); close(p[0]); close(p[1]);
                    execve(left_path, left_argv, environ);
                    dprintf(1, "%s: exec failed\n", left_path);
                    _exit(127);
                }

                pid_t pid2 = fork();
                if (pid2 == 0) {
                    close(0); dup(p[0]); close(p[0]); close(p[1]);
                    execve(right_path, right_argv, environ);
                    dprintf(1, "%s: exec failed\n", right_path);
                    _exit(127);
                }

                close(p[0]); close(p[1]);
                int st1, st2;
                waitpid(pid1, &st1, 0);
                waitpid(pid2, &st2, 0);
                print_status(pid1, st1);
                print_status(pid2, st2);
                free(right_path);
            }
            free(left_path);
        next:
            i = 0;
        } else if (i < MAX_LINE - 1) {
            buf[i++] = ch;
        }
    }
    // Free path dirs before exiting
    for (int k = 0; k < pd.count; ++k) {
        free(pd.dirs[k]);
    }
    return 0;
}
