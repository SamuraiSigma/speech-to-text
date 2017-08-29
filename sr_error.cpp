#include "sr_error.h"

// Defines a reference to SRError::singleton
SRError *SRError::singleton = NULL;

String SRError::get_error_string(SRError::Error err) {
	switch (err) {
		case OK:
			return "All is well :)";
		case UNDEF_FILES_ERR:
			return "One or more file/directory names aren't defined";
		case UNDEF_CONFIG_ERR:
			return "Undefined configuration variable (SRConfig)";
		case MULTIBYTE_STR_ERR:
			return "Couldn't convert filename to a multibyte sequence";
		case MEM_ALLOC_ERR:
			return "No memory available for allocation";
		case CONFIG_CREATE_ERR:
			return "Couldn't create Pocketsphinx configuration variable";
		case REC_CREATE_ERR:
			return "Couldn't open audio device (microphone)";
		case DECODER_CREATE_ERR:
			return "Couldn't create Sphinxbase decoder variable";
		case REC_START_ERR:
			return "Couldn't start recording user's voice";
		case REC_STOP_ERR:
			return "Couldn't stop recording user's voice";
		case UTT_START_ERR:
			return "Couldn't start utterance during speech recognition";
		case UTT_RESTART_ERR:
			return "Couldn't restart utterance during speech recognition";
		case AUDIO_READ_ERR:
			return "Error while reading data from recorder";
	}

	String err_number = itos((int64_t) err);  // Error -> int64_t -> String
	return "No corresponding String for error number " + err_number;
}

SRError * SRError::get_singleton() {
	return singleton;
}

void SRError::_bind_methods() {
	ObjectTypeDB::bind_method("get_error_string", &SRError::get_error_string);

	BIND_CONSTANT(OK);
	BIND_CONSTANT(UNDEF_FILES_ERR);
	BIND_CONSTANT(UNDEF_CONFIG_ERR);
	BIND_CONSTANT(MULTIBYTE_STR_ERR);
	BIND_CONSTANT(CONFIG_CREATE_ERR);
	BIND_CONSTANT(MEM_ALLOC_ERR);
	BIND_CONSTANT(REC_CREATE_ERR);
	BIND_CONSTANT(DECODER_CREATE_ERR);
	BIND_CONSTANT(REC_START_ERR);
	BIND_CONSTANT(REC_STOP_ERR);
	BIND_CONSTANT(UTT_START_ERR);
	BIND_CONSTANT(UTT_RESTART_ERR);
	BIND_CONSTANT(AUDIO_READ_ERR);
}

SRError::SRError() {
	singleton = this;
}
