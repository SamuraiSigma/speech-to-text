#include "sr_config.h"
#include "core/globals.h"         // Globals::get_singleton()->globalize_path()
#include "core/os/memory.h"       // memalloc(), memfree(), memdelete()
#include "core/os/dir_access.h"   // DirAccess::create_for_path()->dir_exists()
#include "core/os/file_access.h"  // FileAccess::create_for_path()->file_exists()

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

SRError::Error SRConfig::init() {
	if (hmm_dirname == "" || dict_filename == "" || kws_filename == "") {
		SRERR_PRINTS(SRError::UNDEF_FILES_ERR);
		return SRError::UNDEF_FILES_ERR;
	}

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
			SRERR_PRINTS(SRError::MEM_ALLOC_ERR);
			return SRError::MEM_ALLOC_ERR;
		}

		wcstombs(convert[i], names[i].c_str(), len + 1);

#ifdef DEBUG_ENABLED
		print_line("[SRConfig Argument] " + String(convert[i]));
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

void SRConfig::set_hmm_dirname(String hmm_dirname) {
	DirAccess *d = DirAccess::create_for_path(hmm_dirname);
	if (d->dir_exists(hmm_dirname))
		this->hmm_dirname = hmm_dirname;
	else {
		String err_msg = String("Directory '" + hmm_dirname + "' not found!");
		ERR_PRINT(err_msg.utf8().get_data());
	}
}

String SRConfig::get_hmm_dirname() {
	return hmm_dirname;
}

void SRConfig::set_dict_filename(String dict_filename) {
	FileAccess *f = FileAccess::create_for_path(dict_filename);
	if (f->file_exists(dict_filename))
		this->dict_filename = dict_filename;
	else {
		String err_msg = String("File '" + dict_filename + "' not found!");
		ERR_PRINT(err_msg.utf8().get_data());
	}
}

String SRConfig::get_dict_filename() {
	return dict_filename;
}

void SRConfig::set_kws_filename(String kws_filename) {
	FileAccess *f = FileAccess::create_for_path(kws_filename);
	if (f->file_exists(kws_filename))
		this->kws_filename = kws_filename;
	else {
		String err_msg = String("File '" + kws_filename + "' not found!");
		ERR_PRINT(err_msg.utf8().get_data());
	}
}

String SRConfig::get_kws_filename() {
	return kws_filename;
}

void SRConfig::_bind_methods() {
	ObjectTypeDB::bind_method("init",         &SRConfig::init);
	ObjectTypeDB::bind_method("get_recorder", &SRConfig::get_recorder);
	ObjectTypeDB::bind_method("get_decoder",  &SRConfig::get_decoder);

	ObjectTypeDB::bind_method("set_hmm_dirname",   &SRConfig::set_hmm_dirname);
	ObjectTypeDB::bind_method("get_hmm_dirname",   &SRConfig::get_hmm_dirname);
	ObjectTypeDB::bind_method("set_dict_filename", &SRConfig::set_dict_filename);
	ObjectTypeDB::bind_method("get_dict_filename", &SRConfig::get_dict_filename);
	ObjectTypeDB::bind_method("set_kws_filename",  &SRConfig::set_kws_filename);
	ObjectTypeDB::bind_method("get_kws_filename",  &SRConfig::get_kws_filename);

	ADD_PROPERTYNZ(PropertyInfo(Variant::STRING, "hmm directory", PROPERTY_HINT_DIR),
	               _SCS("set_hmm_dirname"), _SCS("get_hmm_dirname"));
	ADD_PROPERTYNZ(PropertyInfo(Variant::STRING, "dictionary file",
	                            PROPERTY_HINT_FILE, "dict"),
	               _SCS("set_dict_filename"), _SCS("get_dict_filename"));
	ADD_PROPERTYNZ(PropertyInfo(Variant::STRING, "keywords file",
	                            PROPERTY_HINT_FILE, "kws"),
	               _SCS("set_kws_filename"), _SCS("get_kws_filename"));
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
