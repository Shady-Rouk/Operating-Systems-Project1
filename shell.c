#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

// Adebola Ajayi
// Farouk Balogun

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;
pid_t child_process_pid = (pid_t)-1;

void handler_interrupt(int signum) {
  kill(child_process_pid, SIGKILL);
  printf("\n");
}

void handler_alarm(int signum) {
  kill(child_process_pid, SIGKILL);
}

int main() {
  // Stores the string typed into the command line.
  char command_line[MAX_COMMAND_LINE_LEN];
  char cmd_bak[MAX_COMMAND_LINE_LEN];

  // Stores the tokenized command line input.
  char *arguments[MAX_COMMAND_LINE_ARGS];
    
  while (true) {
    memset(arguments, 0, MAX_COMMAND_LINE_ARGS); //clear former arguments
    
    // getting path to current dir.
    char curr_dir[MAX_COMMAND_LINE_LEN];
    char* dir_ptr = getcwd(curr_dir, sizeof(curr_dir)); 
    do{ 
      // Print the shell prompt.
      printf("%s%s", curr_dir, prompt);
      fflush(stdout);

      // Read input from stdin and store it in command_line. If there's an
      // error, exit immediately. (If you want to learn more about this line,
      // you can Google "man fgets")
      if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
        fprintf(stderr, "fgets error");
        exit(0);
      }

    }while(command_line[0] == 0x0A);  // while just ENTER pressed
  
    // If the user input was EOF (ctrl+d), exit the shell.
    if (feof(stdin)) {
      printf("\n");
      fflush(stdout);
      fflush(stderr);
      return 0;
    }
  
    // TODO:
    // 
    // 0. Modify the prompt to print the current working directory
        
    // 1. Tokenize the command line input (split it on whitespace)
    // command_line is a string containing what was typed into the terminal
    char* token = strtok(command_line, delimiters);
    int i = 0;
    while (token != NULL) {
      token[strcspn(token, "\r\n")] = 0;
      arguments[i] = token;
      token = strtok(NULL, " ");
      i++;
      }
    
    // if arguments[last value of i] == "&", run in background? else
    if (strcmp(arguments[i-1], "&")==0) {/*fork off child and run program*/
      arguments[i-1] = NULL;  
      pid_t background_process;
      background_process = fork();
      if (background_process<0) {
        printf("Error: Could not start process in the background\n");
      }
      else if (background_process==0) {
        if (strcmp(arguments[0], "cd")==0) {
          if (arguments[1] == NULL) {
            chdir(getenv("HOME"));
          }
          else if (chdir(arguments[1]) != 0) {
            printf("%s", arguments[1]);
            perror("");
          }
        }
      
        else if (strcmp(arguments[0], "pwd")==0) {
          printf("%s\n", curr_dir);
        }
      
        else if (strcmp(arguments[0], "echo")==0) {
          int j=1;
          // if the arguments[j] is an env (starts with $), print value stored instead of literal
          if (arguments[j][0] == '$') { 
              char *actual_env = arguments[j]+1;
              printf("%s", getenv(actual_env));
              }
          else {printf("%s", arguments[j]);}
            j++;
          while (arguments[j] != NULL) {
            // if the arguments[j] is an env (starts with $), print value stored instead of literal
            if (arguments[j][0] == '$') { 
              char *actual_env = arguments[j]+1;
              printf(" %s", getenv(actual_env));
              }
            else {printf(" %s", arguments[j]);}
            j++;
          }
          printf("\n");
        }
      
        else if (strcmp(arguments[0], "exit")==0) {
          exit(0);
        }
      
        else if (strcmp(arguments[0], "env")==0) {
          if (arguments[1] == NULL) {
            size_t i=0;
            for (i=0; environ[i]!=NULL; i++) {
              printf("%s\n", environ[i]);
            }
          }
          else {
            printf("%s\n", getenv(arguments[1]));
          }
        }
      
        else if (strcmp(arguments[0], "setenv")==0) {
          char *params[2]; // var_name and var_value
          char* var_token = strtok(arguments[1], "=\n\r\t");
          int x = 0;
          while (var_token != NULL) {
            // var_token[strcspn(var_token, "\r\n")] = 0;
            params[x] = var_token;
            var_token = strtok(NULL, " ");
            x++;
          }
          setenv(params[0], params[1], 1);
        }
      
        // 3. Create a child process which will execute the command line input
        else {
          pid_t pid;
          int status;
          pid = fork();
          if (pid < 0) {
            printf("Error: Could not execute requested process\n");
          } 
          else if (pid == 0) {
            if (execvp(arguments[0], arguments) == -1) {
              perror("execvp() failed: ");
              printf("An error occurred\n");
            }
            exit(0);
          } 
          else {
            wait(NULL);
          }  
        }
      }
    }
    else {
      // 2. Implement Built-In Commands
      if (strcmp(arguments[0], "cd")==0) {
        if (arguments[1] == NULL) {
          chdir(getenv("HOME"));
        }
        else if (chdir(arguments[1]) != 0) {
          printf("%s", arguments[1]);
          perror("");
        }
      }
      
      else if (strcmp(arguments[0], "pwd")==0) {
        printf("%s\n", curr_dir);
      }
      
      else if (strcmp(arguments[0], "echo")==0) {
        int j=1;
        // if the arguments[j] is an env (starts with $), print value stored instead of literal
        if (arguments[j][0] == '$') { 
            char *actual_env = arguments[j]+1;
            printf("%s", getenv(actual_env));
            }
        else {printf("%s", arguments[j]);}
          j++;
        while (arguments[j] != NULL) {
          // if the arguments[j] is an env (starts with $), print value stored instead of literal
          if (arguments[j][0] == '$') { 
            char *actual_env = arguments[j]+1;
            printf(" %s", getenv(actual_env));
            }
          else {printf(" %s", arguments[j]);}
          j++;
        }
        printf("\n");
      }
      
      else if (strcmp(arguments[0], "exit")==0) {
        exit(0);
      }
      
      else if (strcmp(arguments[0], "env")==0) {
        if (arguments[1] == NULL) {
          size_t i=0;
          for (i=0; environ[i]!=NULL; i++) {
            printf("%s\n", environ[i]);
          }
        }
        else {
          printf("%s\n", getenv(arguments[1]));
        }
      }
      
      else if (strcmp(arguments[0], "setenv")==0) {
        char *params[2]; // var_name and var_value
        char* var_token = strtok(arguments[1], "=\n\r\t");
        int x = 0;
        while (var_token != NULL) {
          params[x] = var_token;
          var_token = strtok(NULL, " ");
          x++;
        }
        setenv(params[0], params[1], 1);
      }
  
      // 3. Create a child process which will execute the command line input
      else {
        pid_t pid;
        int status;
        pid = fork();
        if (pid < 0) {
          printf("Error: Could not execute requested process\n");
        } 
        else if (pid == 0) {
          if (execvp(arguments[0], arguments) == -1) {
            perror("execvp() failed: ");
            printf("An error occurred\n");
          }
          exit(0);
        } 
        else {

          // 4. The parent process should wait for the child to complete unless its a background process
          child_process_pid = pid;
          signal(SIGINT, handler_interrupt);
          signal(SIGALRM, handler_alarm);
          alarm(10);
          wait(NULL);
        }
        
      }
    }

  }
  // This should never be reached.
  return -1;
}
