/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
*/

#ifndef _PK2UNPACK_SDMD2_HPP_
#define _PK2UNPACK_SDMD2_HPP_

#include <map>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <unicode/unistr.h>
#include <duct/stream.hpp>
#include <duct/endianstream.hpp>
#include "datacontainer.hpp"

namespace PK2Unpack {

using namespace duct;

class IDSet {
public:
	IDSet() : _count(0), _data(NULL), _unk(0) {
	};
	~IDSet() {
		release();
	};
	
	uint32_t getCount() const {
		return _count;
	};
	const int32_t* getData() const {
		return _data;
	}
	
	void release() {
		if (_data) {
			free(_data);
			_data=NULL;
			_count=0;
		}
	};
	
	void deserialize(Stream* stream);
	void serialize(Stream* stream) const;
	void printInfo(const char* name, unsigned int tabcount=0, bool newline=true) const;
	
protected:
	uint32_t _count;
	int32_t* _data;
	uint8_t _unk;
};

class FileInfo {
public:
	FileInfo() : _dir_index(0), _index(0), _time_modified(0) {
	};
	FileInfo(Stream* stream) {
		deserialize(stream);
	};
	
	uint32_t getDirIndex() const {
		return _dir_index;
	};
	uint32_t getIndex() const {
		return _index;
	};
	
	void deserialize(Stream* stream);
	void serialize(Stream* stream) const;
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	uint32_t _dir_index;
	uint32_t _index;
	DataContainer _dc;
	time_t _time_modified;
};

typedef std::vector<FileInfo> FileInfoVec;

class EntryInfoSet {
public:
	EntryInfoSet() : _name_count(0), _offsets(NULL), _names_size(0), _names(NULL), _i1(0), _i2(0) {
	};
	~EntryInfoSet() {
		release();
	};
	
	uint32_t getNameCount() const {
		return _name_count;
	};
	const uint32_t* getOffsets() const {
		return _offsets;
	};
	uint32_t getNamesSize() const {
		return _names_size;
	};
	const char** getNames() const {
		return (const char**)_names;
	};
	size_t getFileCount() const {
		return _files.size();
	};
	const FileInfoVec& getData() const {
		return _files;
	};
	
	void release() {
		_name_count=0;
		if (_offsets) {
			free(_offsets);
			_offsets=NULL;
		}
		if (_names) {
			free(_names);
			_names=NULL;
			_names_size=0;
		}
	};
	
	void deserialize(Stream* stream);
	void serialize(Stream* stream) const;
	void printInfo(const char* name, unsigned int tabcount=0, bool newline=true) const;
	
protected:
	uint32_t _name_count;
	uint32_t* _offsets;
	uint32_t _names_size;
	char** _names;
	int32_t _i1, _i2;
	FileInfoVec _files;
};

class SDMD2 {
public:
	SDMD2(const char* path) : _path(path) {
	};
	
	void setPath(const char* path) {
		_path=path;
	};
	const char* getPath() {
		return _path;
	};
	
	void deserialize(Stream* stream);
	bool load();
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	const char* _path;
	IDSet _ids1;
	IDSet _ids2;
	EntryInfoSet _entryinfo;
};

} // namespace PK2Unpack

#endif // _PK2UNPACK_SDMD2_HPP_

