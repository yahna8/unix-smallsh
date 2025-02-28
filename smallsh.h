#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT 2048
#define MAX_ARGS 512

// Global variables
int foregroundOnly = 0;  // Mode toggle for foreground-only mode
int lastExitStatus = 0;  // Stores exit status of last foreground process
pid_t bgProcesses[50];   // Array to store background process IDs
int bgCount = 0;         // Number of background processes

// Function prototypes
void prompt();
void handle_SIGTSTP(int signo);
void executeCommand(char *args[], int background, char *inputFile, char *outputFile);
void checkBackgroundProcesses();