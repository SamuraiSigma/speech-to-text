#ifndef SR_CONFIG_H
#define SR_CONFIG_H

#include "core/resource.h"
#include "sr_error.h"

#include "sphinxbase/err.h"
#include "sphinxbase/ad.h"
#include "pocketsphinx.h"

class SRConfig : public Resource {
	OBJ_TYPE(SRConfig, Resource);

private:
	cmd_ln_t *conf;         // Configuration type for sphinx variables
	ad_rec_t *recorder;     // Records sound from microphone
	ps_decoder_t *decoder;  // Decodes speech to text

	String hmm_dirname;    // Hidden Markov Model directory name
	String dict_filename;  // Dictionary filename
	String kws_filename;   // Keywords filename

	// C strings for the above String names
	char *hmm, *dict, *kws;

protected:
	/*
	 * Needed so that GDScript can recognize public methods from this class.
	 */
	static void _bind_methods();

public:
	/*
	 * Initializes variables for recognizing speech. The attributes for HMM
	 * directory name, dictionary filename and keywords filename must have been
	 * previously defined with the appropriate setters. Supposes that these files
	 * and folder follow Pocketsphinx conventions.
	 *
	 * Returns one of the following SRError::Error values:
	 * - OK
	 * - UNDEF_FILES_ERR
	 * - MULTIBYTE_STR_ERR
	 * - MEMALLOC_ERR
	 * - CONFIG_CREATE_ERR
	 * - REC_CREATE_ERR
	 * - DECODER_CREATE_ERR
	 */
	SRError::Error init();

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
	 * Defines the HMM directory name as the specified value. If the directory
	 * doesn't exist, raises an Error.
	 */
	void set_hmm_dirname(String hmm_dirname);

	/*
	 * Returns the currently defined HMM directory name. If no name has been defined
	 * yet, returns an empty String ("").
	 */
	String get_hmm_dirname();

	/*
	 * Defines the dictionary filename as the specified value. If the file doesn't
	 * exist, raises an Error.
	 */
	void set_dict_filename(String dict_filename);

	/*
	 * Returns the currently defined dictionary filename. If no name has been defined
	 * yet, returns an empty String ("").
	 */
	String get_dict_filename();

	/*
	 * Defines the keywords filename as the specified value. If the file doesn't
	 * exist, raises an Error.
	 */
	void set_kws_filename(String kws_filename);

	/*
	 * Returns the currently defined keywords filename. If no name has been defined
	 * yet, returns an empty String ("").
	 */
	String get_kws_filename();

	/*
	 * Initializes attributes.
	 */
	SRConfig();

	/*
	 * Clears memory used by the object.
	 */
	~SRConfig();
};

#endif  // SR_CONFIG_H
