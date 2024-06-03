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
	rmComm=434u, cpComm=515u, mvComm=717u, showComm=769299u,
	mkdirComm=19173741u, rmdirComm=19173810u
};

// Function prototype declarations
char** sortStrArray(char** inArr, uint numOfStrings);
char* fullPathOfFileName(char* fileName, char* notesDirName);
void readInput(int argc, char** argv, uint* subComm, FILE** arg1File, FILE** arg2File, char** fileArg1Name, char** fileArg2Name, char** notesDirName);
void printHelp();
uint showNote(FILE* arg1File);
uint lsNote(DIR* notesDir, char* notesDirName);
// uint addNote();
uint editNote(char* fileArg1Name);
// uint rmNote();
// uint cpNote();
// uint mvNote();
// uint mkNoteDir();
// uint rmNoteDir();

int main(int argc, char** argv) {
	char* fileArg1Name = NULL;
	char* fileArg2Name = NULL;
	uint subComm = helpComm;
	char* homeDirName = NULL;
	char*	notesDirName = NULL;
	DIR* notesDir = NULL;
	FILE* arg1File = NULL;
	FILE* arg2File = NULL;

	#ifndef _WIN32
	homeDirName = getenv("HOME");
	if (homeDirName == NULL) {
		puts("ERROR: environment variable 'HOME' not set\n");
		return 1;
	}
	#endif
	#ifdef _WIN32
	homeDirName = getenv("HOME");
	if (homeDirName == NULL) {
		homeDirName = getenv("USERPROFILE");
	}
	if (homeDirName == NULL) {
		puts("ERROR: environment variables 'HOME' and 'USERPROFILE' not set\n");
		return 1;
	}
	#endif
	notesDirName = malloc((strlen(homeDirName)+19)*sizeof(char));
	strcpy(notesDirName, homeDirName);
	// Convert Windows backslashes to normal forward slashes
	for (uint i = 0; i <= strlen(notesDirName); i++) {
		if (notesDirName[i] == '\\') {
			notesDirName[i] = '/';
		}
	}
	strcat(notesDirName, "/documents/notes");
	notesDir = opendir(notesDirName);
	if (notesDir == NULL) {
		printf("ERROR: Could not open notes directory '%s'\n", notesDirName);
		return 2;
	}
	
	// readInput() parses the command line args and sets the main vars for the program 
	readInput(argc, argv, &subComm, &arg1File, &arg2File, &fileArg1Name, &fileArg2Name, &notesDirName);
	switch (subComm) {
		case showComm:
			showNote(arg1File);
			break;
		case helpComm:
			printHelp();
			break;
		case lsComm:
			lsNote(notesDir, notesDirName);
			break;
		case addComm:
			printf("addNote()\n");
			break;
		case editComm:
			editNote(fileArg1Name);
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
			printHelp();
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

char* fullPathOfFileName(char* fileName, char* notesDirName) {
	char* fullPath = NULL;
	fullPath = malloc((strlen(notesDirName)+strlen(fileName)+6)*sizeof(char));
	sprintf(fullPath, "%s/%s.txt", notesDirName, fileName);
	return fullPath;
}

void readInput(int argc, char** argv, uint* subComm, FILE** arg1File, FILE** arg2File, char** fileArg1Name, char** fileArg2Name, char** notesDirName) {
	switch (argc) {
		case 2:
			*fileArg1Name = fullPathOfFileName(argv[1], *notesDirName);
			if ((*arg1File = fopen(*fileArg1Name,"r+")) != NULL) {
				*subComm=showComm;
			}
			else *subComm=alpha2int(argv[1]);
			break;
		case 3:
			*subComm=alpha2int(argv[1]);
			*fileArg1Name = fullPathOfFileName(argv[2], *notesDirName);
			*arg1File = fopen(*fileArg1Name,"r+");
			break;
		case 4:
			*subComm=alpha2int(argv[1]);
			*fileArg1Name = fullPathOfFileName(argv[2], *notesDirName);
			*arg1File = fopen(*fileArg1Name,"r+");
			*fileArg2Name = fullPathOfFileName(argv[3], *notesDirName);
			*arg2File = fopen(*fileArg2Name,"r+");
			break;
	}
	return;
}

void printHelp() {
	puts(
"usage: note NOTE|SUBCOMMAND [ARG1 [ARG2]]\n"
"	Display NOTE or do SUBCOMMAND\n"
"subcommands:\n"
"	help	|	Display this help text\n"
"	ls		|	List notes and subdirs in note directory. If ARG1\n"
"				is a subdir list notes/subdirs inside it.\n"
"	add	|	Add a new note named ARG1.txt\n"
"	edit	|	Open ARG1.txt with $EDITOR\n"
"	rm		|	Remove ARG1.txt\n"
"	cp		|	Copy ARG1.txt to ARG2.txt\n"
"	mv		|	Move or rename ARG1.txt to ARG2.txt\n"
"	mkdir	|	Create subdir named ARG1\n"
"	rmdir	|	Remove subdir named ARG1");
	return;
}

uint showNote(FILE* arg1File) {
	char* noteContent = NULL;
	ulong length = 0;
	ulong bytesRead = 0;
	if (arg1File) {
		fseek(arg1File, 0, SEEK_END);
		length = ftell(arg1File);
		fseek(arg1File, 0, SEEK_SET);
		noteContent = malloc(length+1);
		if (noteContent) {
			bytesRead = fread (noteContent, 1, length, arg1File);
			noteContent[length]='\0';
		}
		fclose (arg1File);
	}
	fputs(noteContent, stdout);
	return (uint)(length-bytesRead);
}

uint lsNote(DIR* notesDir, char* notesDirName) {
	char** fileNames = NULL;
	char* statFileName = NULL;
	char fileExt[5] = {0,0,0,0,0};
	struct dirent* entry = NULL;
	struct stat statBuf;
	uint isTxtFile = 0;
	uint numOfFiles = 0;
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
	for (uint i = 0; i < numOfFiles; i++) {
		// Construct full file path for stat()
		statFileName = realloc(statFileName, (strlen(sortedFileNames[i])+strlen(notesDirName)+1)*sizeof(char));
		sprintf(statFileName, "%s/%s", notesDirName, sortedFileNames[i]);
		stat(statFileName, &statBuf);
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
	// Start at 2 to hide "." and ".."
	for (uint i=2; i < numOfFiles; i++) {
		if (sortedFileNames[i]) {
			puts(sortedFileNames[i]);
		}
	}
	return 0;
}

uint editNote(char* fileArg1Name) {
	char* editor = getenv("EDITOR");
	if (editor == NULL) {
		puts("ERROR: environment variable 'EDITOR' not set");
		return 1;
	}
	char* editComm = malloc((strlen(editor)+strlen(fileArg1Name)+2)*sizeof(char));
	sprintf(editComm, "%s \"%s\"", editor, fileArg1Name);
	return system(editComm);
}
