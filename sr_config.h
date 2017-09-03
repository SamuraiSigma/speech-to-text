#ifndef SR_CONFIG_H
#define SR_CONFIG_H

#include "core/resource.h"
#include "sr_error.h"

#include "sphinxbase/err.h"
#include "sphinxbase/ad.h"
#include "pocketsphinx.h"

/**
 * Stores filenames and variables for Pocketsphinx speech recognition.
 *
 * Contains filenames and variables needed for Pocketsphinx speech recognition.
 * Instances of this class supply information to SRRunner objects.
 *
 * @author SamuraiSigma
 */
class SRConfig : public Resource {
	OBJ_TYPE(SRConfig, Resource);
	friend class SRRunner;

private:
	cmd_ln_t *conf;         ///< Configuration type for Sphinx variables
	ad_rec_t *recorder;     ///< Records sound from microphone
	ps_decoder_t *decoder;  ///< Decodes speech to text

	String hmm_dirname;    ///< Hidden Markov Model directory name
	String dict_filename;  ///< Dictionary filename
	String kws_filename;   ///< Keywords filename

	char *hmm;   ///< C string path for hmm_dirname
	char *dict;  ///< C string path for dict_filename
	char *kws;   ///< C string path for kws_filename

protected:
	/**
	 * Makes \a GDScript recognize public methods from this class.
	 */
	static void _bind_methods();

public:
	/**
	 * Initializes \a Pocketsphinx speech recognition variables. The attributes for
	 * HMM directory name, dictionary filename and keywords filename must have been
	 * previously defined with the appropriate setters.
	 *
	 * @return One of the following SRError::Error values:
	 * - \c OK
	 * - \c UNDEF_FILES_ERR
	 * - \c MULTIBYTE_STR_ERR
	 * - \c MEMALLOC_ERR
	 * - \c CONFIG_CREATE_ERR
	 * - \c REC_CREATE_ERR
	 * - \c DECODER_CREATE_ERR
	 *
	 * @see set_hmm_dirname for setting the HMM directory name
	 * @see set_dict_filename for setting the dictionary filename
	 * @see set_kws_filename for setting the keywords filename
	 */
	SRError::Error init();

	/**
	 * Sets the HMM directory name as the specified value if the directory exists.
	 *
	 * @param hmm_dirname the HMM directory name.
	 */
	void set_hmm_dirname(String hmm_dirname);

	/**
	 * Returns the currently defined HMM directory name. If no name has been defined
	 * yet, returns an empty <tt>String ("")</tt>.
	 *
	 * @return The current HMM dierctory name, or \c "" if not defined.
	 */
	String get_hmm_dirname();

	/**
	 * Sets the dictionary filename as the specified value if the file exists.
	 *
	 * @param dict_filename the dictionary filename.
	 */
	void set_dict_filename(String dict_filename);

	/**
	 * Returns the currently defined dictionary filename. If no name has been defined
	 * yet, returns an empty <tt>String ("")</tt>.
	 *
	 * @return The current dictionary filename, or \c "" if not defined.
	 */
	String get_dict_filename();

	/**
	 * Sets the keywords filename as the specified value if the file exists.
	 *
	 * @param kws_filename the keywords filename.
	 */
	void set_kws_filename(String kws_filename);

	/**
	 * Returns the currently defined keywords filename. If no name has been defined
	 * yet, returns an empty <tt>String ("")</tt>.
	 *
	 * @return The current keywords filename, or \c "" if not defined.
	 */
	String get_kws_filename();

	/**
	 * Initializes attributes.
	 */
	SRConfig();

	/**
	 * Clears memory used by the object.
	 */
	~SRConfig();
};

#endif  // SR_CONFIG_H
