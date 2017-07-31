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
    int rec_buffer_size;     // Microphone recorder buffer size

    // Stores keywords recognized from microphone in a queue fashion
    Vector<String> kws_buffer;

    // Maximum number of keywords that will be stored in kws_buffer
    int kws_buffer_cap;

    /*
     * Thread wrapper function, calls recognize() method of its SpeechRecognizer
     * argument.
     */
    static void thread_recognize(void *speech_recog);

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
     */
    void config(String hmm_dirname, String dict_filename, String kws_filename);

    /*
     * Creates a thread to repeatedly listen to keywords. Must call config()
     * first, or it will fail. The thread can be stopped with the stop() method.
     */
    void run();

    /*
     * Stops a created thread that is running the run() method. If run() wasn't
     * called previously, this function does nothing.
     */
    void stop();

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
