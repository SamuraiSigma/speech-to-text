#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include "scene/main/node.h"

#include "sphinxbase/err.h"
#include "sphinxbase/ad.h"
#include "pocketsphinx.h"

class SpeechRecognizer : public Node {
    OBJ_TYPE(SpeechRecognizer, Node);

    ad_rec_t *recorder;
    ps_decoder_t *decoder;

protected:
    static void _bind_methods();

public:
    SpeechRecognizer();
    ~SpeechRecognizer();
};

#endif
