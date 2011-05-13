
#ifndef _PK2UNPACK_SDPK2_HPP_
#define _PK2UNPACK_SDPK2_HPP_

//#include <map>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <duct/stream.hpp>
#include <duct/endianstream.hpp>
#include "misc.hpp"

namespace PK2Unpack {

using namespace duct;

enum CompressionMethod {
	COMPMETHOD_UNKNOWN=0,
	COMPMETHOD_FIRST=1,
	/* PC */
	COMPMETHOD_ZLIB=1,
	/* PS3 */
	COMPMETHOD_LZX,
	COMPMETHOD_LAST=COMPMETHOD_LZX
};

// plain-data MD5 hash
class MD5Hash {
public:
	MD5Hash() {
		clear();
	};
	unsigned char* data() {
		return _data;
	};
	const unsigned char* data() const {
		return _data;
	};
	bool isNull() const;
	void clear() {
		memset(_data, 0x00, 16);
	};
	int compare(const MD5Hash& other) const {
		return memcmp(_data, other._data, 16);
	};
	void deserialize(Stream* stream) {
		stream->read(_data, 16);
	};
	void serialize(Stream* stream) const {
		stream->write(_data, 16);
	};
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	unsigned char _data[16];
};

struct MD5HashCompare {
	bool operator()(const MD5Hash* x, const MD5Hash* y) const {
		return x->compare(*y)<0;
	};
};

class Entry {
public:
	Entry() : _blocksize_index(0), _blocksize(0), _size(0), _offset(0) {
	};
	Entry(Stream* stream) {
		deserialize(stream);
	};
	MD5Hash hash() {
		return _hash;
	};
	const MD5Hash& hash() const {
		return _hash;
	};
	unsigned int getBlockSizeIndex() const {
		return _blocksize_index;
	};
	void* read(Stream* stream, CompressionMethod comp_method) const;
	void deserialize(Stream* stream);
	void deserializeBlockSize(Stream* stream);
	void serialize(Stream* stream) const;
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	MD5Hash _hash;
	unsigned int _blocksize_index, _blocksize;
	size_t _size, _offset;
};

//typedef std::map<const MD5Hash*, Entry*, MD5HashCompare> EntryMap;
typedef std::vector<Entry> EntryVec;

class SDPK2 {
public:
	SDPK2(const char* path) : _stream(NULL), _path(path), _comp_method(COMPMETHOD_UNKNOWN), _block_size(0) {
	};
	~SDPK2() {
		clear();
	};
	void setPath(const char* path) {
		_path=path;
	};
	const char* getPath() {
		return _path;
	};
	void clear() {
		clearEntries();
	};
	void clearEntries();
	Entry* findEntry(const MD5Hash& hash);
	const Entry* findEntry(const MD5Hash& hash) const;
	void deserializeInfo(Stream* stream);
	bool open();
	void close();
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	EndianStream* _stream;
	const char* _path;
	CompressionMethod _comp_method;
	size_t _block_size;
	//EntryMap _entries;
	EntryVec _entries;
};

} // namespace PK2Unpack

#endif // _PK2UNPACK_SDPK2_HPP_

