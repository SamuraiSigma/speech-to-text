#include <cstdio>               // printf(), fprintf()
#include "speech_recognizer.h"
#include "core/os/memory.h"     // memalloc(), memfree(), memdelete()

/*
 * Adds the -adcdev option as a possible command line argument.
 * This argument corresponds to the microphone name.
 */
static arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    // Microphone
    {"-adcdev",
     ARG_STRING,
     NULL,  // Use default system microphone
     "Name of audio device to use for input."},
    CMDLN_EMPTY_OPTION
};

SpeechRecognizer::Error SpeechRecognizer::config(String hmm_dirname,
                              String dict_filename,
                              String kws_filename) {
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
            return MULTIBYTE_STR_ERR;

        convert[i] = (char *) memalloc((len + 1) * sizeof(char));
        if (convert[i] == NULL)
            return MEMALLOC_ERR;

        wcstombs(convert[i], names[i].c_str(), len + 1);

        #ifdef DEBUG_ENABLED
            printf("[SpeechRecognizer Argument] %s\n", convert[i]);
        #endif
    }

    // Create configuration variable
    conf = cmd_ln_init(NULL, ps_args(), TRUE,
                        "-hmm", convert[0],
                        "-dict", convert[1],
                        "-kws", convert[2],
                        NULL);

    if (conf == NULL)
        return CONFIG_CREATE_ERR;

    // Update basic configuration with custom one for mic
    conf = cmd_ln_init(conf, cont_args_def, TRUE, NULL);
    if (conf == NULL)
        return CONFIG_CREATE_ERR;

    // Create recorder variable
    recorder = ad_open_dev(cmd_ln_str_r(conf, "-adcdev"),
                            (int) cmd_ln_float32_r(conf, "-samprate"));

    if (recorder == NULL) {
        cmd_ln_free_r(conf);
        return AUDIO_DEV_OPEN_ERR;
    }

    // Creater decoder variable
    decoder = ps_init(conf);

    if (decoder == NULL) {
        cmd_ln_free_r(conf);
        ad_close(recorder);
        return DECODER_CREATE_ERR;
    }

    return OK;
}

SpeechRecognizer::Error SpeechRecognizer::run() {
    if (conf == NULL || recorder == NULL || decoder == NULL)
        return NO_CONFIG_ERR;

    is_running = true;
    running_err = OK;
    recognition = Thread::create(SpeechRecognizer::thread_recognize, this);

    return OK;
}

void SpeechRecognizer::stop() {
    if (recognition != NULL) {
        is_running = false;
        Thread::wait_to_finish(recognition);
        memdelete(recognition);
        recognition = NULL;
    }
}

void SpeechRecognizer::thread_recognize(void *s) {
    SpeechRecognizer *self = (SpeechRecognizer *) s;
    self->recognize();
}

void SpeechRecognizer::recognize() {
    int current_buffer_size = rec_buffer_size;
    int16 buffer[current_buffer_size];
    int32 n;
    const char *hyp;

    // Start recording
    if (ad_start_rec(recorder) < 0) {
        is_running = false;
        running_err = REC_START_ERR;
    }

    // Start utterance
    if (ps_start_utt(decoder) < 0) {
        is_running = false;
        running_err = UTT_START_ERR;
    }

    while (is_running) {
        // Read data from microphone
        if ((n = ad_read(recorder, buffer, current_buffer_size)) < 0) {
            is_running = false;
            running_err = AUDIO_READ_ERR;
        }

        // Process captured sound
        ps_process_raw(decoder, buffer, n, FALSE, FALSE);

        // Check for keyword in captured sound
        hyp = ps_get_hyp(decoder, NULL);
        if (hyp != NULL) {
            if (buffer_size() + 1 <= kws_buffer_cap) {
                kws_buffer.push_back(String(hyp));

                #ifdef DEBUG_ENABLED
                    printf("[SpeechRecognizer] %s\n", hyp);
                #endif
            }
            else {
                fprintf(stderr,
                        "Warning: SpeechRecognizer buffer is full! Cannot store "
                        "more keywords!\n");
            }

            // Restart decoder
            ps_end_utt(decoder);
            if (ps_start_utt(decoder) < 0) {
                is_running = false;
                running_err = UTT_RESTART_ERR;
            }
        }
    }

    // Stop recording
    if (ad_stop_rec(recorder) < 0)
        running_err = REC_STOP_ERR;
}

SpeechRecognizer::Error SpeechRecognizer::get_run_error() {
    return running_err;
}

void SpeechRecognizer::reset_run_error() {
    running_err = OK;
}

String SpeechRecognizer::buffer_get() {
    // TODO: Return some kind of NULL
    if (kws_buffer.size() == 0)
        throw "Can't get keyword from an empty buffer";

    String keyword = kws_buffer.get(0);
    kws_buffer.remove(0);
    return keyword;
}

int SpeechRecognizer::buffer_size() {
    return kws_buffer.size();
}

bool SpeechRecognizer::buffer_empty() {
    return kws_buffer.empty();
}

void SpeechRecognizer::buffer_clear() {
    kws_buffer.clear();
}

int SpeechRecognizer::get_rec_buffer_size() {
    return rec_buffer_size;
}

void SpeechRecognizer::set_rec_buffer_size(int rec_buffer_size) {
    if (rec_buffer_size <= 0) {
        fprintf(stderr, "Microphone recorder buffer size must be greater than 0\n");
        return;
    }
    this->rec_buffer_size = rec_buffer_size;
}

int SpeechRecognizer::get_kws_buffer_cap() {
    return kws_buffer_cap;
}

void SpeechRecognizer::set_kws_buffer_cap(int kws_buffer_cap) {
    if (kws_buffer_cap <= 0) {
        fprintf(stderr, "Keywords buffer capacity must be greater than 0\n");
        return;
    }
    this->kws_buffer_cap = kws_buffer_cap;
}

void SpeechRecognizer::_bind_methods() {
    ObjectTypeDB::bind_method("config", &SpeechRecognizer::config);
    ObjectTypeDB::bind_method("run", &SpeechRecognizer::run);
    ObjectTypeDB::bind_method("stop", &SpeechRecognizer::stop);

    ObjectTypeDB::bind_method("get_run_error", &SpeechRecognizer::get_run_error);
    ObjectTypeDB::bind_method("reset_run_error", &SpeechRecognizer::reset_run_error);

    ObjectTypeDB::bind_method("buffer_get", &SpeechRecognizer::buffer_get);
    ObjectTypeDB::bind_method("buffer_size", &SpeechRecognizer::buffer_size);
    ObjectTypeDB::bind_method("buffer_empty", &SpeechRecognizer::buffer_empty);
    ObjectTypeDB::bind_method("buffer_clear", &SpeechRecognizer::buffer_clear);

    ObjectTypeDB::bind_method("get_rec_buffer_size",
                              &SpeechRecognizer::get_rec_buffer_size);
    ObjectTypeDB::bind_method("set_rec_buffer_size",
                              &SpeechRecognizer::set_rec_buffer_size);
    ObjectTypeDB::bind_method("get_kws_buffer_cap",
                              &SpeechRecognizer::get_kws_buffer_cap);
    ObjectTypeDB::bind_method("set_kws_buffer_cap",
                              &SpeechRecognizer::set_kws_buffer_cap);
}

SpeechRecognizer::SpeechRecognizer() {
    // Disable Pocketphinx log output
    err_set_logfp(NULL);

    conf = NULL; recorder = NULL; decoder = NULL;

    hmm_dirname   = "";
    dict_filename = "";
    kws_filename  = "";

    hmm = NULL; dict = NULL; kws = NULL;

    recognition = NULL;
    is_running = false;
    running_err = OK;

    rec_buffer_size = DEFAULT_REC_BUFFER_SIZE;
    kws_buffer_cap = DEFAULT_KWS_BUFFER_CAP;
}

SpeechRecognizer::~SpeechRecognizer() {
    if (recognition != NULL) {
        is_running = false;
        Thread::wait_to_finish(recognition);
        memdelete(recognition);
    }

    if (recorder != NULL) ad_close(recorder);
    if (decoder  != NULL) ps_free(decoder);
    if (conf     != NULL) cmd_ln_free_r(conf);

    if (hmm != NULL)  memfree(hmm);
    if (dict != NULL) memfree(dict);
    if (kws != NULL)  memfree(kws);
}
