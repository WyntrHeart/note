#if (defined(_WIN32) || defined(__WIN32__))
#include <windows.h>
#define realpath(path, resolved_path) _fullpath(resolved_path, path, _MAX_PATH)
#define mkdir(A, B) mkdir(A)
#else 
#include <unistd.h>
#endif
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "./include/cmdswitch.h"
#include "./include/wynutils.h"

// values precomputed by getCmdID() for the given command strings
#define CMD_HELP    536744u
#define CMD_H       8u
#define CMD_SHOW    769299u
#define CMD_LS      620u
#define CMD_ADD     4225u
#define CMD_A       1u
#define CMD_EDIT    664709u
#define CMD_E       5u
#define CMD_CLEAR   18912643u
#define CMD_CLR     18819u
#define CMD_WHERE   5838103u
#define CMD_W       23u
#define CMD_RM      434u
#define CMD_CP      515u
#define CMD_MV      717u
#define CMD_MKDIR   19173741u
#define CMD_RMDIR   19173810u

// Function prototype declarations
// Main subroutines
int    cmdAdd   (int argc, char** argv); // Append string to a note
int    cmdClr   (int argc, char** argv); // Erase contents of note
int    cmdCp    (int argc, char** argv); // Copy note
int    cmdEdit  (int argc, char** argv); // Open note with $EDITOR
void   cmdHelp  ();                      // Print usage info
int    cmdLs    (int argc, char** argv); // List notes and subdirs in directory
int    cmdMv    (int argc, char** argv); // Move/rename note
int    cmdMkdir (int argc, char** argv); // Make note subdir
int    cmdRm    (int argc, char** argv); // Delete note
int    cmdRmdir (int argc, char** argv); // Delete note subdir
int    cmdShow  (int argc, char** argv); // Print contents of note
int    cmdWhere (int argc, char** argv); // Print full path of note
// Other functions
char* dirNameToPath   (char* dirName, char* notesDirPath);  // Convert directory argument to path
int   exitIfAllocFail (void* checkPtr);                     // Error and exit if ptr is null
char* initNotesDir    ();                                   // Find or create note directory
char* noteNameToPath  (char* noteName, char* notesDirPath); // Convert note name argument to path


// Entry point
int main(int argc, char** argv) {
	// Function data
	unsigned int subCmdID = CMD_HELP;

	// Check argument count
	if ( argc > 4 ) {
		fputs("ERROR: Too many arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	else if ( argc > 1 ) {
		// Calculate cmdID of second argument for switch statement
		subCmdID = getCmdID(argv[1], strlen(argv[1]));
	}

	// Select subroutine based on first argument
	switch (subCmdID) {
		// Append a string to a note
		case CMD_ADD:
		case CMD_A:
			return cmdAdd(argc, argv);
		break;
		// Erase contents of note
		case CMD_CLEAR:
		case CMD_CLR:
			return cmdClr(argc, argv);
		break;
		// Copy note
		case CMD_CP:
			return cmdCp(argc, argv);
		break;
		// Open note in $EDITOR
		case CMD_EDIT:
		case CMD_E:
			return cmdEdit(argc, argv);
		break;
		// Print usage info
		case CMD_HELP:
		case CMD_H:
			cmdHelp();
			return EXIT_SUCCESS;
		break;
		// List notes and subdirs in directory
		case CMD_LS:
			return cmdLs(argc, argv);
		break;
		// Make dir in notes
		case CMD_MKDIR:
			return cmdMkdir(argc, argv);
		break;
		// Move/rename note
		case CMD_MV:
			return cmdMv(argc, argv);
		break;
		// Delete note
		case CMD_RM:
			return cmdRm(argc, argv);
		break;
		// Remove dir from notes
		case CMD_RMDIR:
			return cmdRmdir(argc, argv);
		break;
		case CMD_SHOW:
			return cmdShow(argc, argv);
		break;
		// Print path to note file
		case CMD_WHERE:
		case CMD_W:
			return cmdWhere(argc, argv);
		break;
		// First argument isn't a subcommand. Is it a file?
		default:
			return cmdShow(argc, argv);
		break;
	}
	// This line should never be reached
	return -1;
}


// Append string to end of note
int cmdAdd(int argc, char** argv) {
	if ( argc != 4 ) {
		fputs("ERROR: Invalid arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* notesDirPath = initNotesDir();
	char* filePath = noteNameToPath(argv[2], notesDirPath);
	char* addString = malloc( (strlen(argv[3])+1) * sizeof(char) );
	exitIfAllocFail(addString);
	strcpy(addString, argv[3]);
	return appendToFile(filePath, addString);
}


// Erase contents of note
int cmdClr(int argc, char** argv) {
	if ( argc != 3 ) {
		fputs("ERROR: Invalid arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* notesDirPath = initNotesDir();
	char* filePath = noteNameToPath(argv[2], notesDirPath);
	if ( fclose(fopen(filePath, "w")) != 0 ) {
		int clrErrno = errno;
		fprintf(
			stderr,
			"ERROR: Failed to clear note '%s': %s\n",
			argv[2],
			strerror(clrErrno)
		);
		return clrErrno;
	}
	else return EXIT_SUCCESS;
}


// Copy note
int cmdCp(int argc, char** argv) {
	if ( argc != 4 ) {
		fputs("ERROR: Invalid arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* notesDirPath = initNotesDir();
	char* srcFilePath = noteNameToPath(argv[2], notesDirPath);
	char* destFilePath = noteNameToPath(argv[3], notesDirPath);
	return cp(srcFilePath, destFilePath);
}


// Open note in $EDITOR
int cmdEdit(int argc, char** argv) {
	if ( argc != 3 ) {
		fputs("ERROR: Invalid arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* notesDirPath = initNotesDir();
	char* filePath = noteNameToPath(argv[2], notesDirPath);
	int editErrno = editFile(filePath);
	// If editFile returns -1 then $EDITOR isn't set
	if ( editErrno == -1 ) fputs(
		"ERROR: Environment variable 'EDITOR' is not set\n",
		stderr
	);
	return editErrno;
}


// Print usage info
void cmdHelp() {
	puts(
	"usage: note NOTE|SUBCOMMAND [ARG1 [ARG2]]\n"
	"    Display NOTE or do SUBCOMMAND. Specify notes by name only, 'note' handles\n"
	"    file extensions internally.\n"
	"subcommands:\n"
	"    help|h     Display this help text\n"
	"    ls         List notes and subdirs in note directory. If ARG1 is a\n"
	"               subdir list notes/subdirs inside it.\n"
	"    add|a      Append the string ARG2 to a new line at the end of ARG1\n"
	"    edit|e     Open ARG1 with $EDITOR\n"
	"    clear|clr  Erase contents of ARG1\n"
	"    where|w    Print full path to ARG1 (or the note dir if ARG1 is omitted)\n"
	"    rm         Remove ARG1\n"
	"    cp         Copy ARG1 to ARG2\n"
	"    mv         Move/rename ARG1 to ARG2\n"
	"    mkdir      Create subdir named ARG1\n"
	"    rmdir      Remove subdir named ARG1"
	);
	return;
}


// List .txt files and subdirectories 
int cmdLs(int argc, char** argv) {
	if ( argc > 3 ) {
		fputs("ERROR: Too many arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char*  dirPath      = NULL;
	char   fileExt[5]   = "";
	int    isTxtFile    = 0;
	int    lsErrno      = 0;
	struct lsStruct lsOutput;
	char*  notesDirPath = initNotesDir();
	struct stat statBuf;
	char*  statFilePath = NULL;
	if ( argc < 3 ) {
		dirPath = notesDirPath;
	}
	else {
		dirPath = dirNameToPath(argv[2], notesDirPath);
	}
	lsErrno = ls(dirPath, &lsOutput, 0);
	if ( lsErrno != 0 ) {
		fprintf(
			stderr,
			"ERROR: Failed to list directory '%s': %s\n",
			argv[2],
			strerror(lsErrno)
		);
		return lsErrno;
	}
	// Filter the file and directory names by type
	for (int i = 0; i < lsOutput.numOfFiles; i++) {
		// Check last four letters of file name to identify .txt files
		sprintf(
			fileExt,
			"%4s",
			lsOutput.fileNames[i]+strlen(lsOutput.fileNames[i])-4
		);
		isTxtFile = !strcmp(fileExt, ".txt");
		// Construct full file path for stat() to check if name is a directory
		statFilePath = realloc(
			statFilePath,
			( strlen(dirPath)+strlen(lsOutput.fileNames[i])+2 ) * sizeof(char)
		);
		sprintf(
			statFilePath,
			"%s/%s",
			dirPath,
			lsOutput.fileNames[i]
		);
		stat(statFilePath, &statBuf);
		// Hide "." and ".."
		if ( !strcmp(lsOutput.fileNames[i], ".") || !strcmp(lsOutput.fileNames[i], "..") ) {
			free(lsOutput.fileNames[i]);
			lsOutput.fileNames[i] = NULL;
		}
		// Append a "/" to the names of directories
		else if ( (statBuf.st_mode & S_IFMT) == S_IFDIR ) {
			lsOutput.fileNames[i] = realloc(
				lsOutput.fileNames[i],
				(strlen(lsOutput.fileNames[i])+2) * sizeof(char)
			);
			exitIfAllocFail(lsOutput.fileNames[i]);
			strcat(lsOutput.fileNames[i], "/");
			isTxtFile = 0; // In case a dir name ends with ".txt" for some reason
		}
		// Hide extension on note names
		else if (isTxtFile) {
			lsOutput.fileNames[i][strlen(lsOutput.fileNames[i])-4]='\0';
		}
		// Hide anything that isn't .txt or a dir
		else {
			free(lsOutput.fileNames[i]);
			lsOutput.fileNames[i] = NULL;
		}
	}
	// Print file names
	for (int i=0; i < lsOutput.numOfFiles; i++) {
		if (lsOutput.fileNames[i]) {
			puts(lsOutput.fileNames[i]);
		}
	}
	return EXIT_SUCCESS;
}


// Make note subdir
int cmdMkdir(int argc, char** argv) {
	if ( argc != 3 ) {
		fputs("ERROR: Invalid arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* notesDirPath = initNotesDir();
	char* dirPath = dirNameToPath(argv[2], notesDirPath);
	if ( mkdir(dirPath, 0755) != 0 ) {
		fprintf(
			stderr,
			"ERROR: Failed to create directory '%s': %s\n",
			argv[2],
			strerror(errno)
		);
		return errno;
	}
	return EXIT_SUCCESS;
}


// Move/rename note
int cmdMv(int argc, char** argv) {
	if ( argc != 4 ) {
		fputs("ERROR: Invalid arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* notesDirPath = initNotesDir();
	char* srcFilePath = noteNameToPath(argv[2], notesDirPath);
	char* destFilePath = noteNameToPath(argv[3], notesDirPath);
	int mvErrno = rename(srcFilePath, destFilePath);
	if ( mvErrno != 0 ) {
		fprintf(
			stderr,
			"ERROR: Failed to move note '%s': %s\n",
			argv[2],
			strerror(mvErrno)
		);
		return mvErrno;
	}
	else return EXIT_SUCCESS;
}


// Delete note
int cmdRm(int argc, char** argv) {
	if ( argc != 3 ) {
		fputs("ERROR: Invalid arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* notesDirPath = initNotesDir();
	char* filePath = noteNameToPath(argv[2], notesDirPath);
	if ( remove(filePath) != 0 ) {
		fprintf(
			stderr,
			"ERROR: Failed to delete note '%s': %s\n",
			argv[2],
			strerror(errno)
		);
		return errno;
	}
	else return EXIT_SUCCESS;
}


// Delete note subdir
int cmdRmdir(int argc, char** argv) {
	if ( argc != 3 ) {
		fputs("ERROR: Invalid arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* notesDirPath = initNotesDir();
	char* dirPath = dirNameToPath(argv[2], notesDirPath);
	if ( rmdir(dirPath) != 0 ) {
		fprintf(
			stderr,
			"ERROR: Failed to remove directory '%s': %s\n",
			argv[2],
			strerror(errno)
		);
		return errno;
	}
	else return EXIT_SUCCESS;
}


// Print contents of note
int cmdShow(int argc, char** argv) {
	if ( argc > 3 ) {
		fputs("ERROR: Too many arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* noteName = NULL;
	char* notesDirPath = initNotesDir();
	char* filePath = NULL;
	char* fileContent = NULL;
	// Use argv[1] if invoked as "note <NOTE>"
	if ( argc == 2 ) {
		noteName=argv[1];
	}
	// Use argv[2] if invoked as "note show <NOTE>"
	else {
		noteName=argv[2];
	}
	filePath = noteNameToPath(noteName, notesDirPath);
	int loadErrno = loadFile(&fileContent, filePath);
	if ( fileContent != NULL ) fputs(fileContent, stdout);
	else fprintf(
		stderr,
		"ERROR: Failed to load note '%s': %s\n",
		noteName,
		strerror(loadErrno)
	);
	return loadErrno;
}


// Print full path of note
int cmdWhere(int argc, char** argv) {
	if ( argc > 3 ) {
		fputs("ERROR: Too many arguments\n", stderr);
		cmdHelp();
		return EINVAL;
	}
	char* notesDirPath = initNotesDir();
	struct stat statBuf;
	if ( argc == 2 ) {
		puts(notesDirPath);
		return EXIT_SUCCESS;
	}
	if ( argc == 3 ) {
		// We need to check if argv[2] is a note name or a directory
		char* filePath = noteNameToPath(argv[2], notesDirPath);
		char* dirPath = dirNameToPath(argv[2], notesDirPath);
		// Test it as a note first
		if ( stat(filePath, &statBuf) == 0 ) {
			puts(filePath);
			return EXIT_SUCCESS;
		}
		// Test as a directory if no note is found
		if ( stat(dirPath, &statBuf) == 0 ) {
			printf("%s/\n", dirPath);
			return EXIT_SUCCESS;
		}
		// If neither works, report failure
		fprintf(
			stderr,
			"ERROR: Could not find note or path named '%s'\n",
			argv[2]
		);
		return EXIT_FAILURE;
	}
	// This line should never be reached
	return EXIT_FAILURE;
}


// Convert directory name to full directory path
char* dirNameToPath(char* dirName, char* notesDirPath) {
	// Allocate space for "<notesDirPath>/<dirName>"
	char* dirPath = malloc( (strlen(notesDirPath)+strlen(dirName)+2) * sizeof(char) );
	exitIfAllocFail(dirPath);
	sprintf(dirPath, "%s/%s", notesDirPath, dirName);
	return dirPath;
}


// Print error message and exit the program if pointer is null
int exitIfAllocFail(void* checkPtr) {
	if ( checkPtr == NULL ) {
		perror("ERROR: Failed to allocate memory");
		exit(errno);
	}
	return EXIT_SUCCESS;
}


// Locate notes directory
char* initNotesDir() {
	char* notesDirPath = getenv("NOTE_DIR");
	struct stat statBuf;
	if (notesDirPath == NULL) {
		char* homeDirPath = getenv("HOME");
		#ifndef _WIN32
		if (homeDirPath == NULL) {
			fputs("ERROR: Environment variable 'HOME' not set\n", stderr);
			exit(-1);
		}
		#endif
		#ifdef _WIN32
		if (homeDirPath == NULL) {
			homeDirPath = getenv("USERPROFILE");
		}
		if (homeDirPath == NULL) {
			fputs("ERROR: Environment variables 'HOME' and 'USERPROFILE' not set", stderr);
			exit(-1);
		}
		#endif
		// Allocate space for "<homeDirPath>/Documents/notes"
		notesDirPath = malloc( (strlen(homeDirPath)+17) * sizeof(char) );
		strcpy(notesDirPath, homeDirPath);
		#ifdef _WIN32
		// Convert Windows backslashes to normal forward slashes
		for (int i = 0; i <= strlen(notesDirPath); i++) {
			if (notesDirPath[i] == '\\') {
				notesDirPath[i] = '/';
			}
		}
		#endif
		strcat(notesDirPath, "/Documents/notes");
	}

	// Check if notes dir exists
	if ( stat(notesDirPath, &statBuf) != 0 ) {
		// Create it if it doesn't
		int mkdirErrno = mkdir(notesDirPath, 775);
		if ( mkdirErrno != 0 ) {
			fprintf(
				stderr,
				"ERROR: Directory '%s' does not exist and cannot be created: %s\n",
				notesDirPath,
				strerror(mkdirErrno)
			);
			exit(mkdirErrno);
		}
	}

	return notesDirPath;
}


// Convert note name to full file path
char* noteNameToPath(char* noteName, char* notesDirPath) {
	// Allocate space for "<notesDirPath>/<noteName>.txt"
	char* filePath = malloc( (strlen(notesDirPath)+strlen(noteName)+6) * sizeof(char) );
	exitIfAllocFail(filePath);
	sprintf(filePath, "%s/%s.txt", notesDirPath, noteName);
	return filePath;
}
