#include "sr_runner.h"
#include "sr_error.h"

void SRRunner::start() {
	ERR_FAIL_COND(config.is_null() || queue.is_null());

	if (is_running) stop();
	is_running = true;
	recognition = Thread::create(SRRunner::_thread_recognize, this);
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
	ad_rec_t *recorder = config->get_recorder();
	ps_decoder_t *decoder = config->get_decoder();

	int16 buffer[rec_buffer_size];
	int32 n;
	const char *hyp;

	// Start recording
	if (ad_start_rec(recorder) < 0) {
		is_running = false;
		SRERR_PRINTS(SRError::REC_START_ERR);
		emit_signal(SR_RUNNER_END_SIGNAL, SRError::REC_START_ERR);
		return;
	}

	// Start utterance
	if (ps_start_utt(decoder) < 0) {
		is_running = false;
		ad_stop_rec(recorder);
		SRERR_PRINTS(SRError::UTT_START_ERR);
		emit_signal(SR_RUNNER_END_SIGNAL, SRError::UTT_START_ERR);
		return;
	}

	while (is_running) {
		// Read data from microphone
		if ((n = ad_read(recorder, buffer, rec_buffer_size)) < 0) {
			is_running = false;
			ad_stop_rec(recorder);
			ps_end_utt(decoder);
			SRERR_PRINTS(SRError::AUDIO_READ_ERR);
			emit_signal(SR_RUNNER_END_SIGNAL, SRError::AUDIO_READ_ERR);
			return;
		}

		// Process captured sound
		ps_process_raw(decoder, buffer, n, FALSE, FALSE);

		// Check for keyword in captured sound
		hyp = ps_get_hyp(decoder, NULL);
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
			ps_end_utt(decoder);
			if (ps_start_utt(decoder) < 0) {
				is_running = false;
				ad_stop_rec(recorder);
				ps_end_utt(decoder);
				SRERR_PRINTS(SRError::UTT_RESTART_ERR);
				emit_signal(SR_RUNNER_END_SIGNAL, SRError::UTT_RESTART_ERR);
				return;
			}
		}
	}

	// Stop recording
	if (ad_stop_rec(recorder) < 0) {
		SRERR_PRINTS(SRError::REC_STOP_ERR);
		emit_signal(SR_RUNNER_END_SIGNAL, SRError::REC_STOP_ERR);
	}
}

void SRRunner::set_config(const Ref<SRConfig> &p_config) {
	config = p_config;
}

Ref<SRConfig> SRRunner::get_config() const {
	return config;
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

	ObjectTypeDB::bind_method("set_config", &SRRunner::set_config);
	ObjectTypeDB::bind_method("get_config", &SRRunner::get_config);

	ObjectTypeDB::bind_method("set_rec_buffer_size", &SRRunner::set_rec_buffer_size);
	ObjectTypeDB::bind_method("get_rec_buffer_size", &SRRunner::get_rec_buffer_size);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "config",
	                          PROPERTY_HINT_RESOURCE_TYPE, "SRConfig"),
	             _SCS("set_config"), _SCS("get_config"));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "recorder buffer size",
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
