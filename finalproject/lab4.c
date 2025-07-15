// Andrew Nieves
// Lab 04: My Unix Shell
// 2/16/2025

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<readline/readline.h> // Don't need this or the next line but i dont want to remove it in case it breaks something
#include<readline/history.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

// Greeting shell during startup
void init_shell() {
    clear(); // Clears terminals creen
    char* username = getenv("USER"); // Gets current username from environment variable
    printf("\n\n\nUSER is: @%s", username); // Prints the user's username
    printf("\n");
    sleep(1);
    clear();
}

// Function to take input, uses system call getline() to adjust the size of the buffer passed
int takeInput(char **str) {
    size_t len = 0;
    printf("\n>>>> ");
    ssize_t nread = getline(str, &len, stdin); // Read user input

    // If getline fails, return 1
    if (nread == -1) {
        return 1;
    }

    // Remove newline character from the input
    (*str)[strcspn(*str, "\n")] = 0;
    
    return strlen(*str) == 0;
}


// Function to print Current Directory
void printDir() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDirectory is: %s", cwd);
}

// Function to check and handle output redirection
int checkRedirection(char **parsed, char **filename) {
    for (int i = 0; parsed[i] != NULL; i++) { // Iterates through parsed for ">"
        if (strcmp(parsed[i], ">") == 0) { // If we find ">"
            parsed[i] = NULL;
            *filename = parsed[i + 1]; // Set filename for output redirection
            return 1;
        }
    }
    return 0;
}
// Function where the system command is executed
void execArgs(char **parsed) {
    char *filename = NULL;
    int redirect = checkRedirection(parsed, &filename);
    
    pid_t pid = fork(); // Forks child process
    if (pid == 0) {
        if (redirect) { // Checks for output redirections and redirects output to specified file using open(), and dup2()
            int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("File open failed");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (execvp(parsed[0], parsed) < 0) { // execvp() executes command
            perror("Execution failed");
        }
        exit(0);
    } else {
        wait(NULL); // The partent process waits for child to complete
    }
}

// Function where the piped system commands is executed
void execArgsPiped(char **parsed, char **parsedpipe) {
    int pipefd[2];
    pid_t p1, p2;
    
    if (pipe(pipefd) < 0) {
        perror("Pipe failed");
        return;
    }
    
    p1 = fork(); // First  child process that is forked
    if (p1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO); // The first child is written to the pipe using dup2
        close(pipefd[1]);
        execvp(parsed[0], parsed);
        exit(0);
    }
    
    p2 = fork();
    if (p2 == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO); // The second child reads from the pipe using dup2
        close(pipefd[0]);
        execvp(parsedpipe[0], parsedpipe);
        exit(0);
    }
    
    close(pipefd[0]); // Close the pipes
    close(pipefd[1]);
    wait(NULL); // Waits for children processes
    wait(NULL);
}


// Built in help command, got this from geeksforgeeks and program doesn't run without it
void openHelp() {
    puts("\n***WELCOME TO MY SHELL HELP***"
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in unix shell"
        "\n>pipe handling"
        "\n>improper space handling");
        "\noutput redirection to file";
    return;
}

// Function to execute builtin commands
int ownCmdHandler(char **parsed) {
    if (strcmp(parsed[0], "exit") == 0) {
        printf("\nGoodbye\n");
        exit(0);
    } else if (strcmp(parsed[0], "cd") == 0) {
        chdir(parsed[1]);
        return 1;
    } else if (strcmp(parsed[0], "help") == 0) {
        openHelp();
        return 1;
    }
    return 0;
}

// function for finding pipe "|"
int parsePipe(char *str, char **strpiped) {
    strpiped[0] = strsep(&str, "|"); // Splits input stirng at "|"
    strpiped[1] = strsep(&str, "|");
    return strpiped[1] != NULL; // Return 1 if a pipe is found
}

// function for parsing command words, tokenizes input string by spaces
void parseSpace(char *str, char **parsed) {
    int i;
    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

// Function that processes the entire string by checking for a pipe, tokenizing the command, and calls built in commands if possible
int processString(char *str, char **parsed, char **parsedpipe) {
    char *strpiped[2];
    int piped = parsePipe(str, strpiped);
    
    if (piped) {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);
    } else {
        parseSpace(str, parsed);
    }
    
    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}

// Initialize the shell
int main() {
    char *inputString = NULL; // Dynamically allocate the buffer
    char *parsedArgs[MAXLIST];
    char *parsedArgsPiped[MAXLIST];
    int execFlag;
    init_shell();
    
    while (1) {
        printDir();
        if (takeInput(&inputString)) // Take input dynamically
            continue;
        
        execFlag = processString(inputString, parsedArgs, parsedArgsPiped);
        
        if (execFlag == 1)
            execArgs(parsedArgs);
        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);
    }
    free(inputString); // Free the dynamically allocated memory
    return 0;
}
