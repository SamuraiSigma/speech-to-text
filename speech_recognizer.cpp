#include "speech_recognizer.h"
#include "core/os/memory.h"     // memalloc(), memfree()

/*
 * Adds the -adcdev option as a possible command line argument.
 * This argument corresponds to the microphone name.
 */
static arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    // Microphone
    {"-adcdev",
     ARG_STRING,
     NULL,
     "Name of audio device to use for input."},
    CMDLN_EMPTY_OPTION
};

void SpeechRecognizer::config(String hmm_dirname, String dict_filename, String kws_filename) {
    this->hmm_dirname = hmm_dirname;
    this->dict_filename = dict_filename;
    this->kws_filename = kws_filename;

    String names[3];
    names[0] = hmm_dirname;
    names[1] = dict_filename;
    names[2] = kws_filename;

    char *convert[3];
    hmm = convert[0];
    dict = convert[1];
    kws = convert[2];

    // Convert each filename: String -> wchar_t * -> char *
    for (int i = 0; i < 3; i++) {
        int len = wcstombs(NULL, names[i].c_str(), 0);
        if (len == -1)
            throw "wcstombs(): Can't represent '" + names[i] + "' as a multibyte sequence!";

        convert[i] = (char *) memalloc((len + 1) * sizeof(char));
        if (convert[i] == NULL)
            throw "memalloc(): Couldn't allocate memory!";

        wcstombs(convert[i], names[i].c_str(), len + 1);
    }

    // Create configuration variable
    conf = cmd_ln_init(NULL, ps_args(), TRUE,
                       "-hmm", convert[0],
                       "-dict", convert[1],
                       "-kws", convert[2],
                       NULL);

    if (conf == NULL)
        throw "Failed to create configuration object!";

    // Update basic configuration with custom one for mic
    conf = cmd_ln_init(conf, cont_args_def, TRUE, NULL);
    if (conf == NULL)
        throw "Failed to create configuration object!";

    // Create recorder variable
    recorder = ad_open_dev(cmd_ln_str_r(conf, "-adcdev"),
                           (int) cmd_ln_float32_r(conf, "-samprate"));

    if (recorder == NULL)
        throw "Failed to open audio device!";

    // Creater decoder variable
    decoder = ps_init(conf);

    if (decoder == NULL)
        throw "Failed to create decoder!";
}

void SpeechRecognizer::_bind_methods() {
    ObjectTypeDB::bind_method("config", &SpeechRecognizer::config);
}

SpeechRecognizer::SpeechRecognizer() {
    #ifndef DEBUG_ENABLED
        // Disable Pocketphinx log output
        err_set_logfp(NULL);
    #endif

    conf = NULL; recorder = NULL; decoder = NULL;

    hmm_dirname   = "";
    dict_filename = "";
    kws_filename  = "";

    hmm = NULL; dict = NULL; kws = NULL;
}

SpeechRecognizer::~SpeechRecognizer() {
    // Free sphinx variables
    if (recorder != NULL) ad_close(recorder);
    if (decoder  != NULL) ps_free(decoder);
    if (conf     != NULL) cmd_ln_free_r(conf);

    if (hmm != NULL)  memfree(hmm);
    if (dict != NULL) memfree(dict);
    if (kws != NULL)  memfree(kws);
}
