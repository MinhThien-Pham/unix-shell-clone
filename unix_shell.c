#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Define path separator based on OS
#ifdef _WIN32 // Windows
  #define PATH_SEPARATOR ";"
#else // Unix-like systems
  #define PATH_SEPARATOR ":"
#endif

// Define max arguments
#define MAX_ARGS 100

// Simple command-line interpreter
enum command_type {
  CMD_EXIT,
  CMD_ECHO,
  CMD_TYPE,
  CMD_PWD
};

int get_command_type(const char *cmd) {
  if (strcmp(cmd, "echo") == 0) {
    return CMD_ECHO;
  } else if (strcmp(cmd, "type") == 0) {
    return CMD_TYPE;
  } else if (strcmp(cmd, "exit") == 0) {
    return CMD_EXIT;
  } else if (strcmp(cmd, "pwd") == 0) {
    return CMD_PWD;
  } else {
    return -1; // Unknown command
  }
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char command[1024];

  while (1) {
    printf("$ ");
    // Read command from stdin
    fgets(command, sizeof(command), stdin);
    // Remove trailing newline
    command[strcspn(command, "\n")] = '\0';

    // Split command line into argv[]
    char *argv_list[MAX_ARGS];
    int argc_count = 0;
    char *token = strtok(command, " ");
    while (token != NULL && argc_count < MAX_ARGS) {
      argv_list[argc_count++] = token;
      token = strtok(NULL, " ");
    }

    if (argc_count == 0) {
      continue; // Empty command
    }
    
    char *cmd = argv_list[0];
    int cmd_type = get_command_type(cmd); // Get the command type

    if (cmd_type == CMD_EXIT) {
      break;
    } else if (cmd_type == CMD_ECHO) {
        for (int i = 1; i < argc_count; i++) {
          printf("%s ", argv_list[i]);
        }
        printf("\n");
    } else if (cmd_type == CMD_TYPE) {
        if(argc_count < 2) {
          printf("type: missing argument\n");
          continue;
        }
        if (get_command_type(argv_list[1]) != -1) { // Check if it's a builtin
          printf("%s is a shell builtin\n", argv_list[1]);
        } else { // Search in PATH
          char *path_env = getenv("PATH"); // Get PATH environment variable
          char *path_copy = strdup(path_env); // Duplicate PATH string for tokenization
          char *dir = strtok(path_copy, PATH_SEPARATOR); // Tokenize PATH
          int found = 0;

          while (dir != NULL) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, argv_list[1]); // Construct full path
            if (access(full_path, X_OK) == 0) {
              printf("%s is %s\n", argv_list[1], full_path);
              found = 1;
              break;
            }
            dir = strtok(NULL, PATH_SEPARATOR);
          }
          free(path_copy);
          if (!found) {
            printf("%s: not found\n", argv_list[1]);
          }
        }
    } else if (cmd_type == CMD_PWD) {
        char pwd[1024]; // Buffer for current working directory
        // Get current working directory
        if (getcwd(pwd, sizeof(pwd)) != NULL) {
            printf("%s\n", pwd);
        } else {
            perror("getcwd");
        }
    } else {
        char *path_env = getenv("PATH"); // Get PATH environment variable
        char *path_copy = strdup(path_env); // Duplicate PATH string for tokenization
        char *dir = strtok(path_copy, PATH_SEPARATOR); // Tokenize PATH
        int found = 0;
        char executable_path[1024];
        while (dir != NULL) {
          snprintf(executable_path, sizeof(executable_path), "%s/%s", dir, cmd); // Construct full path
          if (access(executable_path, X_OK) == 0) { // Check if executable
            found = 1;
            break;
          }
          dir = strtok(NULL, PATH_SEPARATOR);
        }
        free(path_copy);
        if (!found) {
          printf("%s: command not found\n", cmd);
        } else{
            char cmdline[1024];
            snprintf(cmdline, sizeof(cmdline), "%s", cmd);
            for (int i = 1; i < argc_count; i++) { // Handle arguments
              int current_len = strlen(cmdline);
              int len = snprintf(cmdline + current_len, sizeof(cmdline) - current_len, " %s", argv_list[i]);
              if (len < 0 || len >= sizeof(cmdline) - current_len) {
                printf("Command line too long\n");
                break;
              }
            }
            int result = system(cmdline);
            if(result == -1) {
              perror("system");
            }
        }
    }
  }
  return 0;
}
