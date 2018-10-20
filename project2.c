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
	// printf("fpath: %s\n", fpath);
	// return 0;
	// printf("initialMimic: %s\n", initialMimic);

	// printf("here\n");
	//
	// //Get the base folder from the source
	// char initialSourceCopy[strlen(fpath)];
	// initialSourceCopy[0] = '\0';
	// strcat(initialSourceCopy, initialSource);
	// char* sourceBaseFolder;
	// sourceBaseFolder = basename(initialSourceCopy);
	// printf("sourceBaseFolder: %s\n", sourceBaseFolder);
	// // return 0;
	//
	// char* sourceBasePlus;
	// sourceBasePlus = strstr(fpath, sourceBaseFolder);
	// printf("sourceBasePlus: %s\n", sourceBasePlus);
	// // strcat(sourceBasePlus, strstr(sourceBaseFolder, fpath));
	// return 0;
	// //
	// // // sourceBasePlus = strstr(sourceBaseFolder, fpath);
	// // printf("sourceBase plus everything after: %s\n", sourceBasePlus);
	// // printf("Got here\n");
	// // // sourceBasePlus[0] = '\0'	;
	// // return 0;
	//
	//
	// // if copying into existing directory
	// // just source folder into dest and keep contents in source
	// if(isDirectory(initialMimic)){
	//
	//
	// }
	// // Copying contents into new directory
	// // bascially just rename source to dest
	// else if(!isDirectory(initialMimic)){
	//
	//
	// }
	//






















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
			// Ignore warnings if the directory already exists
			if(errno != EEXIST){
				printf("%s\n", strerror(errno));
				return -1;
			}
		}
	}


























		// ---------------------------------------------------------------------------------------------------------------------------
		// ---------------------------------------------------------------------------------------------------------------------------
		// ---------------------------------------------------------------------------------------------------------------------------
		// ---------------------------------------------------------------------------------------------------------------------------
		// ---------------------------------------------------------------------------------------------------------------------------
		// ---------------------------------------------------------------------------------------------------------------------------
		// ---------------------------------------------------------------------------------------------------------------------------

	// 	// printf("fpath: %s\n", fpath);
	// 	// printf("IS DIR\n");
	// 	strcat(containingFolder, initialMimic); //Copies initial mimic to containingFolder
	// 	strcat(finalDestPath, containingFolder); //adds the containingFolder to the final dest path
	// 	strcat(finalDestPath, "/"); //Adds a slash to add following
	// 	// if(!isMimicIntoNewDir)
	// 	// 	strcat(finalDestPath, fpath); //Adds the path of the file/folder to end of containing folder
	// 	// else{
	// 			printf("sourcepath: %s\n", fpath);
	// 			printf("finalDestPath: %s\n", finalDestPath);
	// 		// printf("INTO ELSE\n");
	// 		char basenameInitialMimic[MAX_FILENAME];
	// 		basenameInitialMimic[0] = '\0';
	// 		strcat(basenameInitialMimic, initialMimic);
	// 		basename(basenameInitialMimic);
	// 		int slashLocation = 0;
	// 		for(slashLocation = 0; slashLocation < strlen(fpath); ++slashLocation){
	// 			if(fpath[slashLocation] == '/')
	// 				break;
	// 		}
	// 		if(slashLocation == strlen(fpath)){
	// 			strcat(finalDestPath, basenameInitialMimic);
	// 		}
	// 		else{
	// 			char fpathAfterFirstSlash[strlen(fpath) - slashLocation - 1];
	// 			fpathAfterFirstSlash[0] = '\0';
	// 			for(int i = slashLocation + 1; i < strlen(fpath); ++i){
	// 				fpathAfterFirstSlash[i-(slashLocation+1)] = fpath[i];
	// 				// printf("fpath[%i]: %c\n", i, fpath[i]);
	// 			}
	// 			fpathAfterFirstSlash[strlen(fpath) - slashLocation - 1] = '\0';
	// 			printf("fpathAfterFirstSlash: %s\n", fpathAfterFirstSlash);
	// 			// fpathAfterFirstSlash[strlen(fpath)] = '\0';
	// 			// printf("fpathAfterFirstSlash: %s\n", fpathAfterFirstSlash);
	// 			char newFPath[MAX_FILENAME];
	// 			newFPath[0] = '\0';
	// 			strcat(newFPath, basenameInitialMimic);
	// 			strcat(newFPath, "/");
	// 			strcat(newFPath, fpathAfterFirstSlash);
	// 			strncpy(finalDestPath, newFPath, strlen(newFPath));
	// 			finalDestPath[strlen(newFPath)] = '\0';
	// 			printf("newFPath: %s\n", newFPath);
	// 			// strcat(newFPath, fpathAfterFirstSlash);
	// 			// strcat(finalDestPath, fpathAfterFirstSlash);
	// 			printf("finalDestPath: %s\n", finalDestPath);
	// 		}
	// 	}
	// // }
	//
	//
	// //Or if the parent of the source path is a directory
	// else if(isDirectory(dirnameInitialMimic)){
	// 	// printf("191: \tinitialMimic: %s\n", initialMimic);
	// 	isMimicIntoNewDir = 1;
	// 	mkdirz(initialMimic, 0);
	// 	return 0;
	//
	// 	// The basename of the destination (the folder that does not exist)
	// 	// replaces the name of the base folder from the source
	// 	// Below stores the basename of initialMimic in basenameInitialMimic
	// 	// char basenameInitialMimic[strlen(initialMimic)];
	// 	// basenameInitialMimic[0] = '\0';
	// 	// strcat(basenameInitialMimic, initialMimic);
	// 	// basename(basenameInitialMimic);
	// 	// int slashLocation = 0;
	// 	// for(slashLocation = 0; slashLocation < strlen(fpath); ++slashLocation){
	// 	// 	if(fpath[slashLocation] == '/')
	// 	// 		break;
	// 	// }
	// 	// if(slashLocation == strlen(fpath))
	// 	// 	printf("NO SLASH\n");
	// 	// else{
	// 	// 	printf("Slash location: %i\n", slashLocation);
	// 	// 	for(int i = 0; i < strlen(basenameInitialMimic); ++i){
	// 	//
	// 	// 	}
	// 	//
	// 	// }
	// }
	// else if(!strcmp(dirnameInitialMimic, initialMimic)){
	// 	isMimicIntoNewDir = 1;
	// 	mkdirz(dirnameInitialMimic, 0);
	// 	return 0;
	// }
	// else{
	// 	// printf("dirnameInitialMimic: %s\n", dirnameInitialMimic);
	// 	printf("ERROR: directory '%s' is not a valid directory(parent does not exist)\n");
	// 	return -1;
	// }
	//
	// // If fpath(source path) points to a directory, then a new directory is created with the same name as the old one
	// if(tflag == FTW_D){ //fpath points to a directory
	// 	// Gets the stat of the source path so we can apply the same permissions to the final destination
	// 	struct stat sourceStat;
	// 	stat(fpath, &sourceStat);
	// 	if(mkdir(finalDestPath, sourceStat.st_mode) == -1){ //Creates a new directory with same permissions as fpath
	// 		// Ignore warnings if the directory already exists
	// 		if(errno != EEXIST){
	// 			printf("%s\n", strerror(errno));
	// 			return -1;
	// 		}
	// 	}
	// }
	//
	// //If fpath points to a file, then that file is copied
	// else if(tflag == FTW_F){ //fpath points to a file
	// 	if(copyFile(fpath, finalDestPath) == -1){
	// 		if(copyFile(fpath, dirname(finalDestPath)) == -1){
	// 				printf("ERROR copying file\n");
	// 				return -1;
	// 		}
	// 	}
	// }

	// 	//Flags and permssions for opening the file descriptors
	//   unsigned int sourceFlags = O_RDONLY;
	//   unsigned int destFlags = O_CREAT | O_WRONLY | O_TRUNC;
	//   unsigned int destPermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw-
	//
	//   // Opens source and dest for copying
	//   int sourceFileDescriptor = open(fpath, sourceFlags);
	//   int destFileDescriptor = open(finalDestPath, destFlags, destPermissions);
	//
	// 	// Attempts to copy file using the destpath given
	//   ssize_t num_read;
	//   char buf[MAX_BUFFER];
	//   while((num_read = read(sourceFileDescriptor, buf, MAX_BUFFER)) > 0){
	//     if(write(destFileDescriptor, buf, num_read) != num_read){
	//       printf("ERROR during writing\n");
	//     }
	//   }
	// 	// If above fails, tries to copy file using the dirname of the destpath given
	//   if(num_read == -1){
	//     while((num_read = read(sourceFileDescriptor, buf, MAX_BUFFER)) > 0){
	// 			destFileDescriptor = open(dirname(finalDestPath), destFlags, destPermissions);
	// 			if(write(destFileDescriptor, buf, num_read) != num_read){
	// 				printf("ERROR during writing");
	// 			}
	// 		}
	//   }
	// }
	return 0;
}

// Copies file from sourcePath to destPath
int mimic(char** inputs, int numberOfInputs){
	char sourcePath[MAX_FILENAME];
	sourcePath[0] = '\0';
	char destPath[MAX_FILENAME];
	destPath[0] = '\0';
	int isRecursive = 0;
	for(int i = 1; i < numberOfInputs; ++i){
		if(!strcmp(inputs[i], "-r")){
			isRecursive = 1;
		}
		else if(sourcePath[0] == '\0'){
			strncpy(sourcePath, inputs[i], strlen(inputs[i]));
			sourcePath[strlen(inputs[i])] = '\0';
			initialSource[0] = '\0';
			strcat(initialSource, sourcePath);
		}
		else{
			strncpy(destPath, inputs[i], strlen(inputs[i]));
			destPath[strlen(inputs[i])] = '\0';
			printf("inputs[%i]: %s\n", i, inputs[i]);
			printf("DestPath: %s\n", destPath);
			initialMimic[0] = '\0';
			strcat(initialMimic, destPath);
		}
	}

	printf("SourcePath: %s\n", sourcePath);
	printf("DestPath: %s\n", destPath);

	if(isDirectory(sourcePath)){
		// Copy/move directory recursively
		if(isRecursive || isDirectoryEmpty(sourcePath)){
			if(nftw(sourcePath, recursiveMimicMorph, 20, 0) == 0){
				initialMimic[0] = '\0';
				isMimicIntoNewDir = 0;
				initialDirExists = 1;
				return 0;
			}
		}
		// Try to copy/move non-empty directory without recursive flag
		else{
			printf("ERROR: Trying to copy non-empty directory without '-r'\n");
			return -1;
		}
	}

	// printf("Source: %s\n", sourcePath);
	// printf("Destination: %s\n", destPath);
	// printf("Is recursive: %i\n", isRecursive);
	return 0;
	// // if(firstPass){
	// // 		initialMimic[0] = '\0';
	// // 		strcat(initialMimic, destPath);
	// // }
	// // if(nftw(sourcePath, display_info, 20, 0) == 0){
	// // 	initialMimic[0] = '\0';
	// // 	return 0;
	// // }
	//
	//
	// // printf("Initial Mimic: %s\n", initialMimic);
	//
	// // printf("Destination for mimic inside mimic: %s\n", destinationForMimic);
	//
  // // Sets flags and permissions for the files for future use
  // unsigned int sourceFlags = O_RDONLY;
  // unsigned int destFlags = O_CREAT | O_WRONLY | O_TRUNC;
  // unsigned int destPermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw-
	//
  // // Opens source
  // int sourceFileDescriptor = open(sourcePath, sourceFlags);
	//
  // // Determines if source and path are directories
  // int isSourceDirectory = isDirectory(sourcePath);
  // int isDestDirectory = isDirectory(destPath);
	//
  // if(isDestDirectory){
	// 	// printf("Dest Path inside if loop: %s", destPath);
	//
	//
  //   // Below appears to be working for now
  //   char* slash = "/";
	// 	char sourcePathAsArray[10];
	// 	strncpy(sourcePathAsArray, sourcePath, strlen(sourcePath));
	// 	sourcePathAsArray[strlen(sourcePath)] = '\0';
	// 	// printf("Source path as array: %s\n", sourcePathAsArray);
	// 	char* sourceBaseName = basename(sourcePathAsArray);
	// 	// printf("Source base name: %s\n", sourceBaseName);
	// 	size_t destPathWithFileNameSize = strlen(destPath) + strlen(slash) + strlen(sourceBaseName);
	// 	// destPathWithFileName = strcat(destPathWithFileName, destPath);
	//
	// 	char destPathWithFileName[MAX_FILENAME];
	// 	destPathWithFileName[0] = '\0';
	// 	strcat(destPathWithFileName, destPath);
	// 	strcat(destPathWithFileName, slash);
	// 	strcat(destPathWithFileName, sourceBaseName);
	// 	destPath = (char*)(malloc)(destPathWithFileNameSize * sizeof(char));
	// 	strcpy(destPath, destPathWithFileName);
	// 	// destPath = destPathWithFileName;
	// 	// printf("Destpath with filename: %s\n", destPathWithFileName);
	//
	// 	if(inFTW){
	// 		//dest path is just initial mimic + source path
	// 		destPathWithFileName[0] = '\0';
	// 		strcat(destPathWithFileName, initialMimic);
	// 		strcat(destPathWithFileName, slash);
	// 		strcat(destPathWithFileName, sourcePath);
	// 		destPathWithFileNameSize = strlen(destPath) + strlen(slash) + strlen(sourcePath);
	// 		destPath = (char*)(malloc)(strlen(destPathWithFileName) * sizeof(char));
	// 		strcpy(destPath, destPathWithFileName);
	// 	}
	//
	// 	// char* destPathWithFileName = (char*)(malloc)(destPathWithFileNameSize * sizeof(char));
	// 	// destPathWithFileName = strcat(destPathWithFileName, destPath);
	// 	// destPathWithFileName = strcat(destPathWithFileName, slash);
	// 	// destPathWithFileName = strcat(destPathWithFileName, sourceBaseName);
	// 	// destPath = destPathWithFileName;
	// 	// printf("Destpath with filename: %s\n", destPathWithFileName);
	//
	// 	// printf("Dest path inside making full dest name: %s\n", destPath);
	//
	// 	// printf("%i\n", isDirectory(destPath));
	//
  //   // Below is given in project specs
  //   // printf("Dest is directory\n");
  //   // char destName[MAX_FILENAME/2];
  //   // char destPath[MAX_FILENAME/2];
  //   // char destDirectory[MAX_FILENAME/2];
  //   //
  //   // strcpy(destName, basename(sourcePath)); //Gets the file name from source path
  //   // printf("Dest File Name: %s", destName);
  //   // strcpy(destDirectory, basename(destPath)); //Gets the last folder name from destination path
  //   // strcpy(destPath, dirname(destPath)); //Gets the path from destination path
  //   //
  //   // // Creates fullDestination path using elements of path
  //   // fullDestination[0] = '\0'; //Zeroes out string
  //   // strcat(fullDestination, destPath);
  //   // strcat(fullDestination, "/");
  //   // strcat(fullDestination, destDirectory);
  //   // strcat(fullDestination, "/");
  //   // strcat(fullDestination, destName);
  //   //
  //   // printf("%s\n", fullDestination);
  // }
	//
	// // destinationForMimic = destPath;
	//
	// // Copies if source path points to directory and that directory is empty
	// if(isSourceDirectory && (isDirectoryEmpty(sourcePath) || inFTW)){
	// 	// printf("169\n");
	// 	struct stat sourceStat;
	// 	stat(sourcePath, &sourceStat);
	// 	if(isDestDirectory){
	// 		// printf("173\n");
	// 		if(mkdir(destPath, sourceStat.st_mode) == -1){
	// 			if(errno != EEXIST){
	// 				printf("%s\n", strerror(errno));
	// 			}
	// 		}
	// 		else
	// 			printf("Directory '%s' created\n", destPath);
	// 	}
	// 	else{
	// 		// printf("181\n");
	// 		char destPathAsArray[10];
	// 		strncpy(destPathAsArray, destPath, strlen(destPath));
	// 		destPathAsArray[strlen(destPath)] = '\0';
	//
	// 		if(isDirectory(dirname(destPathAsArray))){
	// 		// printf("187\n");
	// 			// printf("%s\n", );
	// 				if(mkdir(destPath, sourceStat.st_mode) == -1){
	// 					if(errno != EEXIST){
	// 						printf("%s\n", strerror(errno));
	// 					}
	// 				}
	// 		}
	// 		else{
	// 			printf("Handle Later\n");
	//
	// 		}
	// 	}
	// 	return 0;
	// }
	// else if(isSourceDirectory && isRecursive == 1){
	//
	// 	int flags = 0;
	// 	if(nftw(sourcePath, display_info, 20, flags))
	// 	// Just for now
	// 	printf("Directory is not empty\n");
	// 	return -1;
	// }
	//
	// printf("\t%s", sourcePath);
	// printf("\t%s\n", destPath);
	// for(int i = 0; i < strlen(destPath); ++i){
	// 	printf("\t\t%i\n", destPath[i]);
	// }
	// // printf("Running long\n");
	//
  // int destFileDescriptor = open(destPath, destFlags, destPermissions);
	// if(destFileDescriptor == -1){
	// 	printf("%s\n", strerror(errno));
	// 	printf("Destination not valid\n");
	// 	exit(0);
	// }
	//
  // ssize_t num_read;
  // char buf[MAX_BUFFER];
  // while((num_read = read(sourceFileDescriptor, buf, MAX_BUFFER)) > 0){
  //   if(write(destFileDescriptor, buf, num_read) != num_read){
  //     printf("ERROR during writing\n");
  //   }
  // }
  // if(num_read == -1 && isSourceDirectory){
  //   while((num_read = read(sourceFileDescriptor, buf, MAX_BUFFER)) > 0){
	// 		destFileDescriptor = open(dirname(destPath), destFlags, destPermissions);
	// 		if(write(destFileDescriptor, buf, num_read) != num_read){
	// 			printf("ERROR during writing");
	// 		}
	// 	}
  // }
	// else if(num_read == -1){
	// 	printf("ERROR during writing");
	// }
	//
  // if(close(sourceFileDescriptor) == -1){
  //   printf("ERROR closing source\n");
  // }
	//
  // if(close(destFileDescriptor) == -1){
  //   printf("ERROR closing destination\n");
  // }
	//
  // // Below is the mimic code for project1
  // // ------------------------------------------------------
  // // ------------------------------------------------------
	// // // Determines if source file is a file or not
	// // struct stat sourceBuf;
	// // stat(sourcePath, &sourceBuf);
	// // if(S_ISREG(sourceBuf.st_mode) == 0){
	// // 	fprintf(stderr, "ERROR: Source is not file\n");
	// // 	return -1;
	// // }
  // //
	// // // Opens files as descriptors to feed into sendfile function
	// // int sourceDescriptor = open(sourcePath, O_RDWR);
	// // int destDescriptor = open(destPath, O_CREAT | O_RDWR, S_IRWXU);
  // //
	// // // Gets size of source file to provide to sendfile
	// // struct stat toGetSize;
	// // fstat(sourceDescriptor, &toGetSize);
	// // int sourceSizeInBytes = toGetSize.st_size;
  // //
	// // // Copies file
	// // // http://man7.org/linux/man-pages/man2/sendfile.2.html
	// // if(sendfile(destDescriptor, sourceDescriptor, NULL, sourceSizeInBytes) == -1){
	// // 	fprintf(stderr, "ERROR: Invalid mimic destination\n");
	// // 	return -1;
	// // }
  // //
	// // destinationForMimic = initialMimic;
	// return 0;
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

	// Attempts to copy file using the destpath given
  ssize_t num_read;
  char buf[MAX_BUFFER];
  while((num_read = read(sourceFileDescriptor, buf, MAX_BUFFER)) > 0){
    if(write(destFileDescriptor, buf, num_read) != num_read){
      printf("ERROR during writing\n");
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
				isMorph = 0;
				// char* source = args[1];
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
				isMorph = 1;

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
