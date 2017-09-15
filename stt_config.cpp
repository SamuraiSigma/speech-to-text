#include "stt_config.h"
#include "core/os/os.h"           // OS::get_singleton()->get_data_dir()
#include "core/os/memory.h"       // memalloc(), memfree(), memdelete()
#include "core/os/dir_access.h"   // DirAccess::exists(), open(), copy()
#include "core/os/file_access.h"  // FileAccess::exists()

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

STTError::Error STTConfig::init() {
	if (hmm_dirname == "" || dict_filename == "" || kws_filename == "") {
		STT_ERR_PRINTS(STTError::UNDEF_FILES_ERR);
		return STTError::UNDEF_FILES_ERR;
	}

	_copy_dir_to_user_stt(hmm_dirname);
	_copy_file_to_user_stt(dict_filename);
	_copy_file_to_user_stt(kws_filename);

#ifdef DEBUG_ENABLED
	print_line("[STTConfig HMM directory] '" + hmm_dirname + "'");
	print_line("[STTConfig dictionary file] '" + dict_filename + "'");
	print_line("[STTConfig keywords file] '" + kws_filename + "'");
#endif

	String names[3];
	names[0] = hmm_dirname;
	names[1] = dict_filename;
	names[2] = kws_filename;

	char *convert[3];

	// Convert each filename: String -> wchar_t * -> char *
	for (int i = 0; i < 3; i++) {
		names[i] = _convert_to_data_path(names[i]);

		int len = wcstombs(NULL, names[i].c_str(), 0);
		if (len == -1) {
			STT_ERR_PRINTS(STTError::MULTIBYTE_STR_ERR);
			return STTError::MULTIBYTE_STR_ERR;
		}

		convert[i] = (char *) memalloc((len + 1) * sizeof(char));
		if (convert[i] == NULL) {
			STT_ERR_PRINTS(STTError::MEM_ALLOC_ERR);
			return STTError::MEM_ALLOC_ERR;
		}

		wcstombs(convert[i], names[i].c_str(), len + 1);
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
		STT_ERR_PRINTS(STTError::CONFIG_CREATE_ERR);
		return STTError::CONFIG_CREATE_ERR;
	}

	// Update basic configuration with custom one for mic
	conf = cmd_ln_init(conf, cont_args_def, TRUE, NULL);
	if (conf == NULL) {
		STT_ERR_PRINTS(STTError::CONFIG_CREATE_ERR);
		return STTError::CONFIG_CREATE_ERR;
	}

	// Create recorder variable
	recorder = ad_open_dev(cmd_ln_str_r(conf, "-adcdev"),
	                       (int) cmd_ln_float32_r(conf, "-samprate"));

	if (recorder == NULL) {
		cmd_ln_free_r(conf);
		STT_ERR_PRINTS(STTError::REC_CREATE_ERR);
		return STTError::REC_CREATE_ERR;
	}

	// Creater decoder variable
	decoder = ps_init(conf);

	if (decoder == NULL) {
		cmd_ln_free_r(conf);
		ad_close(recorder);
		STT_ERR_PRINTS(STTError::DECODER_CREATE_ERR);
		return STTError::DECODER_CREATE_ERR;
	}

	return STTError::OK;
}

void STTConfig::set_hmm_dirname(String hmm_dirname) {
	if (DirAccess::exists(hmm_dirname))
		this->hmm_dirname = hmm_dirname;
	else
		ERR_PRINTS("Directory '" + hmm_dirname + "' not found!");
}

String STTConfig::get_hmm_dirname() {
	return hmm_dirname;
}

void STTConfig::set_dict_filename(String dict_filename) {
	if (FileAccess::exists(dict_filename))
		this->dict_filename = dict_filename;
	else
		ERR_PRINTS("File '" + dict_filename + "' not found!");
}

String STTConfig::get_dict_filename() {
	return dict_filename;
}

void STTConfig::set_kws_filename(String kws_filename) {
	if (FileAccess::exists(kws_filename))
		this->kws_filename = kws_filename;
	else
		ERR_PRINTS("File '" + kws_filename + "' not found!");
}

String STTConfig::get_kws_filename() {
	return kws_filename;
}

String STTConfig::_convert_to_data_path(String filename) {
	String user_path = OS::get_singleton()->get_data_dir();
	String basename = filename.get_file();
	return user_path.plus_file(STT_USER_DIRNAME).plus_file(basename);
}

bool STTConfig::_copy_file_to_user_stt(String filename) {
	String user_dirname = "user://" + String(STT_USER_DIRNAME);
	DirAccess *duser = DirAccess::open(user_dirname);

	String basename = filename.get_file();
	String user_filename = user_dirname.plus_file(basename);

	if (duser->copy(filename, user_filename) != OK) {
		print_line("Couldn't copy '" + filename + "' to '" + user_filename + "'");
		memdelete(duser);
		return false;
	}

	memdelete(duser);
	return true;
}

bool STTConfig::_copy_dir_to_user_stt(String dirname) {
	String user_dirname = "user://" + String(STT_USER_DIRNAME);
	String dir_basename = dirname.get_file();

	DirAccess *duser = DirAccess::open(user_dirname);
	DirAccess *dres = DirAccess::open(dirname);

	duser->make_dir(dir_basename);

	// Copy each file in dirname folder to STT user:// directory
	dres->list_dir_begin();
	String filename = dres->get_next();
	while (filename != "") {
		if (filename == "." || filename == "..") {
			filename = dres->get_next();
			continue;
		}

		String from = dirname.plus_file(filename);
		String to = user_dirname.plus_file(dir_basename)
		                        .plus_file(filename.get_file());

		if (duser->copy(from, to) != OK) {
			print_line("Couldn't copy '" + from + "' to '" + to + "'");
			memdelete(dres);
			memdelete(duser);
			return false;
		}

		filename = dres->get_next();
	}

	memdelete(dres);
	memdelete(duser);
	return true;
}

void STTConfig::_bind_methods() {
	ObjectTypeDB::bind_method("init", &STTConfig::init);

	ObjectTypeDB::bind_method(_MD("set_hmm_dirname", "hmm_dirname"),
	                          &STTConfig::set_hmm_dirname);
	ObjectTypeDB::bind_method("get_hmm_dirname", &STTConfig::get_hmm_dirname);

	ObjectTypeDB::bind_method(_MD("set_dict_filename", "dict_filename"),
	                          &STTConfig::set_dict_filename);
	ObjectTypeDB::bind_method("get_dict_filename", &STTConfig::get_dict_filename);

	ObjectTypeDB::bind_method(_MD("set_kws_filename", "kws_filename"),
	                          &STTConfig::set_kws_filename);
	ObjectTypeDB::bind_method("get_kws_filename", &STTConfig::get_kws_filename);

	ADD_PROPERTYNZ(PropertyInfo(Variant::STRING, "hmm directory", PROPERTY_HINT_DIR),
	               _SCS("set_hmm_dirname"), _SCS("get_hmm_dirname"));
	ADD_PROPERTYNZ(PropertyInfo(Variant::STRING, "dictionary file",
	                            PROPERTY_HINT_FILE, "dict"),
	               _SCS("set_dict_filename"), _SCS("get_dict_filename"));
	ADD_PROPERTYNZ(PropertyInfo(Variant::STRING, "keywords file",
	                            PROPERTY_HINT_FILE, "kws"),
	               _SCS("set_kws_filename"), _SCS("get_kws_filename"));
}

STTConfig::STTConfig() {
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

	// Create STT directory in user://
	DirAccess *da = DirAccess::open("user://");
	da->make_dir(STT_USER_DIRNAME);
	memdelete(da);
}

STTConfig::~STTConfig() {
	if (recorder != NULL) ad_close(recorder);
	if (decoder  != NULL) ps_free(decoder);
	if (conf     != NULL) cmd_ln_free_r(conf);

	if (hmm  != NULL) memfree(hmm);
	if (dict != NULL) memfree(dict);
	if (kws  != NULL) memfree(kws);
}
