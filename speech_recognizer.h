#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include "scene/main/node.h"
#include "core/os/thread.h"
#include "core/vector.h"

#include "sphinxbase/err.h"
#include "sphinxbase/ad.h"
#include "pocketsphinx.h"

// Microphone recorder default buffer size
#define DEFAULT_REC_BUFFER_SIZE 2048

// Default capacity for the keywords buffer
#define DEFAULT_KWS_BUFFER_CAP 200

class SpeechRecognizer : public Node {
    OBJ_TYPE(SpeechRecognizer, Node);

    // Represents an error that can occur when calling a SpeechRecognizer method
    enum Error {
        OK,                  // No error occurred
        MULTIBYTE_STR_ERR,   // Couldn't convert filename to a multibyte sequence
        MEMALLOC_ERR,        // No memory available for allocation
        CONFIG_CREATE_ERR,   // Couldn't create Pocketsphinx config variable
        AUDIO_DEV_OPEN_ERR,  // Couldn't open audio device (microphone)
        DECODER_CREATE_ERR,  // Couldn't create Sphinxbase decoder variable
        NO_CONFIG_ERR,       // Possibly called run() without calling config() first
        REC_START_ERR,       // Couldn't start recording user's voice
        REC_STOP_ERR,        // Couldn't stop recording user's voice
        UTT_START_ERR,       // Couldn't start utterance during speech recognition
        UTT_RESTART_ERR,     // Couldn't restart utterance during speech recognition
        AUDIO_READ_ERR       // Error while reading data from recorder
    };

private:
    cmd_ln_t *conf;         // Configuration type for sphinx variables
    ad_rec_t *recorder;     // Records sound from microphone
    ps_decoder_t *decoder;  // Decodes speech to text

    String hmm_dirname;    // Hidden Markov Model directory name
    String dict_filename;  // Dictionary filename
    String kws_filename;   // Keywords filename

    // C strings for the above String names
    char *hmm, *dict, *kws;

    Thread *recognition;  // Used to run the speech recognition in parallel
    bool is_running;      // If true, speech recognition loop is currently on
    int rec_buffer_size;  // Microphone recorder buffer size

    // Stores the last error occurred in the recognition started by the run() method
    // (if no error has yet ocurred, then its value is OK)
    Error running_err;

    // Stores keywords recognized from microphone in a queue fashion
    Vector<String> kws_buffer;

    // Maximum number of keywords that will be stored in kws_buffer
    int kws_buffer_cap;

    /*
     * Thread wrapper function, calls recognize() method of its SpeechRecognizer
     * argument.
     */
    static void thread_recognize(void *sr);

    /*
     * Repeatedly listens to keywords from the user's microphone input.
     */
    void recognize();

protected:
    /*
     * Needed so that GDScript can recognize public methods from this class.
     */
    static void _bind_methods();

public:
    /*
     * Creates a configuration object for recognizing speech. Receives a directory
     * containing files for the Hidden Markov Model, a dictionary with words from
     * the desired language and a keywords file specifying keywords and their
     * threshold values. These files must follow Pocketsphinx conventions.
     *
     * Returns one of the following Error values:
     * - OK
     * - MULTIBYTE_STR_ERR
     * - MEMALLOC_ERR
     * - CONFIG_CREATE_ERR
     * - AUDIO_DEV_OPEN_ERR
     * - DECODER_CREATE_ERR
     */
    Error config(String hmm_dirname, String dict_filename, String kws_filename);

    /*
     * Creates a thread to repeatedly listen to keywords. Must call config()
     * first, or it will fail. The thread can be stopped with the stop() method.
     *
     * Returns one of the following Error values:
     * - OK
     * - NO_CONFIG_ERR
     */
    Error run();

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
     * Returns the last error occurred in the recognition started by run(), which
     * can be one of the following values:
     * - OK
     * - REC_START_ERR
     * - REC_STOP_ERR
     * - UTT_START_ERR
     * - UTT_RESTART_ERR
     * - AUDIO_READ_ERR
     * If no thread was previously run, returns OK.
     */
    Error get_run_error();

    /*
     * Resets the variable that stores the last error ocurred in the recognition
     * started by the run() method, setting its value to Error OK.
     */
    void reset_run_error();

    /*
     * Removes and returns the first element in the keywords buffer.
     * If the buffer is empty, returns NULL.
     */
    String buffer_get();

    /*
     * Returns how many keywords are in the buffer.
     */
    int buffer_size();

    /*
     * Returns true if the keywords buffer is empty, or false otherwise.
     */
    bool buffer_empty();

    /*
     * Clears all keywords in the buffer, leaving it with a size of 0.
     */
    void buffer_clear();

    /*
     * Returns the microphone recorder buffer size used for speech recognition.
     */
    int get_rec_buffer_size();

    /*
     * Sets the microphone recorder buffer size used for speech recognition as the
     * specified value. Must be >= 0. This method must be called before run(), or it
     * won't have any effect.
     */
    void set_rec_buffer_size(int rec_buffer_size);

    /*
     * Returns the keywords buffer capacity.
     */
    int get_kws_buffer_cap();

    /*
     * Sets the keywords buffer capacity as the specified value. Must be >= 0.
     */
    void set_kws_buffer_cap(int kws_buffer_cap);

    /*
     * Initializes speech recognizer variables.
     */
    SpeechRecognizer();

    /*
     * Clears memory used by a speech recognizer object.
     */
    ~SpeechRecognizer();
};

#endif
