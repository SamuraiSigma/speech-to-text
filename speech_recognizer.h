#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include "scene/main/node.h"

#include "sphinxbase/err.h"
#include "sphinxbase/ad.h"
#include "pocketsphinx.h"

class SpeechRecognizer : public Node {
    OBJ_TYPE(SpeechRecognizer, Node);

    cmd_ln_t *conf;         // Configuration type for sphinx variables
    ad_rec_t *recorder;     // Records sound from microphone
    ps_decoder_t *decoder;  // Decodes speech to text

    String hmm_dirname;    // Hidden Markov Model directory name
    String dict_filename;  // Dictionary filename
    String kws_filename;   // Keywords filename

    // C strings for the above String names
    char *hmm, *dict, *kws;

protected:
    static void _bind_methods();

public:
    /*
     * Creates a configuration object for recognizing speech.
     */
    void config(String hmm_dirname, String dict_filename, String kws_filename);

    /*
     * Initializes speech recognizer variables.
     */
    SpeechRecognizer();

    /*
     * Clears memory used by a speech recognizer object.
     */
    ~SpeechRecognizer();
};

#endif
