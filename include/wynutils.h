#ifndef WYNUTILS_INCLUDED
#define WYNUTILS_INCLUDED

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Structs
struct lsStruct {
    char** fileNames;
    int numOfFiles;
};

// Function prototypes
int appendToFile    (char* filePath, char* addString);                   // Add string to new line at end of file
int cp              (char* srcFilePath, char* destFilePath);             // Copy a file
int editFile        (char* filePath);                                    // Open file with $EDITOR
int loadFile        (char** fileContent, char* filePath);                 // Load file contents into memory
int ls              (char* dirPath, struct lsStruct* output, int flags); // List files in directory
int qsortStrcmp     (const void* a, const void* b);                      // Wraps strcmp() for qsort()


// Add string to new line at end of file
int appendToFile(char* filePath, char* addString) {
	FILE* file = NULL;
	int firstChar = '\0'; // int to fit EOF, which is wider than char
	char lastChar = '\0';

	// Clear errno
	errno = 0;

	// Open file
	file = fopen(filePath, "a+");
	if ( file == NULL ) return errno;

	// Check if file is empty
	fseek(file, 0, SEEK_SET);
	firstChar = fgetc(file);

	// Check for trailing newline
	fseek(file, -1, SEEK_END);
	lastChar = fgetc(file);

	// Insert string at end of file in it's own line
	fseek(file, 0, SEEK_END);
	if ( lastChar == '\n' || firstChar == EOF ) {
		fprintf(file, "%s\n", addString);
	}
	else {
		fprintf(file, "\n%s\n", addString);
	}

	return errno;
}


// Copy a file
int cp(char* srcFilePath, char* destFilePath) {
	int charBuf = '\0'; // Character buffer to copy contents of file. int instead of char to fit EOF
	FILE* srcFile = NULL;
	FILE* destFile = NULL;
	struct stat statBuf;

	// Clear errno
	errno = 0;

	// Check that source file exists and destination file doesn't
	if ( stat(srcFilePath, &statBuf) != 0 ) return errno;
	if ( stat(destFilePath, &statBuf) == 0 ) return EXIT_FAILURE;

	// Open source and destination files
	srcFile = fopen(srcFilePath, "r");
	if ( srcFile == NULL ) return errno;
	destFile = fopen(destFilePath, "w");
	if ( destFile == NULL ) return errno;
	
	// Copy source to destination
	while ( (charBuf = fgetc(srcFile)) != EOF ) {
		fputc(charBuf, destFile);
	}
	if ( ferror(srcFile) || ferror(destFile) ) return errno;

	// Close the files
	fclose(srcFile);
	fclose(destFile);

	return errno;
}


// Open file with $EDITOR
int editFile(char* filePath) {
	// Clear errno
	errno = 0;

	// Get editor from environment
	char* editor = getenv("EDITOR");
	if (editor == NULL) {
		return -1;
	}

	// Construct edit command
	char* editCmdStr = malloc(
		( strlen(editor)+strlen(filePath)+4 ) * sizeof(char)
	);
	sprintf(editCmdStr, "%s \"%s\"", editor, filePath);
	
	// Run editor
	return system(editCmdStr);
}


// List files in directory
int ls(char* dirPath, struct lsStruct* output, int flags) {
	DIR*           dir           = NULL;
	struct dirent* entry         = NULL;

	// Clear errno
	errno = 0;

    // Initialize output struct
    output->fileNames = NULL;
    output->numOfFiles = 0;

	// Open the directory to list
	dir = opendir(dirPath);
	if ( dir == NULL ) {
		return errno;
	}

	// Count files so we can allocate enough pointers for the file names
	while ( (entry = readdir(dir)) != NULL ) output->numOfFiles++;
	output->fileNames = malloc( output->numOfFiles * sizeof(char*) );
    if ( output->fileNames == NULL ) {
        return errno;
    }

	// Reread the dir to record the names
	rewinddir(dir);
	int i = 0;
	while ( (entry = readdir(dir)) != NULL ) {
		if (entry) {
			output->fileNames[i]=malloc( (strlen(entry->d_name)+1) * sizeof(char) );
			strcpy( output->fileNames[i], entry->d_name );
			i++;
		}
	}

	// Sort the file names alphabetically
	qsort(
		output->fileNames,
		output->numOfFiles,
		sizeof(char*),
		qsortStrcmp
	);
    
    return 0;
}


// Load file contents into memory
int loadFile(char** fileContentPtr, char* filePath) {
	// Clear errno
	errno = 0;

	FILE* file = fopen(filePath, "r");
	if ( file == NULL ) return errno;
	unsigned long length = 0;
	
	// Get length of file so we can allocate a buffer
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	*fileContentPtr = realloc(*fileContentPtr, length+1);
	if ( *fileContentPtr == NULL ) {
		return errno;
	}

	// Seek back to beginning of file
	fseek(file, 0, SEEK_SET);

	// Read file to memory byte by byte, discarding CR bytes when found
	for ( unsigned long i = 0; i < length; i++ ) {
		(*fileContentPtr)[i] = (char)fgetc(file);
		if ( (*fileContentPtr)[i] == '\r' ) {
			length--;
			(*fileContentPtr)[i] = (char)fgetc(file);
		}
		if ( ferror(file) != 0 ) return errno;
	}

	// Ensure string ends with a null byte at the correct location
	(*fileContentPtr)[length] = '\0';

	fclose (file);
	return errno;
}


// Wraps strcmp() for qsort()
int qsortStrcmp(const void* a, const void* b) {
	const char* strA = *(const char**)a;
	const char* strB = *(const char**)b;
	return strcmp(strA, strB);
}
#endif
