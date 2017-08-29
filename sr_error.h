#ifndef SR_ERROR_H
#define SR_ERROR_H

#include "core/object.h"
#include "core/ustring.h"

// Shortcut for printing SRError::Error values with ERR_PRINT()
#define SRERR_PRINTS(e) ERR_PRINTS(SRError::get_singleton()->get_error_string(e));

class SRError : public Object {
	OBJ_TYPE(SRError, Object);

public:
	/*
	 * Defines error values that some speech recognition-related functions may
	 * return.
	 */
	enum Error {
		OK,                  // No error occurred
		UNDEF_FILES_ERR,     // One or more file/directory names aren't defined
		UNDEF_CONFIG_ERR,    // Undefined configuration variable (SRConfig)
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

protected:
	// Singleton for this class's only instance.
	static SRError *singleton;

	/*
	 * Needed so that GDScript can recognize public methods and the enum from this
	 * class.
	 */
	static void _bind_methods();

public:
	/*
	 * Returns this class's only instance (or NULL, if it wasn't instanced yet).
	 */
	static SRError * get_singleton();

	/*
	 * Returns a short String explaining the given Error value.
	 */
	String get_error_string(Error err);

	/*
	 * Initializes the class's singleton.
	 */
	SRError();

	/*
	 * Doesn't actually do anything. :P
	 */
	~SRError();
};

// Makes the enum work when binding to methods
VARIANT_ENUM_CAST(SRError::Error);

#endif  // SR_ERROR_H
