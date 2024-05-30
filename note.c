#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "./include/alpha2int.h"

typedef unsigned int uint;
typedef unsigned long ulong;

// Global data
enum comm {
	helpComm=536744u, lsComm=620u, addComm=4225u, editComm=664709u, 
	rmComm=434u, cpComm=515u, mvComm=717u, showComm=769299u,
	mkdirComm=19173741u, rmdirComm=19173810u
};
char* editor = NULL;
uint subComm = helpComm;
char*	notesDirName = NULL;
char* fileArg1Name = NULL;
char* fileArg2Name = NULL;
DIR* notesDir = NULL;
FILE* arg1File = NULL;
FILE* arg2File = NULL;

// Function prototype declarations
uint isAlphaOnly(char* str);
uint isDigitOnly(char* str);
char** sortStrArray(char** inArr, uint numOfStrings);
char* fullPathOfFileName(char* fileName);
uint readInput(int argc, char** argv);
uint printHelp();
uint showNote();
uint lsNote(char* searchStr);
uint addNote(char* noteName);
uint editNote();
uint rmNote(char* noteName);
uint cpNote(char* noteName1, char* noteName2);
uint mvNote(char* noteName1, char* noteName2);
uint mkNoteDir(char* dirName);
uint rmNoteDir(char* dirName);

int main(int argc, char** argv) {
	notesDirName = getenv("HOME");
	if (notesDirName == NULL) {
		puts("ERROR: environment variable 'HOME' not set\n");
		return 1;
	}
	editor = getenv("EDITOR");
	if (editor == NULL) {
		puts("ERROR: environment variable 'EDITOR' not set\n");
		return 1;
	}
	#ifndef _WIN32
	strcat(notesDirName, "/documents/notes");
	#endif
	#ifdef _WIN32
	strcat(notesDirName, "\\documents\\notes");
	#endif
	notesDir = opendir(notesDirName);
	if (notesDir == NULL) {
		printf("ERROR: Could not open notes directory '%s'\n", notesDirName);
		return 2;
	}
	readInput(argc, argv);
	switch (subComm) {
		case showComm:
			showNote();
			break;
		case helpComm:
			printHelp();
			break;
		case lsComm:
			lsNote(NULL);
			break;
		case addComm:
			printf("addNote()\n");
			break;
		case editComm:
			editNote();
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

uint isAlphaOnly(char* str) {
	uint retVal=0;
	for (uint i=0; i<strlen(str); i++) {
		retVal=(retVal | !(str[i]>>6));
		//printf("DEBUG:\t%c\t%u\n", str[i], !(str[i]>>6) );
	}
	return !retVal; //invert so that true=1
}

char** sortStrArray(char** inArr, uint numOfStrings) {
	char** outArr = malloc(numOfStrings*sizeof(char*));
	for (uint i = 0; i < numOfStrings; i++) {
		outArr[i] = malloc(255*sizeof(char));
		strcpy(outArr[i], inArr[i]);
	}
	char tempStr[255];
	uint keepSorting = 1;
	while (keepSorting == 1) {
		keepSorting = 0;
		for (uint i = 0; i < (numOfStrings-1); i++) {
			if ( strcmp(outArr[i], outArr[i+1]) > 1 ) {
				keepSorting = 1;
				strcpy(tempStr, outArr[i+1]);
				strcpy(outArr[i+1], outArr[i]);
				strcpy(outArr[i], tempStr);
				strcpy(tempStr, "");
			}
		}
	}
	return outArr;
}

char* fullPathOfFileName(char* fileName) {
	char* fullPath = NULL;
	fullPath = malloc((strlen(notesDirName)+strlen(fileName)+1)*sizeof(char));
	strcpy(fullPath, notesDirName);
	#ifndef _WIN32
	strcat(fullPath, "/");
	#endif
	#ifdef _WIN32
	strcat(fullPath, "\\");
	#endif
	strcat(fullPath, fileName);
	strcat(fullPath, ".txt");
	return fullPath;
}

uint readInput(int argc, char** argv) {
	uint retVal=0;
	switch (argc) {
		case 2:
			fileArg1Name = fullPathOfFileName(argv[1]);
			if ((arg1File = fopen(fileArg1Name,"r+"))!=NULL) {
				subComm=showComm;
			}
			else if (isAlphaOnly(argv[1])) {
				subComm=alpha2int(argv[1]);
			}
			break;
		case 3:
			if (isAlphaOnly(argv[1])) {
				subComm=alpha2int(argv[1]);
			}
			fileArg1Name = fullPathOfFileName(argv[2]);
			arg1File = fopen(fileArg1Name,"r+");
			retVal = !!(arg1File == NULL);
			break;
		case 4:
			if (isAlphaOnly(argv[1])) {
				subComm=alpha2int(argv[1]);
			}
			fileArg1Name = fullPathOfFileName(argv[2]);
			arg1File = fopen(fileArg1Name,"r+");
			fileArg2Name = fullPathOfFileName(argv[3]);
			arg2File = fopen(fileArg2Name,"r+");
			retVal = !!(arg1File == NULL);
			retVal = !!(arg2File == NULL);
			break;
	}
	return retVal;
}

uint printHelp() {
	char* helpText=
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
"	rmdir	|	Remove subdir named ARG1\n";
	printf("%s", helpText);
	return 0;
}

uint showNote() {
	char* noteContent = NULL;
	ulong length = 0;
	ulong bytesRead = 0;
	uint retVal = 0;
	if (arg1File) {
		fseek(arg1File, 0, SEEK_END);
		length = ftell (arg1File);
		fseek (arg1File, 0, SEEK_SET);
		noteContent = malloc(length+1);
		if (noteContent) {
			bytesRead = fread (noteContent, 1, length, arg1File);
			retVal = (uint)(length-bytesRead);
			noteContent[length]='\0';
		}
		fclose (arg1File);
	}
	printf("%s", noteContent);
	return retVal;
}

uint lsNote(char* searchStr) {
	char** fileNames = NULL;
	struct dirent* entry = NULL;
	struct stat statBuffer;
	uint numOfFiles = 0;
	while ((entry = readdir(notesDir)) != NULL) {
		numOfFiles++;
	}
	fileNames = malloc(numOfFiles*sizeof(char*));
	rewinddir(notesDir);
	uint i = 0;
	while ((entry = readdir(notesDir)) != NULL) {
		if (entry) {
			fileNames[i]=malloc(255*sizeof(char));
			strcpy(fileNames[i], entry->d_name);
			i++;
		}
	}
	char** sortedFileNames = sortStrArray(fileNames, numOfFiles);
	for (uint i = 0; i < numOfFiles; i++) {
		char fileExt[5];
		sprintf(fileExt, "%.*s", 4, sortedFileNames[i]+strlen(sortedFileNames[i])-4);
		//printf("DEBUG: fileExt=%s\n", fileExt);
		if (strcmp(fileExt, ".txt")) {
			strcpy(sortedFileNames[i], "");
		}
		else {
			sortedFileNames[i][strlen(sortedFileNames[i])-4]='\0';
		}
	}
	for (uint i=0; i<numOfFiles; i++) {
		if (strcmp(sortedFileNames[i], "")) {
			puts(sortedFileNames[i]);
		}
	}
	return 0;
}

uint editNote() {
	char editComm[255] = "";
	sprintf(editComm, "%s '%s'", editor, fileArg1Name);
	return system(editComm);
}
