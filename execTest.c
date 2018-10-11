#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv){
  int childPID;
  switch(childPID = fork()){
    case -1:
      printf("Error\n");
    case 0: //If fork successfully creates child process
      execvp("ls", argv); //execvp - "v" accepts list as argument & "p" searches path for executable
      printf("Syserr\n");
    default: //Where parent process goes
      waitpid(childPID, NULL, WUNTRACED); //Waits for child process to finish to continue
  }
return 0;
}
