#include "sr_runner.h"
#include "sr_error.h"
#include "core/os/memory.h"  // memnew(), memdelete()

SRError::Error SRRunner::start() {
	if (config.is_null()) {
		SRERR_PRINTS(SRError::UNDEF_CONFIG_ERR);
		return SRError::UNDEF_CONFIG_ERR;
	}
	if (queue.is_null()) {
		SRERR_PRINTS(SRError::UNDEF_QUEUE_ERR);
		return SRError::UNDEF_QUEUE_ERR;
	}

	if (is_running) stop();
	is_running = true;
	recognition = Thread::create(SRRunner::_thread_recognize, this);

	return SRError::OK;
}

bool SRRunner::running() {
	return is_running;
}

void SRRunner::stop() {
	if (recognition != NULL) {
		is_running = false;
		Thread::wait_to_finish(recognition);
		memdelete(recognition);
		recognition = NULL;
	}
}

void SRRunner::_thread_recognize(void *sr) {
	SRRunner *self = (SRRunner *) sr;
	self->_recognize();
}

void SRRunner::_recognize() {
	int16 buffer[rec_buffer_size];
	int32 n;
	const char *hyp;

	// Start recording
	if (ad_start_rec(config->recorder) < 0) {
		_error_stop(SRError::REC_START_ERR);
		return;
	}

	// Start utterance
	if (ps_start_utt(config->decoder) < 0) {
		_error_stop(SRError::UTT_START_ERR);
		return;
	}

	while (is_running) {
		// Read data from microphone
		if ((n = ad_read(config->recorder, buffer, rec_buffer_size)) < 0) {
			_error_stop(SRError::AUDIO_READ_ERR);
			return;
		}

		// Process captured sound
		ps_process_raw(config->decoder, buffer, n, FALSE, FALSE);

		// Check for keyword in captured sound
		hyp = ps_get_hyp(config->decoder, NULL);
		if (hyp != NULL) {
			// Add new keyword to queue, if possible
			if (queue->add(String(hyp))) {
#ifdef DEBUG_ENABLED
				print_line("[SRRunner] " + String(hyp));
#endif
			}
			else
				WARN_PRINT("Cannot store more keywords in the SRQueue!");

			// Restart decoder
			ps_end_utt(config->decoder);
			if (ps_start_utt(config->decoder) < 0) {
				_error_stop(SRError::UTT_RESTART_ERR);
				return;
			}
		}
	}

	// Stop recording
	if (ad_stop_rec(config->recorder) < 0)
		_error_stop(SRError::REC_STOP_ERR);
}

void SRRunner::_error_stop(SRError::Error err) {
	SRERR_PRINTS(err);

	ad_stop_rec(config->recorder);
	ps_end_utt(config->decoder);
	emit_signal(SR_RUNNER_END_SIGNAL, err);

	is_running = false;
}

void SRRunner::set_config(const Ref<SRConfig> &p_config) {
	stop();
	config = p_config;
}

Ref<SRConfig> SRRunner::get_config() const {
	return config;
}

void SRRunner::set_queue(const Ref<SRQueue> &p_queue) {
	stop();
	queue = p_queue;
}

Ref<SRQueue> SRRunner::get_queue() const {
	return queue;
}

void SRRunner::set_rec_buffer_size(int rec_buffer_size) {
	if (rec_buffer_size <= 0) {
		ERR_PRINT("Microphone recorder buffer size must be greater than 0");
		return;
	}
	stop();
	this->rec_buffer_size = rec_buffer_size;
}

int SRRunner::get_rec_buffer_size() {
	return rec_buffer_size;
}

void SRRunner::_bind_methods() {
	ObjectTypeDB::bind_method("start",   &SRRunner::start);
	ObjectTypeDB::bind_method("running", &SRRunner::running);
	ObjectTypeDB::bind_method("stop",    &SRRunner::stop);

	ObjectTypeDB::bind_method(_MD("set_config", "sr_config"),
	                          &SRRunner::set_config);
	ObjectTypeDB::bind_method("get_config", &SRRunner::get_config);

	ObjectTypeDB::bind_method(_MD("set_queue", "sr_queue"), &SRRunner::set_queue);
	ObjectTypeDB::bind_method("get_queue", &SRRunner::get_queue);

	ObjectTypeDB::bind_method(_MD("set_rec_buffer_size", "size"),
	                          &SRRunner::set_rec_buffer_size);
	ObjectTypeDB::bind_method("get_rec_buffer_size", &SRRunner::get_rec_buffer_size);

	BIND_CONSTANT(DEFAULT_REC_BUFFER_SIZE);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "config",
	                          PROPERTY_HINT_RESOURCE_TYPE, "SRConfig"),
	             _SCS("set_config"), _SCS("get_config"));
	ADD_PROPERTY(PropertyInfo(Variant::INT, "recorder buffer size (bytes)",
	                          PROPERTY_HINT_RANGE, "256,4096,32"),
	             _SCS("set_rec_buffer_size"), _SCS("get_rec_buffer_size"));

	ADD_SIGNAL(MethodInfo(SR_RUNNER_END_SIGNAL,
	                      PropertyInfo(Variant::INT, "error number")));
}

SRRunner::SRRunner() {
	recognition = NULL;
	is_running = false;
	rec_buffer_size = DEFAULT_REC_BUFFER_SIZE;
}

SRRunner::~SRRunner() {
	if (recognition != NULL) {
		is_running = false;
		Thread::wait_to_finish(recognition);
		memdelete(recognition);
	}
}
