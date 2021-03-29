#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

/* Returns 1 if signal is masked */
int sigismasked(int sig) {
    int check;
    sigset_t sig_mask;
    sigemptyset(&sig_mask);

    check = sigprocmask(SIG_BLOCK, NULL, &sig_mask);
    if (check < 0) {
        exit(EXIT_FAILURE);
    }

    return sigismember(&sig_mask, sig);
}

void mask_print_handler(int sig) {
    sigset_t sig_mask;
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, sig);

    printf("%d: signal %s blocked when func is called\n", sig, sigismasked(sig)? "is" : "is NOT");
    signal(sig, mask_print_handler);
    printf("%d: signal %s blocked after signal()\n", sig, sigismasked(sig)? "is" : "is NOT");
    sigprocmask(SIG_UNBLOCK, &sig_mask, NULL);
    printf("%d: signal %s blocked after sigprocmask()\n", sig, sigismasked(sig)? "is" : "is NOT");
}

int main() {
    int pid;
    pid = getpid();

    signal(SIGCHLD, mask_print_handler);
    kill(pid, SIGCHLD);

    signal(SIGUSR1, mask_print_handler);
    kill(pid, SIGUSR1);
    return 0;
}
