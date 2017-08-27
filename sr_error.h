#ifndef SR_ERROR_H
#define SR_ERROR_H

#include "core/ustring.h"

// Shortcut for printing SRError::Error values with ERR_PRINT()
#define SRERR_PRINTS(err) ERR_PRINTS(SRError::get_error_string(err));

class SRError {

public:
	/*
	 * Defines error values that some speech recognition-related functions may
	 * return.
	 */
	enum Error {
		OK,                  // No error occurred
		UNDEF_FILES_ERR,     // One or more file/directory names aren't defined
		MULTIBYTE_STR_ERR,   // Couldn't convert filename to a multibyte sequence
		MEM_ALLOC_ERR,       // No memory available for allocation
		CONFIG_CREATE_ERR,   // Couldn't create Pocketsphinx configuration variable
		REC_CREATE_ERR,      // Couldn't open audio device (microphone)
		DECODER_CREATE_ERR,  // Couldn't create Sphinxbase decoder variable
		REC_START_ERR,       // Couldn't start recording user's voice
		REC_STOP_ERR,        // Couldn't stop recording user's voice
		UTT_START_ERR,       // Couldn't start utterance during speech recognition
		UTT_RESTART_ERR,     // Couldn't restart utterance during speech recognition
		AUDIO_READ_ERR       // Error while reading data from recorder
	};

	/*
	 * Returns a short String explaining the given Error value.
	 */
	static String get_error_string(Error err);
};

#endif
