#ifndef SR_QUEUE_H
#define SR_QUEUE_H

#include "core/reference.h"
#include "core/vector.h"

// Default capacity for the keywords queue
#define DEFAULT_KWS_CAPACITY 100

class SRQueue : public Reference {
	OBJ_TYPE(SRQueue, Reference);

private:
	// Queue for storing recognized keywords
	Vector<String> keywords;

	// Maximum number of keywords that can be stored in the queue
	int capacity;

protected:
	/*
	 * Needed so that GDScript can recognize public methods from this class.
	 */
	static void _bind_methods();

public:
	/*
	 * Removes and returns the first element in the keywords queue.
	 * If the queue is empty, returns an empty String ("").
	 */
	String get();

	/*
	 * Adds the specified keyword to the end of the queue, returning true if
	 * successful.
	 */
	bool add(String kw);

	/*
	 * Returns how many keywords are in the queue.
	 */
	int size();

	/*
	 * Returns true if the keywords queue is empty, or false otherwise.
	 */
	bool empty();

	/*
	 * Clears all keywords in the queue, leaving it with a size of 0.
	 */
	void clear();

	/*
	 * Sets the keywords queue capacity as the specified value. Must be >= 0. If
	 * the new capacity exceeds the current number of elements in the queue, a
	 * warning message is printed, but no further actions are made.
	 */
	void set_capacity(int capacity);

	/*
	 * Returns the keywords queue capacity.
	 */
	int get_capacity();

	/*
	 * Initializes queue capacity.
	 */
	SRQueue();
};

#endif  // SR_QUEUE_H
