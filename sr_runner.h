#ifndef SR_RUNNER_H
#define SR_RUNNER_H

#include "scene/main/node.h"
#include "core/os/thread.h"

#include "sr_config.h"
#include "sr_queue.h"

// Microphone recorder default buffer size
#define DEFAULT_REC_BUFFER_SIZE 2048

// Signal emitted when speech recognition thread has ended
#define SR_RUNNER_END_SIGNAL "speech_recognition_end"

class SRRunner : public Node {
	OBJ_TYPE(SRRunner, Node);

private:
	Thread *recognition;  // Used to run the speech recognition in parallel
	bool is_running;      // If true, speech recognition loop is currently on

	Ref<SRConfig> config;  // Configuration object containing recognition variables
	SRQueue *queue;        // Queue for storing recognized keywords

	// Microphone recorder buffer size
	int rec_buffer_size;

	/*
	 * Thread wrapper function, calls _recognize() method of its SRRunner
	 * argument.
	 */
	static void _thread_recognize(void *sr);

	/*
	 * Repeatedly listens to keywords from the user's microphone input.
	 */
	void _recognize();

	/*
	 * Sets the keywords queue's maximum capacity.
	 */
	void set_queue_capacity(int capacity);

	/*
	 * Returns the keywords queue's current maximum capacity.
	 */
	int get_queue_capacity();

protected:
	/*
	 * Needed so that GDScript can recognize public methods from this class.
	 */
	static void _bind_methods();

public:
	/*
	 * Creates a thread to repeatedly listen to keywords. The thread can be stopped
	 * with the stop() method. If start() was previously called, the current thread
	 * is halted and a new recognition, with the specified arguments, is created.
	 * Returns one of the following SRError::Error values:
	 * - OK
	 * - UNDEF_CONFIG_ERR
	 *
	 * Note: The signal SR_RUNNER_END_SIGNAL is emitted when the thread ends. It
	 * contains an SRError::Error argument representing what made it stop, which can
	 * be one of the following values:
	 * - OK
	 * - REC_START_ERR
	 * - REC_STOP_ERR
	 * - UTT_START_ERR
	 * - UTT_RESTART_ERR
	 * - AUDIO_READ_ERR
	 */
	SRError::Error start();

	/*
	 * Returns true if the speech recognition thread is active, or false otherwise.
	 */
	bool running();

	/*
	 * Stops a created thread that is running the run() method. If run() wasn't
	 * called previously, this function does nothing.
	 */
	void stop();

	/*
	 * Sets the SRConfig object containing recognition variables. If the speech
	 * recognition thread is currently running, it will be stopped.
	 */
	void set_config(const Ref<SRConfig> &p_config);

	/*
	 * Returns the SRConfig object containing recognition variables.
	 */
	Ref<SRConfig> get_config() const;

	/*
	 * Returns the SRQueue that stores recognized keywords.
	 */
	Ref<SRQueue> get_queue() const;

	/*
	 * Creates a new SRQueue to be used, with the same capacity as the previous one.
	 * Note that the reference to the previous SRQueue will still exist, and be
	 * freed when no more references to it exist. If the speech recognition thread is
	 * currently running, it will be stopped.
	 */
	void reset_queue();

	/*
	 * Sets the microphone recorder buffer size used for speech recognition as the
	 * specified value. Must be > 0. If the speech recognition thread is currently
	 * running, it will be stopped.
	 */
	void set_rec_buffer_size(int rec_buffer_size);

	/*
	 * Returns the microphone recorder buffer size used for speech recognition.
	 */
	int get_rec_buffer_size();

	/*
	 * Initializes speech recognizer variables. Also creates a keywords queue.
	 */
	SRRunner();

	/*
	 * Clears memory used by a speech recognizer object.
	 */
	~SRRunner();
};

#endif  // SR_RUNNER_H
