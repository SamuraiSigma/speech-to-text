#ifndef SR_CONFIG_H
#define SR_CONFIG_H

#include "core/reference.h"
#include "sr_error.h"

#include "sphinxbase/err.h"
#include "sphinxbase/ad.h"
#include "pocketsphinx.h"

// Microphone recorder default buffer size
#define DEFAULT_REC_BUFFER_SIZE 2048

class SRConfig : public Reference {
	OBJ_TYPE(SRConfig, Reference);

private:
	cmd_ln_t *conf;         // Configuration type for sphinx variables
	ad_rec_t *recorder;     // Records sound from microphone
	ps_decoder_t *decoder;  // Decodes speech to text

	String hmm_dirname;    // Hidden Markov Model directory name
	String dict_filename;  // Dictionary filename
	String kws_filename;   // Keywords filename

	// C strings for the above String names
	char *hmm, *dict, *kws;

	// Microphone recorder buffer size
	int rec_buffer_size;

protected:
	/*
	 * Needed so that GDScript can recognize public methods from this class.
	 */
	static void _bind_methods();

public:
	/*
	 * Initializes variables for recognizing speech. Receives a directory containing
	 * files for the Hidden Markov Model, a dictionary with words from the desired
	 * language and a keywords file specifying keywords and their threshold values.
	 * These files must follow Pocketsphinx conventions.
	 *
	 * Returns one of the following SRError::Error values:
	 * - OK
	 * - MULTIBYTE_STR_ERR
	 * - MEMALLOC_ERR
	 * - CONFIG_CREATE_ERR
	 * - REC_CREATE_ERR
	 * - DECODER_CREATE_ERR
	 */
	SRError::Error init(String hmm_dirname, String dict_filename,
	                    String kws_filename);

	/*
	 * Returns a pointer to a Pocketsphinx ad_rec_t recorder variable, created
	 * previously by init(). If no recorder variable exists, returns NULL.
	 */
	ad_rec_t * get_recorder();

	/*
	 * Returns a pointer to a Pocketsphinx ps_decoder_t decoder variable, created
	 * previously by init(). If no decoder variable exists, returns NULL.
	 */
	ps_decoder_t * get_decoder();

	/*
	 * Returns the microphone recorder buffer size used for speech recognition.
	 */
	int get_rec_buffer_size();

	/*
	 * Sets the microphone recorder buffer size used for speech recognition as the
	 * specified value. Must be > 0.
	 */
	void set_rec_buffer_size(int rec_buffer_size);

	/*
	 * Initializes attributes.
	 */
	SRConfig();

	/*
	 * Clears memory used by the object.
	 */
	~SRConfig();
};

#endif
