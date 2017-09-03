#ifndef SR_RUNNER_H
#define SR_RUNNER_H

#include "scene/main/node.h"
#include "core/os/thread.h"

#include "sr_config.h"
#include "sr_queue.h"

/**
 * Signal emitted when speech recognition thread has ended
 */
#define SR_RUNNER_END_SIGNAL "speech_recognition_end"

/**
 * Responsible for running speech recognition itself, identifying keywords spoken
 * by the user.
 *
 * @author SamuraiSigma
 */
class SRRunner : public Node {
	OBJ_TYPE(SRRunner, Node);

private:
	Thread *recognition;  ///< Used to run the speech recognition in parallel
	bool is_running;      ///< If true, speech recognition loop is currently on

	Ref<SRConfig> config;  ///< Configuration object containing recognition variables
	Ref<SRQueue> queue;    ///< Queue for storing recognized keywords

	int rec_buffer_size;  ///< Microphone recorder buffer size

	/**
	 * Thread wrapper function, calls _recognize() method of its SRRunner argument.
	 */
	static void _thread_recognize(void *sr);

	/**
	 * Repeatedly listens to keywords from the user's microphone input.
	 */
	void _recognize();

	/**
	 * Stops recognition thread and emits an error value through the end_signal
	 * signal.
	 *
	 * @param err error value indicating what error ocurred.
	 */
	void _error_stop(SRError::Error err);

protected:
	/**
	 * Makes \a GDScript recognize public methods from this class.
	 */
	static void _bind_methods();

public:
	enum {
		DEFAULT_REC_BUFFER_SIZE = 2048  ///< Microphone recorder default buffer size
	};

	/**
	 * Creates a thread to repeatedly listen to keywords. The thread can be stopped
	 * with the stop() method. If start() was previously called, the current thread
	 * is halted and a new recognition, with the specified arguments, is created.
	 * Returns one of the following SRError::Error values:
	 * - \c OK
	 * - \c UNDEF_CONFIG_ERR
	 * - \c UNDEF_QUEUE_ERR
	 *
	 * Note: The signal \c end_signal is emitted when the thread ends. It contains
	 * an SRError::Error argument representing what made it stop, which can be one
	 * of the following values:
	 * - \c OK
	 * - \c REC_START_ERR
	 * - \c REC_STOP_ERR
	 * - \c UTT_START_ERR
	 * - \c UTT_RESTART_ERR
	 * - \c AUDIO_READ_ERR
	 */
	SRError::Error start();

	/**
	 * Returns true if the speech recognition thread is active, or false otherwise.
	 */
	bool running();

	/**
	 * Stops the thread currently doing speech recognition. If start() wasn't called
	 * previously, this function does nothing.
	 */
	void stop();

	/**
	 * Sets the SRConfig object containing recognition variables. If the speech
	 * recognition thread is currently running, it will be stopped.
	 *
	 * @param p_config reference to a SRConfig object.
	 */
	void set_config(const Ref<SRConfig> &p_config);

	/**
	 * Returns the SRConfig object containing recognition variables.
	 *
	 * @return The currently used SRConfig object for speech recognition, or \c NULL
	 * if not previously defined
	 */
	Ref<SRConfig> get_config() const;

	/**
	 * Sets the SRQueue that stores recognized keywords. If the speech recognition
	 * thread is already running, it will be stopped.
	 *
	 * @param p_queue reference to a SRQueue object.
	 */
	void set_queue(const Ref<SRQueue> &p_queue);

	/**
	 * Returns the SRQueue that stores recognized keywords.
	 *
	 * @return The currently used SRQueue object for storing keywords, or \c NULL if
	 * not previously defined
	 */
	Ref<SRQueue> get_queue() const;

	/**
	 * Sets the microphone recorder buffer size used for speech recognition as the
	 * specified value. Must be > 0. If the speech recognition thread is currently
	 * running, it will be stopped.
	 *
	 * @param rec_buffer_size microphone recorder buffer size
	 */
	void set_rec_buffer_size(int rec_buffer_size);

	/**
	 * Returns the microphone recorder buffer size used for speech recognition.
	 *
	 * @return Microphone recorder buffer size
	 */
	int get_rec_buffer_size();

	/**
	 * Initializes speech recognizer variables. Also creates a keywords queue.
	 */
	SRRunner();

	/**
	 * Clears memory used by a speech recognizer object.
	 */
	~SRRunner();
};

#endif  // SR_RUNNER_H
