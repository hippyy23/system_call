/// @file client.c
/// @brief Contiene l'implementazione del client.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/msg.h>

#include "defines.h"
#include "client_functions.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "err_exit.h"


int main(int argc, char * argv[]) {
    // check command line input arguments
    if (argc != 2) {
        printf("Usage: %s path\n", argv[0]);
        return 0;
    }
    g_wd = argv[1];

    // create a mask to block all signals but SIGINT and SIGUSR1
    // set of signals
    sigset_t signalSet;
    // initialize signalSet
    sigfillset(&signalSet);
    // remove SIGINT and SIGUSR1 from the set
    sigdelset(&signalSet, SIGINT);
    sigdelset(&signalSet, SIGUSR1);
    // block all signals but SIGINT and SIGUSR1
    sigprocmask(SIG_SETMASK, &signalSet, NULL);

    // if SIGUSR1 is recieved client_0 finishes
    // if SIGINT is recieved client_0 starts
    if (signal(SIGUSR1, terminate_client0) == SIG_ERR ||
        signal(SIGINT, start_client0) == SIG_ERR)
        ErrExit("change signal handler failed");

    // wait for a signal to be recieved
    printf("waiting for a signal...\n");
    pause();

    // search for all files whose name starts with "send_"
    search(0);
    // get the number of files found
    int numFiles = get_num_files(g_files);
    printf("<Client_0> number of files found %d\n", numFiles);

    // send the number of files to the server through fifo1
    printf("<Client_0> opening fifo1 %s\n", g_fifo1);
    int fifo1FD = open_fifo(g_fifo1, O_WRONLY);

    if (write(fifo1FD, &numFiles, sizeof(numFiles)) != sizeof(numFiles)) {
        ErrExit("write failed");
    }

    int msqid = msgget(g_msgKey, 0);
    if (msqid == -1) {
        ErrExit("msgget failed");
    }

    printf("<Client_0> getting server's shared memory segment...\n");
    // get the server's shared memory segment
    int shmid = alloc_shared_memory(g_shmKey, sizeof(message_struct) * MAX_MESSAGES_PER_IPC);
    // attach the shared memory segment
    char *shmPtr = (char *) attach_shared_memory(shmid, 0);

    // get the semaphore set
    printf("<Client_0> getting server's semaphore set...\n");
    int semid = semget(g_semKey, 4, 0);
    int fifo2FD = open_fifo(g_fifo2, O_WRONLY);
    if (semid > 0) {
        // wait for response from server on shared memory segment
        semOp(semid, START_END, -1);
        if (*shmPtr == '*') {
            printf("<Client_0> recieved '*' from server\n");
            for (int child = 0; child < numFiles; child++) {
                pid_t pid = fork();

                if (pid == -1) {
                    ErrExit("fork failed");
                // check whether the running process is the parent or the child
                } else if (pid == 0) {
                    // code executed only by the child
                    pid_t pid = getpid();
                    // OPEN FILE
                    int fd = open(g_files[child], O_RDONLY, S_IRUSR);
                    if (fd == -1) {
                        ErrExit("open failed");
                    }

                    // CHECK THE NUMBER OF CHARS
                    int numChars = check_num_chars_in_file(fd);
                    // printf("<Client_%d> number of chars found %d\n", pid, numChars);

                    // GET THE SIZE OF EACH PART
                    // we have problems with len < 10 (5, 6, 9)
                    // if 5 - 2 1 1 1
                    // if 6 - 3 1 1 1
                    // if 9 - 3 2 2 2
                    // for the rest use ceil(numChars / 4)
                    int size = ceil(numChars / 4);
                    // printf("<Client_%d> size %d\n", pid, size);
                    // switch (numChars) {
                    //     case 5:
                    //     case 6:
                    //     case 9:
                    //     default:
                    // }

                    // BLOCK ALL CHILDS UNTIL EVERYONE HAS REACHED THIS POINT
                    semOp(semid, WAIT_CHILD, -1);
                    semOp(semid, WAIT_CHILD, 0);

                    // SEND EACH PART THROUGH IPCS

                    // 1. FIFO1
                    message_struct part1;
                    part1.pid = pid;
                    strncpy(part1.path, g_files[child], NAME_MAX);
                    // read from file a chunk of size 'size'
                    printf("<Client_%d> reading content from file...\n", pid);
                    read_from_file(fd, part1.content, size);
                    // write on FIFO1 the 1st part of the file of the size 'size'
                    printf("<Client_%d> writing on FIFO1...\n", pid);
                    write_fifo(fifo1FD, &part1, sizeof(part1));               
                    semOp(semid, SYNC_FIFO1, 1);

                    // 2. FIFO2
                    message_struct part2;
                    part2.pid = pid;
                    strncpy(part2.path, g_files[child], NAME_MAX);
                    // read from file a chunk of size 'size'
                    read_from_file(fd, part2.content, size);

                    // wait for server
                    // write on FIFO2 the 2st part of the file of the size 'size'
                    printf("<Client_%d> writing on FIFO2...\n", pid);
                    write_fifo(fifo2FD, &part2, sizeof(part2));                    
                    semOp(semid, SYNC_FIFO2, 1);
                    
                    // 3. MSGQUEUE
                    msgqueue_struct part3;
                    part3.mtype = 1;
                    part3.mtext.pid = pid;
                    strncpy(part3.mtext.path, g_files[child], NAME_MAX);
                    // read from file a chunk of size 'size'
                    read_from_file(fd, part3.mtext.content, size);
                    // send the message to the server
                    printf("<Client_%d> sending message through msgqueue...\n", pid);
                    size_t mSize = sizeof(msgqueue_struct) - sizeof(long);
                    if (msgsnd(msqid, &part3, mSize, 0) == -1) {
                        ErrExit("msgsnd failed");
                    }

                    // 4. SHDMEM
                    message_struct part4;
                    part4.pid = pid;
                    strncpy(part4.path, g_files[child], NAME_MAX);
                    // read from file a chunk of size 'size'
                    read_from_file(fd, part4.content, size);

                    // close file
                    printf("<Client_%d> closing the file %s...\n", pid, g_files[child]);
                    close(fd);
                    exit(0);
                }
            }
            // code executed only by the parent
            // wait the termination of all child processes
            while(wait(NULL) != -1);
        } else {
            ErrExit("Oops... something went wrong");
        }
    } else {
        ErrExit("semget failed");
    }
    printf("<Client_0> quitting...\n");
    return 0;
}
