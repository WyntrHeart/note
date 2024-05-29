#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alpha2int.h>

typedef unsigned int uint;
typedef unsigned long ulong;

// Global data
enum comm {
	help=536744u, ls=620u, add=4225u, edit=664709u, 
	rm=434u, cp=515u, mv=717u, show=769299u,
	mkdir=19173741u, rmdir=19173810u
};
char* editor = NULL;
uint subComm = help;
char*	notesDirName = NULL;
char* fileArg1Name = NULL;
char* fileArg2Name = NULL;
DIR* notesDir = NULL;
FILE* arg1File = NULL;
FILE* arg2File = NULL;

// Function prototype declarations
uint readInput(int argc, char** argv);
uint isAlphaOnly(char* str);
uint isDigitOnly(char* str);
char** sortStrArray(char** inArr, uint numOfStrings);
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
	strcat(notesDirName, "/documents/notes");
	notesDir = opendir(notesDirName);
	if (notesDir == NULL) {
		printf("ERROR: Could not open notes directory '%s'\n", notesDirName);
		return 2;
	}
	readInput(argc, argv);
	switch (subComm) {
		case show:
			showNote();
			break;
		case help:
			printHelp();
			break;
		case ls:
			lsNote(NULL);
			break;
		case add:
			printf("addNote()\n");
			break;
		case edit:
			editNote();
			break;
		case rm:
			printf("rmNote()\n");
			break;
		case cp:
			printf("cpNote()\n");
			break;
		case mv:
			printf("mvNote()\n");
			break;
		case mkdir:
			printf("mkNoteDir()\n");
			break;
		case rmdir:
			printf("rmNoteDir()\n");
			break;
		default:
			printHelp();
			break;
	}
	return 0;
}

uint readInput(int argc, char** argv) {
	uint retVal=0;
	switch (argc) {
		case 2:
			fileArg1Name=malloc((strlen(notesDirName)+strlen(argv[1])+1)*sizeof(char));
			strcpy(fileArg1Name, notesDirName);
			strcat(fileArg1Name, "/");
			strcat(fileArg1Name, argv[1]);
			strcat(fileArg1Name, ".txt");
			if ((arg1File = fopen(fileArg1Name,"r+"))!=NULL) {
				subComm=show;
			}
			else if (isAlphaOnly(argv[1])) {
				subComm=alpha2int(argv[1]);
			}
			break;
		case 3:
			if (isAlphaOnly(argv[1])) {
				subComm=alpha2int(argv[1]);
			}
			fileArg1Name=malloc((strlen(notesDirName)+strlen(argv[2])+1)*sizeof(char));
			strcpy(fileArg1Name, notesDirName);
			strcat(fileArg1Name, "/");
			strcat(fileArg1Name, argv[2]);
			strcat(fileArg1Name, ".txt");
			arg1File = fopen(fileArg1Name,"r+");
			retVal = !!(arg1File == NULL);
			break;
		case 4:
			if (isAlphaOnly(argv[1])) {
				subComm=alpha2int(argv[1]);
			}
			fileArg1Name=malloc((strlen(notesDirName)+strlen(argv[2])+1)*sizeof(char));
			strcpy(fileArg1Name, notesDirName);
			strcat(fileArg1Name, "/");
			strcat(fileArg1Name, argv[2]);
			strcat(fileArg1Name, ".txt");
			arg1File = fopen(fileArg1Name,"r+");
			fileArg2Name=malloc((strlen(notesDirName)+strlen(argv[3])+1)*sizeof(char));
			strcpy(fileArg2Name, notesDirName);
			strcat(fileArg2Name, "/");
			strcat(fileArg2Name, argv[3]);
			strcat(fileArg2Name, ".txt");
			arg2File = fopen(fileArg2Name,"r+");
			retVal = !!(arg1File == NULL);
			retVal = !!(arg2File == NULL);
			break;
	}
	return retVal;
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
	uint numOfFiles = 0;
	while ((entry = readdir(notesDir)) != NULL) {
		if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
			numOfFiles++;
		}
	}
	fileNames=malloc(numOfFiles*sizeof(char*));
	rewinddir(notesDir);
	uint i = 0;
	while ((entry = readdir(notesDir)) != NULL) {
		if (entry->d_type == DT_REG) {
			fileNames[i]=malloc(255*sizeof(char));
			strcpy(fileNames[i], entry->d_name);
			i++;
		}
		if (entry->d_type == DT_DIR) {
			fileNames[i]=malloc(255*sizeof(char));
			strcpy(fileNames[i], entry->d_name);
			strcat(fileNames[i], "/");
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
