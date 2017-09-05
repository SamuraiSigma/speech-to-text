#include "stt_runner.h"
#include "stt_error.h"
#include "core/os/memory.h"  // memnew(), memdelete()

STTError::Error STTRunner::start() {
	if (config.is_null()) {
		STT_ERR_PRINTS(STTError::UNDEF_CONFIG_ERR);
		return STTError::UNDEF_CONFIG_ERR;
	}
	if (queue.is_null()) {
		STT_ERR_PRINTS(STTError::UNDEF_QUEUE_ERR);
		return STTError::UNDEF_QUEUE_ERR;
	}

	if (is_running) stop();
	is_running = true;
	recognition = Thread::create(STTRunner::_thread_recognize, this);

	return STTError::OK;
}

bool STTRunner::running() {
	return is_running;
}

void STTRunner::stop() {
	if (recognition != NULL) {
		is_running = false;
		Thread::wait_to_finish(recognition);
		memdelete(recognition);
		recognition = NULL;
	}
}

void STTRunner::_thread_recognize(void *runner) {
	STTRunner *self = (STTRunner *) runner;
	self->_recognize();
}

void STTRunner::_recognize() {
	int16 buffer[rec_buffer_size];
	int32 n;
	const char *hyp;

	// Start recording
	if (ad_start_rec(config->recorder) < 0) {
		_error_stop(STTError::REC_START_ERR);
		return;
	}

	// Start utterance
	if (ps_start_utt(config->decoder) < 0) {
		_error_stop(STTError::UTT_START_ERR);
		return;
	}

	while (is_running) {
		// Read data from microphone
		if ((n = ad_read(config->recorder, buffer, rec_buffer_size)) < 0) {
			_error_stop(STTError::AUDIO_READ_ERR);
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
				print_line("[STTRunner] " + String(hyp));
#endif
			}
			else
				WARN_PRINT("Cannot store more keywords in the STTQueue!");

			// Restart decoder
			ps_end_utt(config->decoder);
			if (ps_start_utt(config->decoder) < 0) {
				_error_stop(STTError::UTT_RESTART_ERR);
				return;
			}
		}
	}

	// Stop recording
	if (ad_stop_rec(config->recorder) < 0)
		_error_stop(STTError::REC_STOP_ERR);
}

void STTRunner::_error_stop(STTError::Error err) {
	STT_ERR_PRINTS(err);

	ad_stop_rec(config->recorder);
	ps_end_utt(config->decoder);
	emit_signal(STT_RUNNER_END_SIGNAL, err);

	is_running = false;
}

void STTRunner::set_config(const Ref<STTConfig> &p_config) {
	stop();
	config = p_config;
}

Ref<STTConfig> STTRunner::get_config() const {
	return config;
}

void STTRunner::set_queue(const Ref<STTQueue> &p_queue) {
	stop();
	queue = p_queue;
}

Ref<STTQueue> STTRunner::get_queue() const {
	return queue;
}

void STTRunner::set_rec_buffer_size(int rec_buffer_size) {
	if (rec_buffer_size <= 0) {
		ERR_PRINT("Microphone recorder buffer size must be greater than 0");
		return;
	}
	stop();
	this->rec_buffer_size = rec_buffer_size;
}

int STTRunner::get_rec_buffer_size() {
	return rec_buffer_size;
}

void STTRunner::_bind_methods() {
	ObjectTypeDB::bind_method("start",   &STTRunner::start);
	ObjectTypeDB::bind_method("running", &STTRunner::running);
	ObjectTypeDB::bind_method("stop",    &STTRunner::stop);

	ObjectTypeDB::bind_method(_MD("set_config", "stt_config"),
	                          &STTRunner::set_config);
	ObjectTypeDB::bind_method("get_config", &STTRunner::get_config);

	ObjectTypeDB::bind_method(_MD("set_queue", "stt_queue"), &STTRunner::set_queue);
	ObjectTypeDB::bind_method("get_queue", &STTRunner::get_queue);

	ObjectTypeDB::bind_method(_MD("set_rec_buffer_size", "size"),
	                          &STTRunner::set_rec_buffer_size);
	ObjectTypeDB::bind_method("get_rec_buffer_size", &STTRunner::get_rec_buffer_size);

	BIND_CONSTANT(DEFAULT_REC_BUFFER_SIZE);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "config",
	                          PROPERTY_HINT_RESOURCE_TYPE, "STTConfig"),
	             _SCS("set_config"), _SCS("get_config"));
	ADD_PROPERTY(PropertyInfo(Variant::INT, "recorder buffer size (bytes)",
	                          PROPERTY_HINT_RANGE, "256,4096,32"),
	             _SCS("set_rec_buffer_size"), _SCS("get_rec_buffer_size"));

	ADD_SIGNAL(MethodInfo(STT_RUNNER_END_SIGNAL,
	                      PropertyInfo(Variant::INT, "error number")));
}

STTRunner::STTRunner() {
	recognition = NULL;
	is_running = false;
	rec_buffer_size = DEFAULT_REC_BUFFER_SIZE;
}

STTRunner::~STTRunner() {
	if (recognition != NULL) {
		is_running = false;
		Thread::wait_to_finish(recognition);
		memdelete(recognition);
	}
}
