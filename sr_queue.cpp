#include "sr_queue.h"

String SRQueue::get() {
	if (empty()) {
		WARN_PRINT("Empty keywords queue, returning empty String");
		return String("");
	}

	String kw = keywords.get(0);
	keywords.remove(0);
	return kw;
}

bool SRQueue::add(String kw) {
	if (size() + 1 > capacity)
		return false;

	keywords.push_back(kw);
	return true;
}

int SRQueue::size() {
	return keywords.size();
}

bool SRQueue::empty() {
	return keywords.empty();
}

void SRQueue::clear() {
	keywords.clear();
}

void SRQueue::set_capacity(int capacity) {
	if (capacity <= 0) {
		ERR_PRINT("Keywords queue capacity must be greater than 0");
		return;
	}
	this->capacity = capacity;
	if (size() > capacity)
		WARN_PRINT("New capacity exceeds current number of keywords in buffer");
}

int SRQueue::get_capacity() {
	return capacity;
}

void SRQueue::_bind_methods() {
	ObjectTypeDB::bind_method("get",                 &SRQueue::get);
	ObjectTypeDB::bind_method(_MD("add", "keyword"), &SRQueue::add);
	ObjectTypeDB::bind_method("size",                &SRQueue::size);
	ObjectTypeDB::bind_method("empty",               &SRQueue::empty);
	ObjectTypeDB::bind_method("clear",               &SRQueue::clear);

	ObjectTypeDB::bind_method(_MD("set_capacity", "capacity"),
	                          &SRQueue::set_capacity);
	ObjectTypeDB::bind_method("get_capacity", &SRQueue::get_capacity);

	BIND_CONSTANT(DEFAULT_KWS_CAPACITY);
}

SRQueue::SRQueue() {
	capacity = DEFAULT_KWS_CAPACITY;
}

SRQueue::~SRQueue() {}
