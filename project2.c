#define _XOPEN_SOURCE 500
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ftw.h>
#include <stdint.h>

#define MAX_BUFFER 1024
#define MAX_FILENAME 257
#define MAX_ARGS 64
#define SEPARATORS " \t\n"

extern char** environ;

char* destinationForMimic;
char* initialMimic;
int isRecursive = 0;
int inFTW = 0;

int isDirectory(char* path);
int isDirectoryEmpty(char* path);

// Clears terminal "system clear"
int wipe(){
	system("clear");
	return 0;
}

// Lists files and directories in target directory
// If no directory is specified then prints files and directories in current directory
int filez(int argNum, char** target){

	char* command[argNum + 1];
	for(int i = 0; i < argNum; ++i)
		command[i] = target[i];
	command[argNum] = "-1";
	command[argNum + 1] = NULL;

	int childPID;
	switch (childPID = fork()) {
		case -1:
			printf("Error\n");
		case 0:
			execvp("ls", command);
			printf("Syserr\n");
		default:
			waitpid(childPID, NULL, WUNTRACED);
	}
	return 0;

	// char* command;
	//
	// if(target == NULL)
	// 	command = "ls -1";
	// else{
	// 	command = "ls -1 ";
	// 	size_t spaceNeeded = strlen(command) + strlen(target) + 1;
	// 	char* spaceNeededStr = (char*)(malloc)(spaceNeeded * sizeof(char));
	// 	command = strcat(spaceNeededStr, command);
	// 	command = strcat(command, target);
	// 	command[spaceNeeded] = '\0';
	// }
	//
	// printf("\t%s\n", command);
	//
	// return system(command);
}

// Prints program's README
int help(char* projectPath){

	// Navigates to program's home directory to open README.txt
	char* readmeFileName = "/README.txt";
	size_t readmePathSize = strlen(projectPath) + strlen(readmeFileName);
	char* readmePath = (char*)(malloc)(readmePathSize * sizeof(char));
	readmePath = strcat(readmePath, projectPath);
	readmePath = strcat(readmePath, readmeFileName);

	FILE* readme = fopen(readmePath, "r");
	if(readme == NULL){
		fprintf(stderr, "ERROR: %s; cannot find README\n", strerror(errno));
		return -1;
	}

	// Steps through and prints contents of README to stdout
	char c = fgetc(readme);
	while(c != EOF){
		printf("%c", c);
		c = fgetc(readme);
	}
	return 0;
}

int display_info(const char *fpath, const struct stat *sb, int tflag,struct FTW *ftwbuf){
	// printf("&&&&\t%s", fpath);
	printf("Path inside nftw function: %s\n", fpath);
	printf("Destination for mimic: %s\n", destinationForMimic);
	isRecursive = 0;
	inFTW = 1;
	mimic(fpath, destinationForMimic, 0);

}

// Copies file from sourcePath to destPath
int mimic(char* sourcePath, char* destPath, int firstPass){
	if(firstPass)
		initialMimic = destPath;
	printf("Initial Mimic: %s\n", initialMimic);

	// printf("Destination for mimic inside mimic: %s\n", destinationForMimic);

  // Sets flags and permissions for the files for future use
  unsigned int sourceFlags = O_RDONLY;
  unsigned int destFlags = O_CREAT | O_WRONLY | O_TRUNC;
  unsigned int destPermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw-

  // Opens source
  int sourceFileDescriptor = open(sourcePath, sourceFlags);

  // Determines if source and path are directories
  int isSourceDirectory = isDirectory(sourcePath);
  int isDestDirectory = isDirectory(destPath);

  if(isDestDirectory){
		// printf("Dest Path inside if loop: %s", destPath);


    // Below appears to be working for now
    char* slash = "/";
		char sourcePathAsArray[10];
		strncpy(sourcePathAsArray, sourcePath, strlen(sourcePath));
		sourcePathAsArray[strlen(sourcePath)] = '\0';
		// printf("Source path as array: %s\n", sourcePathAsArray);
		char* sourceBaseName = basename(sourcePathAsArray);
		// printf("Source base name: %s\n", sourceBaseName);
		size_t destPathWithFileNameSize = strlen(destPath) + strlen(slash) + strlen(sourceBaseName);
		// destPathWithFileName = strcat(destPathWithFileName, destPath);

		char destPathWithFileName[MAX_FILENAME];
		destPathWithFileName[0] = '\0';
		strcat(destPathWithFileName, destPath);
		strcat(destPathWithFileName, slash);
		strcat(destPathWithFileName, sourceBaseName);
		destPath = (char*)(malloc)(destPathWithFileNameSize * sizeof(char));
		strcpy(destPath, destPathWithFileName);
		// destPath = destPathWithFileName;
		// printf("Destpath with filename: %s\n", destPathWithFileName);

		if(inFTW){
			//dest path is just initial mimic + source path
			destPathWithFileName[0] = '\0';
			strcat(destPathWithFileName, initialMimic);
			strcat(destPathWithFileName, slash);
			strcat(destPathWithFileName, sourcePath);
			destPathWithFileNameSize = strlen(destPath) + strlen(slash) + strlen(sourcePath);
			destPath = (char*)(malloc)(strlen(destPathWithFileName) * sizeof(char));
			strcpy(destPath, destPathWithFileName);
		}

		// char* destPathWithFileName = (char*)(malloc)(destPathWithFileNameSize * sizeof(char));
		// destPathWithFileName = strcat(destPathWithFileName, destPath);
		// destPathWithFileName = strcat(destPathWithFileName, slash);
		// destPathWithFileName = strcat(destPathWithFileName, sourceBaseName);
		// destPath = destPathWithFileName;
		// printf("Destpath with filename: %s\n", destPathWithFileName);

		// printf("Dest path inside making full dest name: %s\n", destPath);

		// printf("%i\n", isDirectory(destPath));

    // Below is given in project specs
    // printf("Dest is directory\n");
    // char destName[MAX_FILENAME/2];
    // char destPath[MAX_FILENAME/2];
    // char destDirectory[MAX_FILENAME/2];
    //
    // strcpy(destName, basename(sourcePath)); //Gets the file name from source path
    // printf("Dest File Name: %s", destName);
    // strcpy(destDirectory, basename(destPath)); //Gets the last folder name from destination path
    // strcpy(destPath, dirname(destPath)); //Gets the path from destination path
    //
    // // Creates fullDestination path using elements of path
    // fullDestination[0] = '\0'; //Zeroes out string
    // strcat(fullDestination, destPath);
    // strcat(fullDestination, "/");
    // strcat(fullDestination, destDirectory);
    // strcat(fullDestination, "/");
    // strcat(fullDestination, destName);
    //
    // printf("%s\n", fullDestination);
  }

	destinationForMimic = destPath;

	// Copies if source path points to directory and that directory is empty
	if(isSourceDirectory && (isDirectoryEmpty(sourcePath) || inFTW)){
		// printf("169\n");
		struct stat sourceStat;
		stat(sourcePath, &sourceStat);
		if(isDestDirectory){
			// printf("173\n");
			if(mkdir(destPath, sourceStat.st_mode) == -1){
				if(errno != EEXIST){
					printf("%s\n", strerror(errno));
				}
			}
			else
				printf("Directory '%s' created\n", destPath);
		}
		else{
			// printf("181\n");
			char destPathAsArray[10];
			strncpy(destPathAsArray, destPath, strlen(destPath));
			destPathAsArray[strlen(destPath)] = '\0';

			if(isDirectory(dirname(destPathAsArray))){
			// printf("187\n");
				// printf("%s\n", );
					if(mkdir(destPath, sourceStat.st_mode) == -1){
						if(errno != EEXIST){
							printf("%s\n", strerror(errno));
						}
					}
			}
			else{
				printf("Handle Later\n");

			}
		}
		return 0;
	}
	else if(isSourceDirectory && isRecursive == 1){

		int flags = 0;
		if(nftw(sourcePath, display_info, 20, flags))
		// Just for now
		printf("Directory is not empty\n");
		return -1;
	}

	printf("\t%s", sourcePath);
	printf("\t%s\n", destPath);
	for(int i = 0; i < strlen(destPath); ++i){
		printf("\t\t%i\n", destPath[i]);
	}
	// printf("Running long\n");

  int destFileDescriptor = open(destPath, destFlags, destPermissions);
	if(destFileDescriptor == -1){
		printf("%s\n", strerror(errno));
		printf("Destination not valid\n");
		exit(0);
	}

  ssize_t num_read;
  char buf[MAX_BUFFER];
  while((num_read = read(sourceFileDescriptor, buf, MAX_BUFFER)) > 0){
    if(write(destFileDescriptor, buf, num_read) != num_read){
      printf("ERROR during writing\n");
    }
  }
  if(num_read == -1 && isSourceDirectory){
    while((num_read = read(sourceFileDescriptor, buf, MAX_BUFFER)) > 0){
			destFileDescriptor = open(dirname(destPath), destFlags, destPermissions);
			if(write(destFileDescriptor, buf, num_read) != num_read){
				printf("ERROR during writing");
			}
		}
  }
	else if(num_read == -1){
		printf("ERROR during writing");
	}

  if(close(sourceFileDescriptor) == -1){
    printf("ERROR closing source\n");
  }

  if(close(destFileDescriptor) == -1){
    printf("ERROR closing destination\n");
  }

  // Below is the mimic code for project1
  // ------------------------------------------------------
  // ------------------------------------------------------
	// // Determines if source file is a file or not
	// struct stat sourceBuf;
	// stat(sourcePath, &sourceBuf);
	// if(S_ISREG(sourceBuf.st_mode) == 0){
	// 	fprintf(stderr, "ERROR: Source is not file\n");
	// 	return -1;
	// }
  //
	// // Opens files as descriptors to feed into sendfile function
	// int sourceDescriptor = open(sourcePath, O_RDWR);
	// int destDescriptor = open(destPath, O_CREAT | O_RDWR, S_IRWXU);
  //
	// // Gets size of source file to provide to sendfile
	// struct stat toGetSize;
	// fstat(sourceDescriptor, &toGetSize);
	// int sourceSizeInBytes = toGetSize.st_size;
  //
	// // Copies file
	// // http://man7.org/linux/man-pages/man2/sendfile.2.html
	// if(sendfile(destDescriptor, sourceDescriptor, NULL, sourceSizeInBytes) == -1){
	// 	fprintf(stderr, "ERROR: Invalid mimic destination\n");
	// 	return -1;
	// }
  //
	destinationForMimic = initialMimic;
	return 0;
}

// Removes file pointed to by path
int erase(char* path){
	// http://man7.org/linux/man-pages/man3/remove.3.html
	if(remove(path) == -1){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

// Copies files from sourcePath to destPath
int morph(char* sourcePath, char* destPath){

	// Determines whether destPath points to directory or not
	struct stat destPathBuf;
	stat(destPath, &destPathBuf);
	int isDestDirectory = S_ISDIR(destPathBuf.st_mode);

	// If destPath points to directory, the filename of the file pointed to by sourcePath (found by basename function)
	// is appended to destPath, to preserve filename after move
	char* slash = "/";
	if(isDestDirectory != 0){
		// http://man7.org/linux/man-pages/man3/basename.3.html
		char* sourceBaseName = basename(sourcePath);
		size_t destPathWithFileNameSize = strlen(destPath) + strlen(slash) + strlen(sourceBaseName);
		char* destPathWithFileName = (char*)(malloc)(destPathWithFileNameSize * sizeof(char));
		destPathWithFileName = strcat(destPathWithFileName, destPath);
		destPathWithFileName = strcat(destPathWithFileName, slash);
		destPathWithFileName = strcat(destPathWithFileName, sourceBaseName);
		destPath = destPathWithFileName;

	}

	// Rename function moves the file at sourcePath to destPath by renaming it starting at the root directory
	// http://man7.org/linux/man-pages/man2/rename.2.html
	if(rename(sourcePath, destPath) == -1){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

// Changes the current working dirctory if an input is provided and updates the PWD environment variable
// If no input is provided, prints current working directory to stdout
int mychdir(char* input){

	// http://man7.org/linux/man-pages/man2/chdir.2.html
	if(chdir(input) == -1){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		return -1;
	}

	char cwd[256];
	getcwd(cwd, sizeof(cwd));
	char* updateEnv = "PWD=";
	size_t updateEnvSize = strlen(updateEnv) + strlen(cwd);
	char* updateEnvSizeMem = (char*)(malloc)(updateEnvSize * sizeof(char));
	updateEnv = strcat(updateEnvSizeMem, updateEnv);
	updateEnv = strcat(updateEnv, cwd);
	putenv(updateEnv);

	return 0;
}

// Checks if path is a directory
// Taken from https://oudalab.github.io/cs3113fa18/projects/morphmimic
int isDirectory(char* path){
  struct stat statbuf;
  if(stat(path, &statbuf) != 0)
    return 0;
  return S_ISDIR(statbuf.st_mode);
}

int isDirectoryEmpty(char *dirname) {
	int n = 0;
  struct dirent *d;
  DIR *dir = opendir(dirname);
  if (dir == NULL) //Not a directory or doesn't exist
    return 1;
  while ((d = readdir(dir)) != NULL) {
    ++n;
    if(n > 2)
      break;
  }
  closedir(dir);
  if (n <= 2) //Directory Empty
    return 1;
  else
    return 0;
}

// Main function where program begins
int main(int argc, char** argv){

	char buf[MAX_BUFFER];
	char* args[MAX_ARGS] ;
	char** arg;
	char* prompt = "==>";
	int argNum;

	// Gets program home on startup in order to be able to open README on 'help'
	char* pathPnt = getenv("PWD");
	char path[strlen(pathPnt) + 1];
	for(int i = 0; i < (int)strlen(pathPnt); ++i){
		path[i] = pathPnt[i];
	}
	path[strlen(pathPnt)] = '\0';

	// Opens provided batch file and stores it in 'batchFile'
	FILE* batchFile = NULL;
	int batchProvided = 0;
	// If batch file is provided as an argument
	if(argc == 2){
		batchFile = fopen(argv[1], "r");
		stdin = batchFile;
		if(batchFile == NULL)
			fprintf(stderr, "ERROR: %s", strerror(errno));
		else
			batchProvided = 1;
	}
	// If batch file is piped in
	else if(argc == 1){
		batchFile = stdin;
	}
	else
		fprintf(stderr, "ERROR: Unexpected arguments");

	// Loops until end of stdin
	while(!feof(stdin)){

		/* Prints prompt out */
		fputs(prompt, stdout);

		/* Waits for input after prompt and stores the input with max size of MAX_BUFFER in buf */
		if(fgets(buf, MAX_BUFFER, stdin)){

			char* fullInput = buf;

			// Keeps track of number of arguments
			argNum = 0;

			// Tokenizes input based on SEPARATORS
			char* tkn = strtok(fullInput, SEPARATORS);
			for(int i = 0; tkn != NULL; ++i){
				++argNum;
				if(!strcmp(tkn, "-r")){
					isRecursive = 1;
				}
				args[i] = tkn;
				tkn = strtok(NULL, SEPARATORS);
			}

			// Prints next to prompt the input from provided batch file
			// Currently buggy, as when batch file is not provided, it prints out on a new line the
			// command entered
			if(batchFile != NULL){
				for(int i = 0; i < argNum; ++i){
						printf("%s ", args[i]);
				}
				printf("\n");
			}

			// ------------------------------------------------------------------------------------------
			// Below compares command(first token) to predefined keywords
			// if command matches, then it runs that funtion with provided arguments(following tokens)
			// If command does not	match any kewords, then all tokens are passed to system function,
			// allowing system to handle the command
			// ------------------------------------------------------------------------------------------

			/* System exit */
			if(!strcmp(args[0], "esc")){
				if(argNum > 1){
					fprintf(stderr, "ERROR: Too many arguments\n");
				}
				else{
					break;
				}
			}

			/* System clear */
			else if(!strcmp(args[0], "wipe")){
				if(argNum > 1)
					fprintf(stderr, "ERROR: Too many arguments\n");
				else
					wipe();
			}

			// Prints files according to 'filez' function
			else if(!strcmp(args[0], "filez")){
				args[0] = argv[0];
				filez(argNum, args);
				// if(argNum > 2)
				// 	fprintf(stderr, "ERROR: Too many arguments\n");
				// else{
				// 	if(argNum == 1)
				// 		filez(NULL);
				// 	if(argNum ==2)
				// 		filez(args[1]);
				// }
			}

			 // Prints environment variables to stdout
			else if(!strcmp(args[0], "environ")){
				if(argNum > 1){
					fprintf(stderr, "ERROR: Too many arguments\n");
				}
				else{
					char** env = environ;
					while(*env){
						printf("%s\n", *env);
						*env++;
					}

				}
			}

			// Prints provided arguments to stdout
			else if(!strcmp(args[0], "ditto")){
				if(argNum != 1){
					for(int i = 1; i < argNum; ++i){
						printf("%s ", args[i]);
					}
					printf("\n");
				}
			}

			/* Prints README */
			else if(!strcmp(args[0], "help")){
				if(argNum > 1)
					fprintf(stderr, "ERROR: Too many arguments\n");
				else
					help(path);
			}

			// Copies file pointed to by 1st argument to 2nd argument
			else if(!strcmp(args[0], "mimic")){
				char* source = args[1];
				char* dest = args[2];
				source[strlen(source)] = '\0';
				dest[strlen(dest)] = '\0';
				mimic(source, dest, 1);
				// if(argNum == 3)
				// 	mimic(args[1], args[2]);
				// else if(argNum < 3)
				// 	fprintf(stderr, "ERROR: Too few arguments\n");
				// else if(argNum > 3)
				// 	fprintf(stderr, "ERROR: Too many arguments\n");
			}

			// Deletes files pointed to by ith argument
			else if(!strcmp(args[0], "erase")){
				if(argNum < 2)
					fprintf(stderr, "ERROR: Too few arguments\n");
				else
					for(int i = 1; i < argNum; ++i)
						erase(args[i]);

			}

			// Moves file pointed to by 1st argument to location pointed to by 2nd argument
			else if(!strcmp(args[0], "morph")){
				if(argNum == 3)
					morph(args[1], args[2]);
				else if(argNum < 3)
					fprintf(stderr, "ERROR: Too few arguments\n");
				else if(argNum > 3)
					fprintf(stderr, "ERROR: Too many arguments\n");
			}

			// Changes current working directory if argument is provided
			// if not, prints current working directory
			else if(!strcmp(args[0], "chdir")){
				if(argNum == 1)
					printf("%s\n", getenv("PWD"));
				else if(argNum > 2)
					fprintf(stderr, "ERROR: Too many arguments\n");
				else
					mychdir(args[1]);
			}

			// Command not defined here, so pass to system
			else{

				char* command = args[0];
				char* space = " ";

				for(int i = 1; i < argNum; ++i){
					size_t memToAllocate = strlen(command) + strlen(args[i]) + strlen(space);
					char* arrayOfCorrectSize = (char*)(malloc)(memToAllocate * sizeof(char));
					command = strcat(arrayOfCorrectSize, command);
					command = strcat(command, space);
					command = strcat(command, args[i]);
				}

				system(command);
			}

		}

	}

	// Newline printed for formatting
	return 0;
}
