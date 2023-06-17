#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

/* In the code most of the arrays have the size of CMDLINE_MAX */
#define CMDLINE_MAX 512


/* The struct contains the variables used the most often in the code.*/
/* In this code, we have a lot of duplicates for cmd, ParsedArgs etc because we didn't
want to modify them */
struct process {
  // A duplicate for cmd
  char newCmd[CMDLINE_MAX];
  // cmdCopy will have another copy of the cmd
  char cmdCopy[CMDLINE_MAX];
  // This count the number of pipes in the cmd given by the user
  int pipeCounter;
  char directoryName[CMDLINE_MAX];
  /*This stores the pwd (directory) in the array if the user inputs
  pushd or popd*/
  char dirs[CMDLINE_MAX][CMDLINE_MAX];
  /*Notes the current directory, so that after cd, it checks whether we
  changed the directory or not */
  char prevDirectory[CMDLINE_MAX];
  // position specifies the empty index of the dirs array
  int position;
  /*This is a very interesting variable, if enter = 0, execvp is run,
  if enter = 1, execvp is not run */
  int Enter;
  // Parsedcmd is parsing the input based on space
  char* Parsedcmd[CMDLINE_MAX];
  // ParsedArgs parses the array based on pipes
  char *ParsedArgs[CMDLINE_MAX];
  // A duplicate of ParsedArgs
  char ParsedArgsCpy_[CMDLINE_MAX];
  // Another duplicate of ParsedArgs
  char ParsedArgsCpy[CMDLINE_MAX];
  /*This will store the file name if it involves in input/output
  redirection */
  char FileName[CMDLINE_MAX];
  // index is to show the current free index for the Parsedcmd array
  int index;
  int status[CMDLINE_MAX];
};


// The function executes if the cd is in cmd
void cd (char* mainArg, struct process MyProcess, char cmd[CMDLINE_MAX]){
  // copies the destination into mainArg and breaks it on the basis of " "
  mainArg = strtok(NULL, " ");
  strcpy(MyProcess.prevDirectory, getcwd(MyProcess.directoryName, CMDLINE_MAX));
  chdir(mainArg);

  // If the directory didn't change, then print the error condition
  if (strcmp(getcwd(MyProcess.directoryName, CMDLINE_MAX),
  MyProcess.prevDirectory) == 0){

          fprintf(stderr, "Error: cannot cd into directory\n");
          fprintf(stderr, "+ completed '%s' [1]\n", cmd);
  }
  else {
      fprintf(stderr, "+ completed '%s' [0]\n", cmd);
  }
}

// The function executes if the pushd is in cmd
struct process pushd(char* mainArg, struct process MyProcess, char cmd[CMDLINE_MAX]){
  // If the index 0 of the dirs array is empty
  if (MyProcess.position == 0){
    // copies the directory path to dirs
    strcpy(MyProcess.dirs[0], getcwd(MyProcess.directoryName, CMDLINE_MAX));
    ++MyProcess.position; // increment the position by 1
  }
  mainArg = strtok(NULL, " ");
  strcpy(MyProcess.prevDirectory, getcwd(MyProcess.directoryName, CMDLINE_MAX));
  if (strcmp(mainArg, "..") != 0){ // if it is not cd ..
      chdir(mainArg); // change the directory
  }

  // If the directory didn't change, then print the error condition
  if (strcmp(getcwd(MyProcess.directoryName, CMDLINE_MAX),
  MyProcess.prevDirectory) == 0){
    fprintf(stderr, "Error: no such directory\n");
    fprintf(stderr, "+ completed '%s' [1]\n", cmd);
  }
  else {
    // receives the currentDirectory
    char* currentDirectory = getcwd(MyProcess.directoryName, CMDLINE_MAX);
    // copies the directory path to dirs
    strcpy(MyProcess.dirs[MyProcess.position], currentDirectory);
    MyProcess.position += 1; // increment the position
    fprintf(stderr, "+ completed '%s' [0]\n", cmd);
  }

  return MyProcess;

}

// The function executes if the popd is in cmd
struct process popd(struct process MyProcess, char cmd[CMDLINE_MAX]){
  // If the index 0 of the dirs array is empty
  if (MyProcess.position == 0){
      // copies the directory path to dirs
      strcpy(MyProcess.dirs[0], getcwd(MyProcess.directoryName, CMDLINE_MAX));
      ++MyProcess.position; // increment the position
  }
  strcpy(MyProcess.prevDirectory, getcwd(MyProcess.directoryName, CMDLINE_MAX));
  chdir(".."); // same as cd ..
  // receives the currentDirectory
  // If the directory didn't change, then print the error condition
  if (strcmp(getcwd(MyProcess.directoryName, CMDLINE_MAX),
    MyProcess.prevDirectory) == 0){
    fprintf(stderr, "Error: directory stack empty\n");
    fprintf(stderr, "+ completed '%s' [1]\n", cmd);
  }
  else {
    // receives the currentDirectory
    char* currentDirectory = getcwd(MyProcess.directoryName, CMDLINE_MAX);
    // copies the directory path to dirs
    strcpy(MyProcess.dirs[MyProcess.position], currentDirectory);
    MyProcess.position += 1; // increment the position
    fprintf(stderr, "+ completed '%s' [0]\n", cmd);
  }

  return MyProcess;
}

// The function executes if the dirs is in cmd
struct process dirs(struct process MyProcess, char cmd[CMDLINE_MAX]){
  // If the index 0 of the dirs array is empty
  if (MyProcess.position == 0){
      // copies the directory path to dirs
      strcpy(MyProcess.dirs[0], getcwd(MyProcess.directoryName, CMDLINE_MAX));
      printf("%s\n", MyProcess.dirs[0]);
      ++MyProcess.position; // increment the position
  }
  else{

      // Printing the dirs array from reverse
      for (int i = MyProcess.position - 1; i >= 0; --i){
          printf("%s\n", MyProcess.dirs[i]);
      }
  }
  fprintf(stderr, "+ completed '%s' [0]\n", cmd);
  return MyProcess;
}



/* The pipeFinder function finds the number of pipes in the input,
 and counts the number of pipes */
struct process pipeFinder(struct process MyProcess, char cmd[CMDLINE_MAX]){
  strcpy(MyProcess.newCmd, cmd);
  // Iterates through newCmd character by character to find a pipe
  for (unsigned int i = 0; i < strlen(MyProcess.newCmd); ++i){

      // if a pipe is found
      if (MyProcess.newCmd[i] == '|'){
          // If there no is no command before the pipe
          if (i == 0){
              fprintf(stderr, "Error: missing command\n");
              MyProcess.Enter = 1;
          }
          else {
            if (!(MyProcess.newCmd[i+2]) ){
              fprintf(stderr, "Error: missing command\n");
              MyProcess.Enter = 1;
            }
            else if (!(MyProcess.newCmd[i-2]) ){
              fprintf(stderr, "Error: missing command\n");
              MyProcess.Enter = 1;
            }
          }
            // pipeCounter is incremented
          ++MyProcess.pipeCounter;
      }
  }
  //pipeCounter finds the total number of pipes in the command
  return MyProcess;
}


// If the command is involving output redirection
struct process outputRedir(struct process MyProcess, int sd){
  // breaks the input based on '>'
  char *SplitoutRedir = strtok(MyProcess.ParsedArgsCpy_, ">");

  char *FileoutRedir = strtok(NULL, ">");
  if (FileoutRedir != NULL){
    // locates the empty index of FileName array
    int FileNameIndex = 0;
    int spaceIndex = 0;

    MyProcess.FileName[FileNameIndex] = '\0';

    // specifies the length of the file
    int lenFile = strlen(FileoutRedir);
    /* his loops FileName the trimmed version of FileNameIndex (removing
    the leading spaces) */
    while (lenFile > FileNameIndex){
        if (FileoutRedir[spaceIndex] == ' '){
        }
        else {
            MyProcess.FileName[FileNameIndex] = FileoutRedir[spaceIndex];
            ++FileNameIndex; // incrementing the index
        }
        ++spaceIndex;
    }
    MyProcess.FileName[FileNameIndex] = '\0';


    strcpy(MyProcess.ParsedArgsCpy, SplitoutRedir);
    MyProcess.ParsedArgsCpy[strlen(SplitoutRedir)] = '\0';
    // SplitCmd are the parsed values of the command
    char* SplitCmd = strtok(MyProcess.ParsedArgsCpy, " ");
    while (SplitCmd != NULL){

        // All the parsed values of the input based on space is stored in the Parsedcmd array
        MyProcess.Parsedcmd[MyProcess.index] = SplitCmd;
        SplitCmd = strtok(NULL, " ");
        ++MyProcess.index;
    }
    MyProcess.Parsedcmd[MyProcess.index] = NULL;
    ++MyProcess.index;

    // open and write in the file
    sd = open(MyProcess.FileName, O_WRONLY | O_CREAT, 0644);
    if (sd < 0){ // If unable to open
      if (strcmp(FileoutRedir, " ") == 0){
        fprintf(stderr, "Error: no output file\n");
        MyProcess.Enter = 1;
      }
      else {
        fprintf(stderr, "Error: cannot open output file\n");
        MyProcess.Enter = 1;
      }
    }
    else {
      dup2(sd,STDOUT_FILENO);
      close(sd); // Close the file
    }
  }
  else {
    // No file present
    fprintf(stderr, "Error: no output file\n");
  }

  return MyProcess;
}



// If the command is involving input redirection
struct process inputRedir(struct process MyProcess, int sd){
  // breaks the input based on '<'
  char *SplitinRedir = strtok(MyProcess.ParsedArgsCpy_, "<");
  char *FileinRedir = strtok(NULL, "<");
  // If a file is present
  if (FileinRedir != NULL){
    // locates the empty index of FileName array
    int FileNameindex=  0;
    int spaceindex = 0;
    MyProcess.FileName[FileNameindex] = '\0';
    int lenFile = strlen(FileinRedir);

    /* his loops FileName the trimmed version of FileNameIndex (removing
    the leading spaces) */
    while (lenFile > FileNameindex){
        if (FileinRedir[spaceindex] != ' '){
            MyProcess.FileName[FileNameindex] = FileinRedir[spaceindex];
            ++FileNameindex; // increment in the fileName index
        }
        ++spaceindex;
    }
    MyProcess.FileName[FileNameindex] = '\0';
    strcpy(MyProcess.ParsedArgsCpy, SplitinRedir);
    MyProcess.ParsedArgsCpy[strlen(SplitinRedir)] = '\0';
    // SplitCmd are the parsed values of the command
    char* SplitCmd = strtok(MyProcess.ParsedArgsCpy, " ");
    while (SplitCmd != NULL){
        // All the parsed values of the input based on space is stored in the Parsedcmd array
        MyProcess.Parsedcmd[MyProcess.index] = SplitCmd;
        SplitCmd = strtok(NULL, " ");
        ++MyProcess.index;
    }
    MyProcess.Parsedcmd[MyProcess.index] = NULL;
    ++MyProcess.index;

    sd = open(MyProcess.FileName, O_RDONLY); // // open and read in the file
    if (sd < 0){ // If unable to open
        if (strcmp(FileinRedir, " ") == 0){
          fprintf(stderr, "Error: no input file\n");
          MyProcess.Enter = 1;
        }
        else {
          fprintf(stderr, "Error: cannot open input file\n");
          MyProcess.Enter = 1;
        }
    }
    else {
        dup2(sd,STDIN_FILENO);
        close(sd);
    }

  }
  else { // No file present
      fprintf(stderr, "Error: no input file\n");
  }
  return MyProcess;
}



/* Parses the input cmd */
void Parsing(struct process MyProcess, char cmd[CMDLINE_MAX]){

  // index is to show the current free index for the Parsedcmd array
  MyProcess.index = 0;
  // count is to show the current free index for the ParsedArgs array
  int count = 0;
  /*In our code, we don't wait in the same for loop as we use fork(),
  hence we remember all pids in an array, and use the array in another
  for loop */
  pid_t pids[CMDLINE_MAX];
  // parses the input based on pipes
  char* parsePipes = strtok(MyProcess.cmdCopy, "|");
  while (parsePipes != NULL){
    /*All the parsed values of the input based on pipes is stored in
    the ParsedArgs array */
    MyProcess.ParsedArgs[count] = parsePipes;
    parsePipes = strtok(NULL, "|");
    ++count;
  }

  int fd[2];
  pipe(fd);
  pid_t pid;
  // The for loop will run as many one more than the pipeCounter value
  for (int i = 0; i <= MyProcess.pipeCounter; ++i){
      pid = fork();
      if (pid == 0){ // Child
        int sd = 0;
        char* outRedir = strchr(MyProcess.ParsedArgs[i], '>');
        char* inRedir = strchr(MyProcess.ParsedArgs[i], '<');
        strcpy(MyProcess.ParsedArgsCpy_, MyProcess.ParsedArgs[i]);

        // If the command is involving output redirection
        if (outRedir != NULL){
          MyProcess = outputRedir(MyProcess, sd);
        }
        // If the command is involving input redirection
        else if (inRedir != NULL){
          MyProcess = inputRedir(MyProcess, sd);
        }
        else {
              strcpy(MyProcess.ParsedArgsCpy, MyProcess.ParsedArgs[i]);
              MyProcess.ParsedArgsCpy[strlen(MyProcess.ParsedArgs[i])] = '\0';
              // SplitCmd are the parsed values of the command
              char* SplitCmd = strtok(MyProcess.ParsedArgsCpy, " ");
              while (SplitCmd != NULL){
                  /* All the parsed values of the input based on space
                  is stored in the Parsedcmd array */
                  MyProcess.Parsedcmd[MyProcess.index] = SplitCmd;
                  SplitCmd = strtok(NULL, " ");
                  ++MyProcess.index;
              }
              MyProcess.Parsedcmd[MyProcess.index] = NULL;
              ++MyProcess.index;
          }
          if (MyProcess.pipeCounter >= 1){
               if ((MyProcess.Parsedcmd[1]) == NULL && (MyProcess.Parsedcmd[0]) == NULL){
                   fprintf(stderr, "Error: missing command\n");
                   MyProcess.Enter = 1;

               }
              if (i == 0){
                  close(fd[0]); // close read access
                  dup2(fd[1], STDOUT_FILENO); // Replace stdout with pipe
                  close(fd[1]); // close write access
              }
              else if (i == 1){
                  if (MyProcess.pipeCounter == 1){
                      close(fd[1]); // close write access
                      dup2(fd[0], STDIN_FILENO); // Replace stdin with pipe
                      close(fd[0]); // close read access
                  }
                  else {
                      dup2(fd[0], STDIN_FILENO); // Replace stdin with pipe
                      close(fd[0]); // close read access
                      dup2(fd[1], STDOUT_FILENO); // Replace stdout with pipe
                      close(fd[1]); // close write access
                  }
              }
              else if (i == 2){
                  close(fd[1]); // close write access
                  dup2(fd[0], STDIN_FILENO); // Replace stdin with pipe
                  close(fd[0]); // close read access
              }
          }
          if (MyProcess.Enter == 0){
            execvp(MyProcess.Parsedcmd[0], MyProcess.Parsedcmd);
            fprintf(stderr, "Error: command not found\n");
          }

          exit(1);
      }
      else if (pid > 0){ // Parent
          pids[i] = pid;
      }

  }
  close(fd[0]);
  close(fd[1]);
  // Bunch of for loops, all cleaning the arrays
  for (int i = 0; i < MyProcess.index; ++i){
      MyProcess.Parsedcmd[i] = "\0";
  }
  for (int i = 0; i <= MyProcess.pipeCounter; ++i){
      MyProcess.ParsedArgs[i] = "\0";
  }
  for (int i = 0; i <= MyProcess.pipeCounter; ++i){
      MyProcess.ParsedArgsCpy[i] = '\0';
  }
  for (int i = 0; i <= MyProcess.pipeCounter; ++i){
      MyProcess.ParsedArgsCpy_[i] = '\0';
  }
  for (int i =0; i <= MyProcess.pipeCounter; ++i){
      MyProcess.FileName[i] = '\0';
  }

  // A for loop for waiting, as we didn't do that in the fork() for loop
  for (int i = 0; i <= MyProcess.pipeCounter; ++i){
      int status;
      waitpid(pids[i], &status, 0);
     // printf("status: %d\n", status);
      MyProcess.status[i] = status;

  }
  fprintf(stderr,"+ completed '%s' ", cmd);
  for (int g = 0; g<=MyProcess.pipeCounter; g++){
   fprintf(stderr,"[%d]", MyProcess.status[g]);
  }
  fprintf(stderr,"\n");

}



int main(void){
  char cmd[CMDLINE_MAX];
  struct process MyProcess;
  MyProcess.position = 0;

  // This loop will only be exited if the user types the command exit
  while(1){
    MyProcess.Enter = 0;
    // If checkIn is true, the command can be parsed
    bool checkIn = true;
    MyProcess.pipeCounter = 0;
    char *nl;

    /* Print prompt */
    printf("sshell$ ");
    fflush(stdout);
    /* Get command line */
    fgets(cmd, CMDLINE_MAX, stdin);
    /* Print command line if stdin is not provided by terminal */
    if (!isatty(STDIN_FILENO)) {
            printf("%s", cmd);
            fflush(stdout);
    }
    /* Remove trailing newline from command line */
    nl = strchr(cmd, '\n');
    if (nl)
            *nl = '\0';
    /* Builtin command */
    if (!strcmp(cmd, "exit")) {
          fprintf(stderr, "Bye...\n");
          fprintf(stderr,"+ completed 'exit' [0]\n");

            exit(0);
            break;
    }
    /* The cmdLenCheck checks if the input typed by the user has more
    than 16 commands */
    char cmdLenCheck[CMDLINE_MAX];
    // The number of commands in the input
    int cmdLen = 0;
    cmdLenCheck[0] = '\0';
    strcpy(cmdLenCheck, cmd);
    cmdLenCheck[strlen(cmd)] = '\0';

    // cmdPart is a part of the cmd, broken if space is found
    char* cmdPart = strtok(cmdLenCheck, " ");

    while (cmdPart != NULL){
        cmdLen++;
        cmdPart = strtok(NULL, " ");
    }

    /* cmdLen has the total arguments in cmd now, if it is more
    than 16, an error message should be printed */
    if (cmdLen > 16){
        fprintf(stderr, "Error: too many process arguments\n");
        checkIn = false;
    }

    if (checkIn == true){
      MyProcess.newCmd[0] = '\0';
      strcpy(MyProcess.newCmd, cmd);

      /*Basically if mainArg = cd/pwd/push/popd/dirs, we have to
      manually implement it, else we have to just fork it*/
      char *mainArg = strtok(MyProcess.newCmd, " ");
      // If the user inputted cd
      if (strcmp(mainArg, "cd") == 0){
        cd(mainArg, MyProcess, cmd);
      }
      // If the user inputted pwd
      else if (strcmp(mainArg, "pwd") == 0){
        printf("%s\n",getcwd(MyProcess.directoryName, CMDLINE_MAX));
      }
      // If the user inputted pushd
      else if (strcmp(mainArg, "pushd") == 0){
        MyProcess = pushd(mainArg, MyProcess, cmd);
      }
      // If the user inputted popd
      else if (strcmp(mainArg, "popd") == 0){
        MyProcess = popd(MyProcess, cmd);
      }
      // If the user inputted dirs
      else if (strcmp(mainArg, "dirs") == 0){
        MyProcess = dirs(MyProcess, cmd);
      }

      // If the command is not in all of the previous ones
      else{
          strcpy(MyProcess.cmdCopy, cmd);
          strcpy(MyProcess.newCmd, cmd);
          MyProcess = pipeFinder(MyProcess, cmd);
          Parsing(MyProcess, cmd);
      }
    }
  }
  return EXIT_SUCCESS;
}
