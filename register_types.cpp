#include "register_types.h"
#include "object_type_db.h"

#include "sr_config.h"
#include "sr_queue.h"
#include "sr_runner.h"

void register_speech_recognizer_types() {
	ObjectTypeDB::register_type<SRConfig>();
	ObjectTypeDB::register_type<SRQueue>();
	ObjectTypeDB::register_type<SRRunner>();
}

void unregister_speech_recognizer_types() {
	// Nothing to do here
}
