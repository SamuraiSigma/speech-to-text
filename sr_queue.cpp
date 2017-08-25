#include "sr_queue.h"

String SRQueue::get() {
	if (empty()) {
		WARN_PRINT("Empty keywords buffer, returning empty String");
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

int SRQueue::get_capacity() {
	return capacity;
}

void SRQueue::set_capacity(int capacity) {
	if (capacity <= 0) {
		ERR_PRINT("Keywords buffer capacity must be greater than 0");
		return;
	}
	this->capacity = capacity;
}

void SRQueue::_bind_methods() {
	ObjectTypeDB::bind_method("get",   &SRQueue::get);
	ObjectTypeDB::bind_method("add",   &SRQueue::add);
	ObjectTypeDB::bind_method("size",  &SRQueue::size);
	ObjectTypeDB::bind_method("empty", &SRQueue::empty);
	ObjectTypeDB::bind_method("clear", &SRQueue::clear);

	ObjectTypeDB::bind_method("get_capacity", &SRQueue::get_capacity);
	ObjectTypeDB::bind_method("set_capacity", &SRQueue::set_capacity);
}

SRQueue::SRQueue() {
	capacity = DEFAULT_KWS_CAPACITY;
}
