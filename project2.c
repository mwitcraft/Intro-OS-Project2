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

char initialMimic[MAX_BUFFER];
char initialSource[MAX_BUFFER];
int isRecursive = 0;
int inFTW = 0;
int isMimicIntoNewDir = 0;
int isMorph = 0;
int initialDirExists = 1;

int isDirectory(char* path);
int isDirectoryEmpty(char* path);

// Clears terminal "system clear"
int wipe(){
	// system("clear");

	int childPID;
	switch (childPID = fork()) {
		case -1:
			printf("Error\n");
		case 0:
			execlp("clear", "clear", NULL);
			printf("Syserr\n");
			return 0;
		default:
			waitpid(childPID, NULL, WUNTRACED);
	}
	kill(childPID, SIGTERM);
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
			return 0;
		default:
			waitpid(childPID, NULL, WUNTRACED);
	}
	kill(childPID, SIGTERM);
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

int recursiveMimicMorph(const char *fpath, const struct stat *sb, int tflag,struct FTW *ftwbuf){

	// Making arrays to hold the paths of the source and destination
	// The final dest path is the location initally given to mimic to plus the
	// entire path of the source
	char finalDestPath[MAX_FILENAME];
	char containingFolder[strlen(initialMimic)];
	char dirnameInitialMimic[strlen(initialMimic)];
	finalDestPath[0] = '\0';
	containingFolder[0] = '\0';
	dirnameInitialMimic[0] = '\0';
	strcat(dirnameInitialMimic, initialMimic);
	// printf("before dirname Initial mimic: %s\n", initialMimic);
	dirname(dirnameInitialMimic);
	// printf("after dirname Initial mimic: %s\n", initialMimic);
	//Get the base folder from the source
	char initialSourceCopy[strlen(fpath)];
	initialSourceCopy[0] = '\0';
	strcat(initialSourceCopy, initialSource);
	char* sourceBaseFolder;
	sourceBaseFolder = basename(initialSourceCopy);
	// printf("sourceBaseFolder: %s\n", sourceBaseFolder);
	// return 0;

	char* sourceBasePlus;
	sourceBasePlus = strstr(fpath, sourceBaseFolder);
	// printf("sourceBasePlus: %s\n", sourceBasePlus);
	// strcat(sourceBasePlus, strstr(sourceBaseFolder, fpath));
	// return 0;

	//If the source path is a directory
	if(isDirectory(initialMimic)){
		if(initialDirExists){
			printf("InitialDir Exists\n");
			// finalDestPath = initialMimic + sourceBaseFolder + fpath after sourceBaseFolder
			strcat(finalDestPath, initialMimic);
			strcat(finalDestPath, "/");
			strcat(finalDestPath, sourceBasePlus);
			printf("finalDestPath: %s\n", finalDestPath);
		}
		else if (!initialDirExists) {
			printf("InitialDir Does Not Exist\n");
			printf("sourceBase: %s\n", sourceBaseFolder);
			// finalDestPath = initialMimic + fpath after sourceBaseFolder
			strcat(finalDestPath, initialMimic);
			// strcat(finalDestPath, "/");
			char fPathAfterSourceBase[strlen(fpath) - strlen(initialSource)];
			for(int i = strlen(initialSource); i < strlen(fpath); ++i){
					fPathAfterSourceBase[i - strlen(initialSource)] = fpath[i];
			}
			fPathAfterSourceBase[strlen(fpath) - strlen(initialSource)] = '\0';
			strcat(finalDestPath, fPathAfterSourceBase);
			printf("finalDestPath: %s\n", finalDestPath);
		}

	}
	else if(!isDirectory(initialMimic) || !strcmp(initialMimic, dirnameInitialMimic)){
		initialDirExists = 0;
		if(isDirectory(dirnameInitialMimic) || !strcmp(initialMimic, dirnameInitialMimic)){
			mkdirz(initialMimic, 0);
			return 0;
		}
		else{
			printf("ERROR: parent is not a directory\n");
		}
	}

	//If source(fpath) is a file, copy the files to the destination(finalDestPath)
	if(tflag == FTW_F){ //fpath points to a file
		printf("\tfpath is a file\n");
		if(copyFile(fpath, finalDestPath) == -1){
				printf("ERROR copying file\n");
				return -1;
		}
	}

	//If source(fpath) is a directory, create the directory as finalDestPath
	else if(tflag == FTW_D){ //fpath points to a directory
		printf("\tfpath is a directory\n");
		// Gets the stat of the source path so we can apply the same permissions to the final destination
		struct stat sourceStat;
		stat(fpath, &sourceStat);
		if(mkdirz(finalDestPath, sourceStat.st_mode) == -1){ //Creates a new directory with same permissions as fpath
				printf("%s\n", strerror(errno));
				return -1;
		}
	}

	return 0;
}

int eraseAfterMorph(const char *fpath, const struct stat *sb, int tflag,struct FTW *ftwbuf){
		// If fpath is file, erase that file
		if(tflag == FTW_F){
			erase(fpath);
		}
		// If fpath is a directory, remove that directory
		else{
			rmdirz(fpath);
		}
		return 0;
}



// Copies file from sourcePath to destPath
int mimic(char** inputs, int numberOfInputs){
	// Creates char arrays to hold the source and destination paths
	// Initializes with size of MAX_FILENAME because don't know size
	char sourcePath[MAX_FILENAME];
	sourcePath[0] = '\0'; // Terminating char placed in front so when strings are added to it, it already has a terminating char
	char destPath[MAX_FILENAME];
	destPath[0] = '\0';
	int isRecursive = 0; // To store whether or not user specified recursive mimic or morph
	for(int i = 1; i < numberOfInputs; ++i){
		if(!strcmp(inputs[i], "-r")){
			isRecursive = 1;
		}
		// First input not being '-r' is the source
		else if(sourcePath[0] == '\0'){
			strncpy(sourcePath, inputs[i], strlen(inputs[i]));
			sourcePath[strlen(inputs[i])] = '\0';
			initialSource[0] = '\0';
			strcat(initialSource, sourcePath);
		}
		// Second input not being '-r' is the destination
		else{
			strncpy(destPath, inputs[i], strlen(inputs[i]));
			destPath[strlen(inputs[i])] = '\0';
			initialMimic[0] = '\0';
			strcat(initialMimic, destPath);
		}
	}

	// If the source is a directory:
	if(isDirectory(sourcePath)){
		// Copy/move directory recursively if recursive flag is set or is directory is empty
		if(isRecursive || isDirectoryEmpty(sourcePath)){
			// Calls the recursiveMimicMorph function on every file/directory in sourcePath
			if(nftw(sourcePath, recursiveMimicMorph, 20, 0) == 0){
				// Resets all of the global variables necessary for mimic/morph to work
				initialMimic[0] = '\0';
				isMimicIntoNewDir = 0;
				initialDirExists = 1;
			}
		}
		// Try to copy/move non-empty directory without recursive flag
		// Throw error
		else{
			printf("ERROR: Trying to copy non-empty directory without '-r'\n");
			return -1;
		}

		// If command is morph, remove all files contained in sourcepath
		if(!strcmp(inputs[0], "morph")){
			// Calls eraseAfterMorph function on every file in sourcePath
			// FTW_DEPTH flag set to list all of the files inside a directory before the directory
			// Necessary because you cannot remove a non-empty directory
			nftw(sourcePath, eraseAfterMorph, 20, FTW_DEPTH);
		}
	}

	else{
		// Create a copy of sourcePath to feed into basename
		char sourceBaseName[strlen(sourcePath)];
		strncpy(sourceBaseName, sourcePath, strlen(sourcePath));
		sourceBaseName[strlen(sourcePath)] = '\0';
		// Must store result of basename in pointer
		char* sourceBaseNamePointer = basename(sourceBaseName);
		// Copy sourceBaseNamePointer into sourceBaseName
		strncpy(sourceBaseName, sourceBaseNamePointer, strlen(sourceBaseNamePointer));
		sourceBaseName[strlen(sourceBaseNamePointer)] = '\0'; // Must terminate string after strncpy

		//Creates and initializes what will be the final destination path
		char finalDestPath[strlen(destPath) + strlen(initialSource)];
		finalDestPath[0] = '\0'; //Initialize with terminating character
		//If the destPath is a directory, the finalDestPath will be destPath + / + sourceBaseName
		if(isDirectory(destPath)){
			strcat(finalDestPath, destPath);
			strcat(finalDestPath, "/");
			strcat(finalDestPath, sourceBaseName);
		}
		//If destPath is not a directory, it must be pointing to a file, so finalDestPath = destPath
		else{
			strcat(finalDestPath, destPath);
		}

		//Attempt to copy the file
		if(copyFile(sourcePath, finalDestPath) == -1)
				return -1;

		//If the command was morph, delete the source file
		if(!strcmp(inputs[0], "morph")){
			erase(sourcePath);
		}
	}


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

int mkdirz(char* path, mode_t mode){
	if(mode == 0){
		if(mkdir(path, 0777) == -1){
			fprintf(stderr, "MKDIRZ ERROR: %s\n", strerror(errno));
			return -1;
		}
		else
			return 0;
	}
	else
		if(mkdir(path, mode) == -1){
			fprintf(stderr, "MKDIRZ ERROR: %s\n", strerror(errno));
			return -1;
		}
		else
			return 0;
}

int rmdirz(char* path){
	if(rmdir(path) == -1){
		fprintf(stderr, "RMDIRZ ERROR: %s\n", strerror(errno));
		return -1;
	}
	else
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

int copyFile(char* sourcePath, char* destPath){
  // Sets flags and permissions for the files for future use
  unsigned int sourceFlags = O_RDONLY; //Opens source as read only
  unsigned int destFlags = O_CREAT | O_WRONLY | O_TRUNC; // Opens destination as write only(WRONLY), and may create the file if it does not exist(CREAT)
  unsigned int destPermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw-

	// Opens the paths given as file descriptors
  int sourceFileDescriptor = open(sourcePath, sourceFlags);
  int destFileDescriptor = open(destPath, destFlags, destPermissions);
	// If there was an error opening the files, throw error
	if(sourceFileDescriptor == -1){
		printf("error opening source\n");
		return -1;
	}
	if(destFileDescriptor == -1){
		printf("error opening destination\n");
		return -1;
	}


	// Attempts to copy file using the destpath given
  ssize_t num_read;
  char buf[MAX_BUFFER];
  while((num_read = read(sourceFileDescriptor, buf, MAX_BUFFER)) > 0){
    if(write(destFileDescriptor, buf, num_read) != num_read){
      printf("ERROR during writing\n");
			return -1;
    }
  }
	return num_read;
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
		printf("%s%s", getenv("PWD"), prompt);
		// fputs(prompt, stdout);

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
				// char* sorce = args[1];
				// char* dest = args[2];
				// source[strlen(source)] = '\0';
				// dest[strlen(dest)] = '\0';
				mimic(args, argNum);
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
				mimic(args, argNum);

				// if(argNum == 3)
				// 	morph(args[1], args[2]);
				// else if(argNum < 3)
				// 	fprintf(stderr, "ERROR: Too few arguments\n");
				// else if(argNum > 3)
				// 	fprintf(stderr, "ERROR: Too many arguments\n");
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

			else if(!strcmp(args[0], "mkdirz")){
				if(argNum < 2)
					fprintf(stderr, "ERROR: Too few arguments\n");
				else{
					for(int i = 1; i < argNum; ++i)
						mkdirz(args[i], 0);
				}
			}

			else if(!strcmp(args[0], "rmdirz")){
				if(argNum < 2)
					fprintf(stderr, "ERROR: Too few arguments\n");
				else{
					for(int i = 1; i < argNum; ++i)
						rmdirz(args[i]);
				}

			}

			// Command not defined here, so run command using system calls
			else{
				// Extracts first argument as the command to run
				char command[MAX_BUFFER];
				command[0] = '\0';
				strcat(command, args[0]);

				//Adds additional provided args to arguments list
				char* arguments[MAX_BUFFER];
				arguments[0] = command; // 1st command of args list must be the command to run
				for(int i = 1; i < argNum; ++i){
					arguments[i] = args[i];
				}
				arguments[argNum] = NULL; // Last entry in list must be NULL

				// Fork a process to run the command with the provided arguments
				int childPID = 0;
				switch (childPID = fork()) {
					case -1:
						printf("Error\n");
					case 0:
						execvp(command, arguments);
						printf("Syserr\n");
						return 0;
					default:
						waitpid(childPID, NULL, WUNTRACED);
				}
				kill(childPID, SIGTERM);


				// char* command = args[0];
				// char* space = " ";
				//
				// for(int i = 1; i < argNum; ++i){
				// 	size_t memToAllocate = strlen(command) + strlen(args[i]) + strlen(space);
				// 	char* arrayOfCorrectSize = (char*)(malloc)(memToAllocate * sizeof(char));
				// 	command = strcat(arrayOfCorrectSize, command);
				// 	command = strcat(command, space);
				// 	command = strcat(command, args[i]);
				// }
				//
				// system(command);
			}

		}

	}

	// Newline printed for formatting
	return 0;
}
