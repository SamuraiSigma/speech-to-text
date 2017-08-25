#include "sr_config.h"
#include "core/os/memory.h"  // memalloc(), memfree(), memdelete()
#include "core/globals.h"    // Globals::get_singleton()->globalize_path()

/*
 * Adds the -adcdev option as a possible command line argument.
 * This argument corresponds to the microphone name.
 */
static arg_t cont_args_def[] = {
	POCKETSPHINX_OPTIONS,
	// Microphone
	{
		"-adcdev",
		ARG_STRING,
		NULL,  // Use default system microphone
		"Name of audio device to use for input."
	},
	CMDLN_EMPTY_OPTION
};

SRError::Error SRConfig::init(String hmm_dirname, String dict_filename,
                              String kws_filename) {
	this->hmm_dirname   = hmm_dirname;
	this->dict_filename = dict_filename;
	this->kws_filename  = kws_filename;

	String names[3];
	names[0] = hmm_dirname;
	names[1] = dict_filename;
	names[2] = kws_filename;

	char *convert[3];

	// Convert each filename: String -> wchar_t * -> char *
	for (int i = 0; i < 3; i++) {
		// Get path without "res://" stuff
		names[i] = Globals::get_singleton()->globalize_path(names[i]);

		int len = wcstombs(NULL, names[i].c_str(), 0);
		if (len == -1) {
			SRERR_PRINTS(SRError::MULTIBYTE_STR_ERR);
			return SRError::MULTIBYTE_STR_ERR;
		}

		convert[i] = (char *) memalloc((len + 1) * sizeof(char));
		if (convert[i] == NULL) {
			SRERR_PRINTS(SRError::MEMALLOC_ERR);
			return SRError::MEMALLOC_ERR;
		}

		wcstombs(convert[i], names[i].c_str(), len + 1);

#ifdef DEBUG_ENABLED
		print_line("[SpeechRecognizer Argument] " + String(convert[i]));
#endif
	}

	hmm  = convert[0];
	dict = convert[1];
	kws  = convert[2];

	// Create configuration variable
	conf = cmd_ln_init(NULL, ps_args(), TRUE,
	                   "-hmm", hmm,
	                   "-dict", dict,
	                   "-kws", kws,
	                   NULL);

	if (conf == NULL) {
		SRERR_PRINTS(SRError::CONFIG_CREATE_ERR);
		return SRError::CONFIG_CREATE_ERR;
	}

	// Update basic configuration with custom one for mic
	conf = cmd_ln_init(conf, cont_args_def, TRUE, NULL);
	if (conf == NULL) {
		SRERR_PRINTS(SRError::CONFIG_CREATE_ERR);
		return SRError::CONFIG_CREATE_ERR;
	}

	// Create recorder variable
	recorder = ad_open_dev(cmd_ln_str_r(conf, "-adcdev"),
	                       (int) cmd_ln_float32_r(conf, "-samprate"));

	if (recorder == NULL) {
		cmd_ln_free_r(conf);
		SRERR_PRINTS(SRError::REC_CREATE_ERR);
		return SRError::REC_CREATE_ERR;
	}

	// Creater decoder variable
	decoder = ps_init(conf);

	if (decoder == NULL) {
		cmd_ln_free_r(conf);
		ad_close(recorder);
		SRERR_PRINTS(SRError::DECODER_CREATE_ERR);
		return SRError::DECODER_CREATE_ERR;
	}

	return SRError::OK;
}

ad_rec_t * SRConfig::get_recorder() {
	return recorder;
}

ps_decoder_t * SRConfig::get_decoder() {
	return decoder;
}

int SRConfig::get_rec_buffer_size() {
	return rec_buffer_size;
}

void SRConfig::set_rec_buffer_size(int rec_buffer_size) {
	if (rec_buffer_size <= 0) {
		fprintf(stderr, "Microphone recorder buffer size must be greater than 0\n");
		return;
	}
	this->rec_buffer_size = rec_buffer_size;
}

void SRConfig::_bind_methods() {
	ObjectTypeDB::bind_method("init",         &SRConfig::init);
	ObjectTypeDB::bind_method("get_recorder", &SRConfig::get_recorder);
	ObjectTypeDB::bind_method("get_decoder",  &SRConfig::get_decoder);

	ObjectTypeDB::bind_method("get_rec_buffer_size", &SRConfig::get_rec_buffer_size);
	ObjectTypeDB::bind_method("set_rec_buffer_size", &SRConfig::set_rec_buffer_size);
}

SRConfig::SRConfig() {
	// Disable Pocketphinx log output
	err_set_logfp(NULL);

	conf = NULL;
	recorder = NULL;
	decoder = NULL;

	hmm_dirname   = "";
	dict_filename = "";
	kws_filename  = "";

	hmm = NULL;
	dict = NULL;
	kws = NULL;
}

SRConfig::~SRConfig() {
	if (recorder != NULL) ad_close(recorder);
	if (decoder  != NULL) ps_free(decoder);
	if (conf     != NULL) cmd_ln_free_r(conf);

	if (hmm  != NULL) memfree(hmm);
	if (dict != NULL) memfree(dict);
	if (kws  != NULL) memfree(kws);
}
