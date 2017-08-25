#include "sr_error.h"

String SRError::get_error_string(SRError::Error err) {
	switch (err) {
		case OK:
			return "All is well :)";
		case MULTIBYTE_STR_ERR:
			return "Couldn't convert filename to a multibyte sequence";
		case MEMALLOC_ERR:
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
