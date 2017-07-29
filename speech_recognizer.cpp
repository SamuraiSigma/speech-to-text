#include <cstdio>
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

void SpeechRecognizer::config(String hmm_dirname, String dict_filename,
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
        if (len == -1) {
            throw "wcstombs(): Can't represent '" + names[i] + "' as a "
                "multibyte sequence!";
        }

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

    if (recorder == NULL) {
        cmd_ln_free_r(conf);
        throw "Failed to open audio device!";
    }

    // Creater decoder variable
    decoder = ps_init(conf);

    if (decoder == NULL) {
        cmd_ln_free_r(conf);
        ad_close(recorder);
        throw "Failed to create decoder!";
    }
}

void SpeechRecognizer::run() {
    if (conf == NULL || recorder == NULL || decoder == NULL)
        throw "Must configure speech recognizer with config() first!";

    is_running = true;
    recognition = Thread::create(SpeechRecognizer::thread_recognize, this);
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
    int current_buf_size = rec_buf_size;
    int16 buffer[current_buf_size];
    int32 n;
    const char *hyp;

    // Start recording
    if (ad_start_rec(recorder) < 0) {
        is_running = false;
        throw "Failed to start recording";
    }

    // Start utterance
    if (ps_start_utt(decoder) < 0) {
        is_running = false;
        throw "Failed to start utterance";
    }

    while (is_running) {
        // Read data from microphone
        if ((n = ad_read(recorder, buffer, current_buf_size)) < 0) {
            is_running = false;
            throw "Failed to read audio\n";
        }

        // Process captured sound
        ps_process_raw(decoder, buffer, n, FALSE, FALSE);

        // Check for keyword in captured sound
        hyp = ps_get_hyp(decoder, NULL);
        if (hyp != NULL) {
            kws_buffer.push_back(String(hyp));
            printf(">> %s\n", hyp);

            // Restart decoder
            ps_end_utt(decoder);
            if (ps_start_utt(decoder) < 0) {
                is_running = false;
                throw "Failed to start utterance";
            }
        }
    }

    // Stop recording
    if (ad_stop_rec(recorder) < 0)
        throw "Failed to stop recording";
}

String SpeechRecognizer::buffer_get() {
    if (kws_buffer.size() == 0)
        throw "Can't get keyword from an empty buffer";

    String keyword = kws_buffer.get(0);
    kws_buffer.remove(0);
    return keyword;
}

int SpeechRecognizer::buffer_size() {
    return kws_buffer.size();
}

bool SpeechRecognizer::buffer_is_empty() {
    return kws_buffer.empty();
}

void SpeechRecognizer::buffer_clear() {
    kws_buffer.clear();
}

int SpeechRecognizer::get_rec_buf_size() {
    return rec_buf_size;
}

void SpeechRecognizer::set_rec_buf_size(int rec_buf_size) {
    this->rec_buf_size = rec_buf_size;
}

void SpeechRecognizer::_bind_methods() {
    ObjectTypeDB::bind_method("config", &SpeechRecognizer::config);
    ObjectTypeDB::bind_method("run", &SpeechRecognizer::run);
    ObjectTypeDB::bind_method("stop", &SpeechRecognizer::stop);

    ObjectTypeDB::bind_method("buffer_get", &SpeechRecognizer::buffer_get);
    ObjectTypeDB::bind_method("buffer_size", &SpeechRecognizer::buffer_size);
    ObjectTypeDB::bind_method("buffer_is_empty", &SpeechRecognizer::buffer_is_empty);
    ObjectTypeDB::bind_method("buffer_clear", &SpeechRecognizer::buffer_clear);

    ObjectTypeDB::bind_method("get_rec_buf_size",
                              &SpeechRecognizer::get_rec_buf_size);
    ObjectTypeDB::bind_method("set_rec_buf_size",
                              &SpeechRecognizer::set_rec_buf_size);
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
    rec_buf_size = REC_DEFAULT_BUF_SIZE;
}

SpeechRecognizer::~SpeechRecognizer() {
    printf("Keywords (%d):", buffer_size());
    while (!buffer_is_empty()) {
        printf("%s\n", buffer_get().c_str());
    }

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
