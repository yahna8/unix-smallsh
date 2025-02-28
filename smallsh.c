/**
 * @file smallsh.c
 * @brief A small Unix shell implementation in C.
 *
 * This shell provides functionality for executing commands,
 * handling input/output redirection, background execution,
 * and built-in commands (`exit`, `cd`, `status`).
 * It also includes custom signal handlers for `SIGINT` (Ctrl+C)
 * and `SIGTSTP` (Ctrl+Z).
 */

 #include "smallsh.h"

 /** 
  * @brief Signal handler for SIGTSTP (Ctrl+Z).
  * 
  * Toggles foreground-only mode when Ctrl+Z is pressed.
  * If in foreground-only mode, background execution (`&`) is ignored.
  * 
  * @param signo Signal number (SIGTSTP).
  */
 void handle_SIGTSTP(int signo) {
     if (foregroundOnly == 0) {
         foregroundOnly = 1;
         write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n", 50);
     } else {
         foregroundOnly = 0;
         write(STDOUT_FILENO, "\nExiting foreground-only mode\n", 30);
     }
 }
 
 /**
  * @brief Displays the command prompt to the user.
  *
  * Uses `fflush()` for immediate display of output.
  */
 void prompt() {
     printf(": ");
     fflush(stdout);
 }
 
 /**
  * @brief Checks for completed background processes and reaps them.
  *
  * Uses `waitpid()` with `WNOHANG` flag to check if a background process has completed.
  * If a process has terminated, prints its exit status or termination signal.
  */
 void checkBackgroundProcesses() {
     int status;
     pid_t pid;
     for (int i = 0; i < bgCount; i++) {
         pid = waitpid(bgProcesses[i], &status, WNOHANG);
         if (pid > 0) {
             printf("background pid %d is done: ", pid);
             if (WIFEXITED(status)) {
                 printf("exit value %d\n", WEXITSTATUS(status));
             } else {
                 printf("terminated by signal %d\n", WTERMSIG(status));
             }
             fflush(stdout);
         }
     }
 }
 
 /**
  * @brief Executes a given command with I/O redirection and background execution.
  *
  * Handles:
  * - Input redirection (`<`)
  * - Output redirection (`>`)
  * - Background execution (`&`)
  * - Foreground execution with proper signal handling
  *
  * @param args Array of command arguments (null-terminated).
  * @param background Flag for background execution (`1` = background, `0` = foreground).
  * @param inputFile Name of input file (`NULL` if not specified).
  * @param outputFile Name of output file (`NULL` if not specified).
  */
 void executeCommand(char *args[], int background, char *inputFile, char *outputFile) {
     pid_t spawnPid = fork();
     int childStatus;
 
     if (spawnPid == -1) {
         perror("fork() failed");
         exit(1);
     } else if (spawnPid == 0) { // Child process
         // Handle input redirection
         if (inputFile) {
             int inputFD = open(inputFile, O_RDONLY);
             if (inputFD == -1) {
                 perror("cannot open input file");
                 exit(1);
             }
             dup2(inputFD, STDIN_FILENO);
             close(inputFD);
         }
 
         // Handle output redirection
         if (outputFile) {
             int outputFD = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
             if (outputFD == -1) {
                 perror("cannot open output file");
                 exit(1);
             }
             dup2(outputFD, STDOUT_FILENO);
             close(outputFD);
         }
 
         // Set signal handling
         struct sigaction ignoreAction = {0};
         ignoreAction.sa_handler = SIG_IGN;
         sigaction(SIGTSTP, &ignoreAction, NULL); // Ignore Ctrl+Z in child process
 
         if (!background) {
             struct sigaction defaultAction = {0};
             defaultAction.sa_handler = SIG_DFL;
             sigaction(SIGINT, &defaultAction, NULL); // Allow Ctrl+C in foreground
         }
 
         // Execute the command
         if (execvp(args[0], args) == -1) {
             perror("command not found");
             exit(1);
         }
     } else { // Parent process
         if (background && !foregroundOnly) {
             printf("background pid is %d\n", spawnPid);
             fflush(stdout);
             bgProcesses[bgCount++] = spawnPid;
         } else {
             waitpid(spawnPid, &childStatus, 0); // Wait for foreground process
             if (WIFEXITED(childStatus)) {
                 lastExitStatus = WEXITSTATUS(childStatus);
             } else {
                 printf("terminated by signal %d\n", WTERMSIG(childStatus));
                 fflush(stdout);
                 lastExitStatus = WTERMSIG(childStatus);
             }
         }
     }
 }
 
 /**
  * @brief Main function that runs the smallsh shell.
  *
  * - Initializes signal handlers for `SIGTSTP` (Ctrl+Z) and `SIGINT` (Ctrl+C).
  * - Reads user input, processes built-in commands, and executes external commands.
  * - Handles input/output redirection and background execution.
  *
  * @return `EXIT_SUCCESS` when the shell terminates.
  */
 int main() {
     struct sigaction SIGTSTP_action = {0};
     SIGTSTP_action.sa_handler = handle_SIGTSTP;
     sigfillset(&SIGTSTP_action.sa_mask);
     SIGTSTP_action.sa_flags = SA_RESTART;
     sigaction(SIGTSTP, &SIGTSTP_action, NULL);
 
     struct sigaction SIGINT_action = {0};
     SIGINT_action.sa_handler = SIG_IGN;
     sigfillset(&SIGINT_action.sa_mask);
     SIGINT_action.sa_flags = 0;
     sigaction(SIGINT, &SIGINT_action, NULL);
 
     while (1) {
         checkBackgroundProcesses();
         prompt();
 
         char input[MAX_INPUT];
         if (!fgets(input, MAX_INPUT, stdin)) {
             clearerr(stdin);
             continue;
         }
 
         if (input[0] == '#' || input[0] == '\n') continue; // Ignore comments & blank lines
 
         char *args[MAX_ARGS];
         char *token = strtok(input, " \n");
         int argCount = 0;
         int background = 0;
         char *inputFile = NULL, *outputFile = NULL;
 
         while (token) {
             if (strcmp(token, "<") == 0) {
                 token = strtok(NULL, " \n");
                 inputFile = token;
             } else if (strcmp(token, ">") == 0) {
                 token = strtok(NULL, " \n");
                 outputFile = token;
             } else if (strcmp(token, "&") == 0 && strtok(NULL, " \n") == NULL) {
                 background = 1;
             } else {
                 args[argCount++] = token;
             }
             token = strtok(NULL, " \n");
         }
         args[argCount] = NULL;
 
         if (argCount == 0) continue;
 
         // Handle built-in commands
         if (strcmp(args[0], "exit") == 0) {
             for (int i = 0; i < bgCount; i++) {
                 kill(bgProcesses[i], SIGTERM);
             }
             exit(0);
         } else if (strcmp(args[0], "cd") == 0) {
             if (argCount > 1) {
                 if (chdir(args[1]) != 0) {
                     perror("cd failed");
                 }
             } else {
                 chdir(getenv("HOME"));
             }
             continue;
         } else if (strcmp(args[0], "status") == 0) {
             printf("exit value %d\n", lastExitStatus);
             fflush(stdout);
             continue;
         }
 
         // Execute non-built-in commands
         executeCommand(args, background, inputFile, outputFile);
     }
 }
 