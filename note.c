#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "./include/alpha2int.h"

typedef unsigned int uint;
typedef unsigned long ulong;

// Global data
// "comm" is used to convert subcommand arguments into integers that can be
// used with a switch statement
enum comm {
	helpComm=536744u, lsComm=620u, addComm=4225u, editComm=664709u, 
	rmComm=434u, cpComm=515u, mvComm=717u, /*showComm=769299u,*/
	mkdirComm=19173741u, rmdirComm=19173810u
};

// Function prototype declarations
char** sortStrArray(char** inArr, uint numOfStrings);
// char* fullPathOfFileName(char* fileName, char* notesDirPath);
void printHelp();
uint showNote(char* notePath);
uint lsNote(int argc, char** argv, char* notesDirPath);
// uint addNote();
uint editNote(char* fileArg1Name);
// uint rmNote();
// uint cpNote();
// uint mvNote();
// uint mkNoteDir();
// uint rmNoteDir();

int main(int argc, char** argv) {
	uint	subComm			= helpComm;
	char*	homeDirPath		= NULL;
	char*	notesDirPath	= NULL;
	struct	stat statBuf; 

	#ifndef _WIN32
	homeDirPath = getenv("HOME");
	if (homeDirPath == NULL) {
		puts("ERROR: environment variable 'HOME' not set\n");
		return 1;
	}
	#endif
	#ifdef _WIN32
	homeDirPath = getenv("HOME");
	if (homeDirPath == NULL) {
		homeDirPath = getenv("USERPROFILE");
	}
	if (homeDirPath == NULL) {
		puts("ERROR: environment variables 'HOME' and 'USERPROFILE' not set");
		return 1;
	}
	#endif
	notesDirPath = malloc((strlen(homeDirPath)+18)*sizeof(char));
	strcpy(notesDirPath, homeDirPath);
	// Convert Windows backslashes to normal forward slashes
	for (uint i = 0; i <= strlen(notesDirPath); i++) {
		if (notesDirPath[i] == '\\') {
			notesDirPath[i] = '/';
		}
	}
	strcat(notesDirPath, "/documents/notes");

	// Read first argument as subcommand
	if ( argc > 1 ) {
		subComm = alpha2int(argv[1]);
	}
	else if ( argc > 4 ) {
		fputs("ERROR: too many arguments", stderr);
	}
	switch (subComm) {
		case helpComm:
			printHelp();
			break;
		case lsComm:
			lsNote(argv, notesDirPath);
			break;
		case addComm:
			printf("addNote()\n");
			break;
		case editComm:
			editNote(argv, notesDirPath);
			break;
		case rmComm:
			printf("rmNote()\n");
			break;
		case cpComm:
			printf("cpNote()\n");
			break;
		case mvComm:
			printf("mvNote()\n");
			break;
		case mkdirComm:
			printf("mkNoteDir()\n");
			break;
		case rmdirComm:
			printf("rmNoteDir()\n");
			break;
		default:
			// First argument isn't a subcommand. Is it a file?
			char* filePath = malloc((strlen(notesDirPath)+strlen(argv[1])+6)*sizeof(char));
			sprintf(filePath, "%s/%s.txt", notesDirPath, argv[1]);
			if ( stat(filePath, &statBuf) == 0 ) {
				showNote(filePath)
			}
			else printHelp();
			break;
	}
	return 0;
}

// Bubble sort
char** sortStrArray(char** inArr, uint numOfStrings) {
	char** outArr = malloc(numOfStrings*sizeof(char*));
	for (uint i = 0; i < numOfStrings; i++) {
		outArr[i] = malloc((strlen(inArr[i])+1)*sizeof(char));
		strcpy(outArr[i], inArr[i]);
	}
	char* tempStr = NULL;
	uint keepSorting = 1;
	while (keepSorting == 1) {
		keepSorting = 0;
		for (uint i = 0; i < (numOfStrings-1); i++) {
			if ( strcmp(outArr[i], outArr[i+1]) > 1 ) {
				keepSorting = 1;
				tempStr = outArr[i+1];
				outArr[i+1] = outArr[i];
				outArr[i] = tempStr;
			}
		}
	}
	return outArr;
}

void printHelp() {
	puts(
"usage: note NOTE|SUBCOMMAND [ARG1 [ARG2]]\n"
"    Display NOTE or do SUBCOMMAND\n"
"subcommands:\n"
"    help   Display this help text\n"
"    ls     List notes and subdirs in note directory. If ARG1 is a\n"
"           subdir list notes/subdirs inside it.\n"
"    add    Append the string ARG2 to a new line at the end of ARG1.txt\n"
"    edit   Open ARG1.txt with $EDITOR\n"
"    rm     Remove ARG1.txt\n"
"    cp     Copy ARG1.txt to ARG2.txt\n"
"    mv     Move or rename ARG1.txt to ARG2.txt\n"
"    mkdir  Create subdir named ARG1\n"
"    rmdir  Remove subdir named ARG1");
	return;
}

uint showNote(char* notePath) {
	char* noteContent = NULL;
	FILE* noteFile = fopen(notePath, "r+");
	ulong length = 0;
	ulong bytesRead = 0;
	if (noteFile) {
		fseek(noteFile, 0, SEEK_END);
		length = ftell(noteFile);
		fseek(noteFile, 0, SEEK_SET);
		noteContent = malloc(length+1);
		if (noteContent) {
			bytesRead = fread (noteContent, 1, length, noteFile);
			noteContent[length]='\0';
		}
		fclose (noteFile);
	}
	fputs(noteContent, stdout);
	return (uint)(length-bytesRead);
}

uint lsNote(int argc, char** argv, char* notesDirPath) {
	DIR* notesDir = NULL;
	char** fileNames = NULL;
	char* statFilePath = NULL;
	char* lsDir = NULL;
	char fileExt[5] = {0,0,0,0,0};
	struct dirent* entry = NULL;
	struct stat statBuf;
	uint isTxtFile = 0;
	uint numOfFiles = 0;

	// Check if argument to ls subcommand is a directory
	if ( argc == 3 ) {
		lsDir = malloc((strlen(notesDirPath)+strlen(argv[2])+2)*sizeof(char));
		sprintf(lsDir, "%s/%s", notesDirPath, argv[2]);
		if ( stat(lsDir, statBuf) != 0 ) {
			fprintf(stderr, "ERROR: Cannot open directory '%s'", lsDir);
			return 2;
		}
	}
	else if ( argc > 3 ) {
		fputs("ERROR: Too many arguments to subcommand 'ls'", stderr);
		return 3;
	}
	else {
		lsDir = malloc((strlen(notesDirPath)+1)*sizeof(char));
	}

	// Open the directory to list
	notesDir = opendir(lsDir);
	if ( notesDir == NULL ) {
		fprintf(stderr, "ERROR: Cannot open directory '%s'", lsDir);
		return 2;
	}

	// Count files so we can allocate enough pointers for the filenames
	while ((entry = readdir(notesDir)) != NULL) {
		numOfFiles++;
	}
	fileNames = malloc(numOfFiles*sizeof(char*));

	// Reread the dir to record the names
	rewinddir(notesDir);
	uint i = 0;
	while ((entry = readdir(notesDir)) != NULL) {
		if (entry) {
			fileNames[i]=malloc((strlen(entry->d_name)+1)*sizeof(char));
			strcpy(fileNames[i], entry->d_name);
			i++;
		}
	}

	// Sort the file names alphabetically
	char** sortedFileNames = sortStrArray(fileNames, numOfFiles);

	// Filter the file names by type
	for (uint i = 0; i < numOfFiles; i++) {
		// Construct full file path for stat()
		statFilePath = realloc(statFilePath, (strlen(sortedFileNames[i])+strlen(notesDirPath)+1)*sizeof(char));
		sprintf(statFilePath, "%s/%s", notesDirPath, sortedFileNames[i]);
		stat(statFilePath, &statBuf);
		// Check file extension to identify txt files
		sprintf(fileExt, "%.*s", 4, sortedFileNames[i]+strlen(sortedFileNames[i])-4);
		isTxtFile = !strcmp(fileExt, ".txt");
		// Append a "/" to the names of directories
		if ( (statBuf.st_mode & S_IFMT) == S_IFDIR) {
			sortedFileNames[i] = realloc(sortedFileNames[i], (strlen(sortedFileNames[i])+2)*sizeof(char));
			strcat(sortedFileNames[i], "/");
		}
		// Hide extension on note names
		else if (isTxtFile) {
			sortedFileNames[i][strlen(sortedFileNames[i])-4]='\0';
		}
		// Hide files that aren't txt or dir
		else {
			free(sortedFileNames[i]);
			sortedFileNames[i] = NULL;
		}
	}

	// Print file names. Start at 2 to hide "." and ".."
	for (uint i=2; i < numOfFiles; i++) {
		if (sortedFileNames[i]) {
			puts(sortedFileNames[i]);
		}
	}
	return 0;
}

uint editNote(argv, notesDirPath) {
	if ( argc > 3) {
		fputs("ERROR: Too many arguments to subcommand 'edit'", stderr);
		return 3;
	}
	char* notePath = malloc((strlen(argv[2])+strlen(notesDirPath)+6)*sizeof(char));
	
	char* editor = getenv("EDITOR");
	if (editor == NULL) {
		fputs("ERROR: environment variable 'EDITOR' not set", stderr);
		return 1;
	}
	char* editComm = malloc((strlen(editor)+strlen(notePath)+2)*sizeof(char));
	sprintf(editComm, "%s \"%s\"", editor, notePath);
	return system(editComm);
}
