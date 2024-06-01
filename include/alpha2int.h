#ifndef INCLUDED_ALPHA2INT
#define INCLUDED_ALPHA2INT

#include <string.h>

// This function packs the last five bits of up to six alphabetical chars into an unsigned integer.
// The intention is to allow quick parsing of subcommand arguments using switch statements
unsigned int alpha2int(char* str) {
	unsigned int length = strlen(str);
	unsigned int retVal = 0;
	unsigned int validChar = 1;
	for (unsigned int i = 0; i < length; i++) {
		// "|32u" is a bitmask that converts letters to all lowercase.
		// Then we check if the value is outside the range of a-z.
		// Invert so that out of range = false
		// Binary and the result with validChar to unset the 1 bit if the character is invalid
		validChar = validChar & !( ((str[i]|32u) < 'a') || ((str[i]|32u) > 'z') );
		// "&31u" is bitmask that zeroes all but the first 5 bits
		retVal = (retVal | ((str[i]&31u) << i*5));
	}
	// Zero the return value if str is too big or contains non-alphabetical characters
	return retVal * validChar * !(length > (sizeof(unsigned int)*8/5));
}
#endif
