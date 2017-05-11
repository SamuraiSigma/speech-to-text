#include "register_types.h"
#include "object_type_db.h"
#include "speech_recognizer.h"

void register_speech_recognizer_types() {
    ObjectTypeDB::register_type<SpeechRecognizer>();
}

void unregister_speech_recognizer_types() {
   // Nothing to do here
}
