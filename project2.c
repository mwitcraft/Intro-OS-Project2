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

char initialDestination[MAX_BUFFER];
char initialSource[MAX_BUFFER];
int isMimicIntoNewDir = 0;
int initialDirExists = 1;

int isDirectory(char* path);
int isDirectoryEmpty(char* path);
int wipe();
int filez(int argNum, char** args);
int help(char* projectPath);
int recursiveMimicMorph(const char *fpath, const struct stat *sb, int tflag,struct FTW *ftwbuf);
int eraseAfterMorph(const char *fpath, const struct stat *sb, int tflag,struct FTW *ftwbuf);
int mimic(char** inputs, int numberOfInputs);
int erase(const char* path);
int mychdir(char* input);
int mkdirz(char* path, mode_t mode);
int rmdirz(const char* path);
int isDirectory(char* path);
int isDirectoryEmpty(char *dirname);
int copyFile(const char* sourcePath, char* destPath);

// Clears terminal like "system clear"
int wipe(){

	int childPID;
	switch (childPID = fork()) {
		case -1:
			printf("Error\n");
		case 0:
			execlp("clear", "clear", NULL); //Calls clear
			printf("Syserr\n");
			return 0; //Signals end of child process
		default:
			waitpid(childPID, NULL, WUNTRACED); //Parent process waits on child process to be completed
	}
	kill(childPID, SIGTERM);//Kills the child process on completion
	return 0;
}

// Lists files and directories in target directory
// If no directory is specified then prints files and directories in current directory
int filez(int argNum, char** args){

	//Variables initialized to store file names and modes for IO redirection
	char outFile[MAX_FILENAME];
	outFile[0] = '\0';
	char outMode[2];
	outMode[0] = '\0';
	int outFileProvided = 0;

	char inFile[MAX_FILENAME];
	inFile[0] = '\0';
	char inMode[2];
	inMode[0] = '\0';
	int inFileProvided = 0;

	//Adds additional provided args to arguments list
	char* arguments[MAX_BUFFER];
	arguments[0] = args[0]; // 1st command of args list must be the command to run
	int argumentsSize = 1;
	for(int i = 1; i < argNum; ++i){
		//Set stdin to args[i + 1]
		if(!strcmp(args[i], "<")){
			//Mode = "r" (O_RDONLY)
			strcat(inMode, "r");
			//File path to replace stdin is stored in inFile
			strcat(inFile, args[i + 1]);
			//To skip next file, because the file after '<' is the name of the file to replace stdout
			i = i + 2;
			//Set new stdin file flag
			inFileProvided = 1;
			//Needed so program does not break, IDK why
			if(i >= argNum)
				break;
		}
		//Set stdout to args[i + 1], created if it doesn't exist, either truncated or appended to depending on options
		if(!strcmp(args[i], ">") || !strcmp(args[i], ">>")){
			//File is truncated
			//Mode = "w" (O_WRONLY|O_CREAT|O_TRUNC)
			if(!strcmp(args[i], ">"))
				strcat(outMode,"w");
			//File is appended to
			//Mode = "a" (O_WRONLY|O_CREAT|O_APPEND)
			if(!strcmp(args[i], ">>"))
				strcat(outMode, "a");
			//File path to replace stdout is stored in outFile
			strcat(outFile, args[i + 1]);
			//To skip next file, because the file after '>' or '>>' is the name of the file to replace stdout
			i = i + 2;
			//Set new stdout file flag
			outFileProvided = 1;
			//Needed so program does not break, IDK why
			if(i >= argNum)
				break;
		}
		//Provided argument is not part of IO redirection, therefore continue like normal
		else{
			//Must specify argumentsSize because may skip some "i's" because of redirection
			arguments[argumentsSize] = args[i];
			++argumentsSize;
		}
	}
	arguments[argumentsSize] = "-1"; // Add '-1' because filez = 'ls -1'
	arguments[argumentsSize + 1] = NULL; //Final entry in pointer passed to exec must be NULL

	int childPID;
	switch (childPID = fork()) {
		case -1:
			printf("Error\n");
		case 0:
			//If stdout or stdin is being redirected, associate stream with that file
			if(outFileProvided){
				if(freopen(outFile, outMode, stdout) == NULL)
					fprintf(stderr, "freopen ERROR: %s\n", strerror(errno));
			}
			if(inFileProvided){
				if(freopen(inFile, inMode, stdin) == NULL)
					fprintf(stderr, "freopen ERROR: %s\n", strerror(errno));
			}
			execvp("ls", arguments);
			printf("Syserr\n");
			return 0;
		default:
			waitpid(childPID, NULL, WUNTRACED);
	}
	kill(childPID, SIGTERM);
	return 0;
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

// Function called by nftw after mimic or morph on a directory
int recursiveMimicMorph(const char *fpath, const struct stat *sb, int tflag,struct FTW *ftwbuf){

	// Making arrays to hold the paths of the source and destination
	// The final dest path is the location initally given to mimic to plus the
	// entire path of the source
	char finalDestPath[MAX_FILENAME];
	char containingFolder[strlen(initialDestination)];
	char dirnameinitialDestination[strlen(initialDestination)];
	finalDestPath[0] = '\0';
	containingFolder[0] = '\0';
	dirnameinitialDestination[0] = '\0';

	//Store the dirname of the destination in dirnameinitialDestination
	strcat(dirnameinitialDestination, initialDestination);
	dirname(dirnameinitialDestination);

	//Get the base folder from the source
	char initialSourceCopy[strlen(fpath)];
	initialSourceCopy[0] = '\0';
	strcat(initialSourceCopy, initialSource);
	//Store the basename of the initial source in sourceBaseFolder
	char* sourceBaseFolder;
	sourceBaseFolder = basename(initialSourceCopy);

	//Get everything after the basename of the initial source and store it in sourceBasePlus
	//This is necessary for nested folders
	char* sourceBasePlus;
	sourceBasePlus = strstr(fpath, sourceBaseFolder);

	//If the source path is a directory
	if(isDirectory(initialDestination)){
		//The final destination differs on whether or not the destination existed before the morph/mimic call
		if(initialDirExists){ // finalDestPath = initialDestination + sourceBaseFolder + fpath after sourceBaseFolder
			strcat(finalDestPath, initialDestination);
			strcat(finalDestPath, "/");
			strcat(finalDestPath, sourceBasePlus);
		}
		else if (!initialDirExists) { // finalDestPath = initialDestination + fpath after sourceBaseFolder
			strcat(finalDestPath, initialDestination);
			//Must remove sourceBaseFolder fpath to get correct destination
			char fPathAfterSourceBase[strlen(fpath) - strlen(initialSource)];
			for(int i = strlen(initialSource); i < strlen(fpath); ++i){
					fPathAfterSourceBase[i - strlen(initialSource)] = fpath[i];
			}
			fPathAfterSourceBase[strlen(fpath) - strlen(initialSource)] = '\0';
			strcat(finalDestPath, fPathAfterSourceBase);
		}

	}
	//If the destination directory does not exist or is the same as the dirname of the destination dirctory (implying that the destination directory is in the CWD)
	//Create the directory and set the initialDirExists flag to false if the parent directory exists
	else if(!isDirectory(initialDestination) || !strcmp(initialDestination, dirnameinitialDestination)){
		initialDirExists = 0;
		if(isDirectory(dirnameinitialDestination) || !strcmp(initialDestination, dirnameinitialDestination)){
			mkdirz(initialDestination, 0);
			return 0;
		}
		else{
			fprintf(stderr, "ERROR: parent is not a directory\n");
		}
	}

	//If source(fpath) is a file, copy the file to the destination(finalDestPath)
	if(tflag == FTW_F){ //fpath points to a file
		if(copyFile(fpath, finalDestPath) == -1){
				fprintf(stderr, "ERROR copying file\n");
				return -1;
		}
	}

	//If source(fpath) is a directory, create the directory as finalDestPath
	else if(tflag == FTW_D){ //fpath points to a directory
		// Gets the stat of the source path so we can apply the same permissions to the final destination
		struct stat sourceStat;
		stat(fpath, &sourceStat);
		if(mkdirz(finalDestPath, sourceStat.st_mode) == -1){ //Creates a new directory with same permissions as fpath
				fprintf(stderr, "%s\n", strerror(errno));
				return -1;
		}
	}

	return 0;
}

// Function called by nftw() to delete contents (after a morph call)
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
			initialDestination[0] = '\0';
			strcat(initialDestination, destPath);
		}
	}

	// If the source is a directory:
	if(isDirectory(sourcePath)){
		// Copy/move directory recursively if recursive flag is set or is directory is empty
		if(isRecursive || isDirectoryEmpty(sourcePath)){
			// Calls the recursiveMimicMorph function on every file/directory in sourcePath
			if(nftw(sourcePath, recursiveMimicMorph, 20, 0) == 0){
				// Resets all of the global variables necessary for mimic/morph to work
				initialDestination[0] = '\0';
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
int erase(const char* path){
	// http://man7.org/linux/man-pages/man3/remove.3.html
	if(remove(path) == -1){
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

//Creates a directory
int mkdirz(char* path, mode_t mode){
	//If creating brand new directory, initialize with permissions RWX
	if(mode == 0){
		if(mkdir(path, 0777) == -1){
			fprintf(stderr, "MKDIRZ ERROR: %s\n", strerror(errno));
			return -1;
		}
		else
			return 0;
	}
	//If creating a new directory as a copy of another one, use permissions of old
	else
		if(mkdir(path, mode) == -1){
			fprintf(stderr, "MKDIRZ ERROR: %s\n", strerror(errno));
			return -1;
		}
		else
			return 0;
}

//Removes and empty directory
int rmdirz(const char* path){
	// http://man7.org/linux/man-pages/man2/rmdir.2.html
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

//Determines whether a directory is empty or not
int isDirectoryEmpty(char *dirname) {
	int n = 0;
  struct dirent *d; //Directory object that stores directory properties
  DIR *dir = opendir(dirname);
  if (dir == NULL) //Not a directory or doesn't exist
    return 1;
  while ((d = readdir(dir)) != NULL) { //Reads info inside directory
    ++n;
		//If there is more than 2 entries in the directory, then it's not empty
		//2 because of '.'(current directory) and '..'(parent directory)
    if(n > 2)
      break;
  }
  closedir(dir); //Close the directory
  if (n <= 2) //Directory Empty
    return 1;
  else
    return 0;
}

//Copies file from sourcePath to destPath
int copyFile(const char* sourcePath, char* destPath){
  // Sets flags and permissions for the files for future use
  unsigned int sourceFlags = O_RDONLY; //Opens source as read only
  unsigned int destFlags = O_CREAT | O_WRONLY | O_TRUNC; // Opens destination as write only(WRONLY), and may create the file if it does not exist(CREAT)
  unsigned int destPermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw-

	// Opens the paths given as file descriptors
  int sourceFileDescriptor = open(sourcePath, sourceFlags);
  int destFileDescriptor = open(destPath, destFlags, destPermissions);
	// If there was an error opening the files, throw error
	if(sourceFileDescriptor == -1){
		fprintf(stderr, "error opening source: %s\n", sourcePath);
		return -1;
	}
	if(destFileDescriptor == -1){
		fprintf(stderr, "error opening destination: %s\n", destPath);
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
	char* prompt = "==>";
	int argNum;

	// Gets program home on startup in order to be able to open README on 'help'
	char* pathPnt = getenv("PWD");
	char path[strlen(pathPnt) + 1];
	for(int i = 0; i < (int)strlen(pathPnt); ++i){
		path[i] = pathPnt[i];
	}
	path[strlen(pathPnt)] = '\0';

	//If an argument is supplied on program startup, then that argument is
	//associated with stdin
	int batchProvided = 0; //Flag to let program know batchfile was provided
	if(argc == 2){
		if(freopen(argv[1], "r", stdin) != NULL)
			batchProvided = 1;
		else
			fprintf(stderr, "freopen() ERROR: %s\n", strerror(errno));
	}

	// Loops until end of stdin
	while(!feof(stdin)){

		//Prints out prompt, prepended with the current working directory
		printf("%s%s", getenv("PWD"), prompt);

		/* Waits for input after prompt and stores the input with max size of MAX_BUFFER in buf */
		if(fgets(buf, MAX_BUFFER, stdin)){

			char* fullInput = buf;

			// Keeps track of number of arguments
			argNum = 0;

			// Tokenizes input based on SEPARATORS
			char* tkn = strtok(fullInput, SEPARATORS);
			for(int i = 0; tkn != NULL; ++i){
				++argNum;
				args[i] = tkn;
				tkn = strtok(NULL, SEPARATORS);
			}

			//If a batchfile has been provided, prints out the commands after the prompt
			if(batchProvided){
				for(int i = 0; i < argNum; ++i)
					printf("%s ", args[i]);
				printf("\n");
				fflush(stdout);
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
				args[0] = argv[0]; //Gives name of program as first arguments to filez
				filez(argNum, args);
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
			// Basically echo
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

			// Copies file or destination pointed to by 1st argument to 2nd argument
			else if(!strcmp(args[0], "mimic")){
				mimic(args, argNum);
			}

			// Deletes files pointed to by ith argument
			else if(!strcmp(args[0], "erase")){
				if(argNum < 2)
					fprintf(stderr, "ERROR: Too few arguments\n");
				else
					for(int i = 1; i < argNum; ++i)
						erase(args[i]);

			}

			// Moves file or directory pointed to by 1st argument to location pointed to by 2nd argument
			else if(!strcmp(args[0], "morph")){
				mimic(args, argNum);
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

			//Creates new directory
			else if(!strcmp(args[0], "mkdirz")){
				if(argNum < 2)
					fprintf(stderr, "ERROR: Too few arguments\n");
				else{
					//Can create multiple directories with one user call to 'mkdirz'
					for(int i = 1; i < argNum; ++i)
						mkdirz(args[i], 0);
				}
			}

			//Deletes empty directories
			else if(!strcmp(args[0], "rmdirz")){
				if(argNum < 2)
					fprintf(stderr, "ERROR: Too few arguments\n");
				else{
					//Can erase multiple directories with one user call to 'rmdirz'
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

				//Variables initialized to store file names and modes for IO redirection
				char outFile[MAX_FILENAME];
				outFile[0] = '\0';
				char outMode[2];
				outMode[0] = '\0';
				int outFileProvided = 0;

				char inFile[MAX_FILENAME];
				inFile[0] = '\0';
				char inMode[2];
				inMode[0] = '\0';
				int inFileProvided = 0;

				//Adds additional provided args to arguments list
				char* arguments[MAX_BUFFER];
				arguments[0] = command; // 1st command of args list must be the command to run
				int argumentsSize = 1;
				for(int i = 1; i < argNum; ++i){
					//Set stdin to args[i + 1]
					if(!strcmp(args[i], "<")){
						//Mode = "r" (O_RDONLY)
						strcat(inMode, "r");
						//File path to replace stdin is stored in inFile
						strcat(inFile, args[i + 1]);
						//To skip next file, because the file after '<' is the name of the file to replace stdout
						i = i + 2;
						//Set new stdin file flag
						inFileProvided = 1;
						//Needed so program does not break, IDK why
						if(i >= argNum)
							break;
					}
					//Set stdout to args[i + 1], created if it doesn't exist, either truncated or appended to depending on options
					if(!strcmp(args[i], ">") || !strcmp(args[i], ">>")){
						//File is truncated
						//Mode = "w" (O_WRONLY|O_CREAT|O_TRUNC)
						if(!strcmp(args[i], ">"))
							strcat(outMode,"w");
						//File is appended to
						//Mode = "a" (O_WRONLY|O_CREAT|O_APPEND)
						if(!strcmp(args[i], ">>"))
							strcat(outMode, "a");
						//File path to replace stdout is stored in outFile
						strcat(outFile, args[i + 1]);
						//To skip next file, because the file after '>' or '>>' is the name of the file to replace stdout
						i = i + 2;
						//Set new stdout file flag
						outFileProvided = 1;
						//Needed so program does not break, IDK why
						if(i >= argNum)
							break;
					}
					//Provided argument is not part of IO redirection, therefore continue like normal
					else{
						//Must specify argumentsSize because may skip some "i's" because of redirection
						arguments[argumentsSize] = args[i];
						++argumentsSize;
					}
				}

				arguments[argumentsSize] = NULL; // Last entry in list must be NULL

				// Fork a process to run the command with the provided arguments
				int childPID = 0;
				switch (childPID = fork()) {
					case -1: //Error in fork
						printf("Error\n");
					case 0: //Fork success
						//If stdout or stdin is being redirected, associate stream with that file
						if(outFileProvided){
							if(freopen(outFile, outMode, stdout) == NULL)
								fprintf(stderr, "freopen ERROR: %s\n", strerror(errno));
						}
						if(inFileProvided){
							if(freopen(inFile, inMode, stdin) == NULL)
								fprintf(stderr, "freopen ERROR: %s\n", strerror(errno));
						}
						execvp(command, arguments); //Runs the command with the arguments specified
						fprintf(stderr, "Syserr with command: %s\n", command);
						return 0; //Signal end of child process
					default:
						waitpid(childPID, NULL, WUNTRACED);//Wait for child process to finish
				}
				kill(childPID, SIGTERM);//Kill child process

			}

		}

	}
	return 0;
}
