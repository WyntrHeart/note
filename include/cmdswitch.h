#ifndef CMDSWITCH_INCLUDED
#define CMDSWITCH_INCLUDED

#include <stdint.h>

uint32_t getCmdID(char* inStr, int numOfChars) {
	/* Return value, an unsigned 32bit int packed with the last 5 bits of up to 6 chars.
	5 bits is enough to differentiate all letters of the alphabet, ignoring case.
	if cmdID == 0, the input was invalid. */
	uint32_t cmdID = 0;

	// Flag for input validity check
	int inputValid = 1;

	// limit numOfChars to 6 (written as a branchless conditional)
	numOfChars = numOfChars*(numOfChars<=6) + 6*(numOfChars>6);

	// Loop through the input string
	for (int i=0; i<numOfChars; i++) {
		// Set inputValid to 0 if char is not an ASCII letter
		inputValid = inputValid * !(
			inStr[i] < 'A'
			||
			inStr[i] > 'z'
			||
			(inStr[i]&31u) > ('Z'&31u) // Check gap between upper and lowercase letters
		);

		// Cast char to int
		uint32_t addBits = (uint32_t)inStr[i];

		// Zero all but the lowest 5 bits
		addBits = addBits & 31u;

		// Left shift by 5 bits per char offset
		addBits = addBits << (5*i);

		// Pack bits into cmdID
		cmdID = cmdID | addBits;
	}

	// Return cmdID or 0 if input was invalid
	return cmdID * inputValid; 
}

#endif
