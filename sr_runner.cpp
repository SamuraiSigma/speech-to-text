#include <cstdio>       // printf(), fprintf()
#include "sr_runner.h"
#include "sr_error.h"

void SRRunner::start(Ref<SRConfig> config, Ref<SRQueue> queue) {
    if (is_running)
        stop();

    this->config = config;
    this->queue = queue;

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
    int current_buffer_size = config->get_rec_buffer_size();
    ad_rec_t *recorder = config->get_recorder();
    ps_decoder_t *decoder = config->get_decoder();

    int16 buffer[current_buffer_size];
    int32 n;
    const char *hyp;

    // Start recording
    if (ad_start_rec(recorder) < 0) {
        is_running = false;
        emit_signal(SR_RUNNER_END_SIGNAL, SRError::REC_START_ERR);
        return;
    }

    // Start utterance
    if (ps_start_utt(decoder) < 0) {
        is_running = false;
        ad_stop_rec(recorder);
        emit_signal(SR_RUNNER_END_SIGNAL, SRError::UTT_START_ERR);
        return;
    }

    while (is_running) {
        // Read data from microphone
        if ((n = ad_read(recorder, buffer, current_buffer_size)) < 0) {
            is_running = false;
            ad_stop_rec(recorder);
            ps_end_utt(decoder);
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
                    printf("[SpeechRecognizer] %s\n", hyp);
                #endif
            }
            else {
                fprintf(stderr,
                        "Warning: SpeechRecognizer buffer is full! Cannot store "
                        "more keywords!\n");
            }

            // Restart decoder
            ps_end_utt(decoder);
            if (ps_start_utt(decoder) < 0) {
                is_running = false;
                ad_stop_rec(recorder);
                ps_end_utt(decoder);
                emit_signal(SR_RUNNER_END_SIGNAL, SRError::UTT_RESTART_ERR);
                return;
            }
        }
    }

    // Stop recording
    if (ad_stop_rec(recorder) < 0)
        emit_signal(SR_RUNNER_END_SIGNAL, SRError::REC_STOP_ERR);
}

void SRRunner::_bind_methods() {
    ObjectTypeDB::bind_method("start",   &SRRunner::start);
    ObjectTypeDB::bind_method("running", &SRRunner::running);
    ObjectTypeDB::bind_method("stop",    &SRRunner::stop);

    ADD_SIGNAL(MethodInfo(SR_RUNNER_END_SIGNAL,
               PropertyInfo(Variant::INT, "error number")));
}

SRRunner::SRRunner() {
    recognition = NULL;
    is_running = false;
}

SRRunner::~SRRunner() {
    if (recognition != NULL) {
        is_running = false;
        Thread::wait_to_finish(recognition);
        memdelete(recognition);
    }
}
