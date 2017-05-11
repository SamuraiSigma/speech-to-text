#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include "scene/main/node.h"
#include "sphinxbase/err.h"
#include "sphinxbase/ad.h"

class SpeechRecognizer : public Node {
    OBJ_TYPE(SpeechRecognizer, Node);

    ad_rec_t *recorder;

protected:
    static void _bind_methods();

public:
    SpeechRecognizer();
    ~SpeechRecognizer();
};

#endif
