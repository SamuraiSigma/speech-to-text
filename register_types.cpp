#include "register_types.h"
#include "object_type_db.h"

#include "sr_config.h"
#include "sr_queue.h"
#include "sr_runner.h"
#include "sr_error.h"

static SRError *sr_error = NULL;

void register_speech_recognizer_types() {
	ObjectTypeDB::register_type<SRConfig>();
	ObjectTypeDB::register_type<SRQueue>();
	ObjectTypeDB::register_type<SRRunner>();
	ObjectTypeDB::register_virtual_type<SRError>();

	sr_error = memnew(SRError);
	Globals::get_singleton()->add_singleton(Globals::Singleton("SRError", SRError::get_singleton()));
}

void unregister_speech_recognizer_types() {
	if (sr_error) memdelete(sr_error);
}
