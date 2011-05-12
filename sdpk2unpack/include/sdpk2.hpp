
#ifndef _PK2UNPACK_SDPK2_HPP_
#define _PK2UNPACK_SDPK2_HPP_

#include <map>
#include <vector>
#include <stdlib.h>
#include <unicode/unistr.h>
#include <duct/stream.hpp>
#include <duct/endianstream.hpp>

namespace PK2Unpack {

using namespace duct;

enum EntryFlag {
	ENTRYFLAG_NONE=0x00,
	ENTRYFLAG_TABLE=0x01,
	ENTRYFLAG_COMPRESSED=0x02,
	ENTRYFLAG_UNCOMPRESSED=0x04
};

enum EntryType {
	ENTRYTYPE_NONE=0x00,
	ENTRYTYPE_DIR=0x01,
	ENTRYTYPE_FILE=0x02
};

class Entry {
public:
	Entry()
		: _id(0), _dirid(0), _flags(ENTRYFLAG_NONE), _type(ENTRYTYPE_NONE), _name(NULL), _i1(0), _i2(0), _i3(0) {
	};
	Entry(unsigned int id, unsigned int flags=ENTRYFLAG_NONE, EntryType type=ENTRYTYPE_NONE)
		: _id(id), _dirid(0), _flags(flags), _type(type), _name(NULL), _i1(0), _i2(0), _i3(0) {
	};
	~Entry() {
		release();
	};
	
	void setId(unsigned int id) {
		_id=id;
	};
	unsigned int getId() const {
		return _id;
	};
	void setFlags(unsigned int flags) {
		_flags=flags;
	};
	unsigned int getFlags() const {
		return _flags;
	};
	bool hasFlag(EntryFlag flag) {
		return (_flags&flag)!=0;
	};
	void setType(EntryType type) {
		_type=type;
	};
	EntryType getType() const {
		return _type;
	};
	void setName(const char* name) {
		release();
		_name=name;
	};
	const char* getName() const {
		return _name;
	};
	
	void release() {
		if (_name) {
			//free(_name);
			_name=NULL;
		}
	};
	
	void deserializeId(Stream* stream);
	void serializeId(Stream* stream) const;
	void deserializeInfo(Stream* stream);
	void serializeInfo(Stream* stream) const;
	//void* read() const;
	//void write();
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	unsigned int _id, _dirid;
	unsigned int _flags;
	EntryType _type;
	const char* _name;
	int _i1, _i2, _i3;
};

typedef std::map<unsigned int, Entry*> EntryMap;

class IDSet {
public:
	IDSet() : _count(0), _data(NULL), _unk(0) {
	};
	~IDSet() {
		release();
	};
	
	unsigned int getCount() const {
		return _count;
	};
	const int* getData() const {
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
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	unsigned int _count;
	int* _data;
	unsigned char _unk;
};

class NameSet {
public:
	NameSet() : _count(0), _offsets(NULL), _names_size(0), _data(NULL), _i1(0), _i2(0) {
	};
	~NameSet() {
		release();
	};
	
	unsigned int getCount() const {
		return _count;
	};
	const unsigned int* getOffsets() const {
		return _offsets;
	};
	unsigned int getNamesSize() const {
		return _names_size;
	};
	const char** getData() const {
		return (const char**)_data;
	};
	
	void release() {
		_count=0;
		if (_offsets) {
			free(_offsets);
		}
		if (_data) {
			free(_data);
			_data=NULL;
			_names_size=0;
		}
	};
	
	void deserialize(Stream* stream);
	void serialize(Stream* stream) const;
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	unsigned int _count;
	unsigned int* _offsets;
	unsigned int _names_size;
	char** _data;
	int _i1, _i2;
};

class FileInfo {
public:
	FileInfo() : _dir_id(0), _id(0), _i1(0), _i2(0), _i3(0) {
	};
	FileInfo(Stream* stream) {
		deserialize(stream);
	};
	
	unsigned int getDirId() const {
		return _dir_id;
	};
	unsigned int getId() const {
		return _id;
	};
	
	void deserialize(Stream* stream);
	void serialize(Stream* stream) const;
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	unsigned int _dir_id;
	unsigned int _id;
	int _i1, _i2, _i3;
};

typedef std::vector<FileInfo> FileInfoVec;

class FileSet {
public:
	FileSet() {
	};
	
	unsigned int getCount() const {
		return _count;
	};
	const FileInfoVec& getData() const {
		return _data;
	};
	
	void deserialize(Stream* stream);
	void serialize(Stream* stream) const;
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	unsigned int _count;
	FileInfoVec _data;
};

class Archive {
public:
	Archive(const char* pathkey) : _stream(NULL), _pathkey(pathkey) {
	};
	Archive(const UnicodeString& pathkey) : _stream(NULL), _pathkey(pathkey) {
	};
	~Archive() {
		clear();
	};
	
	void setPathKey(const UnicodeString& pathkey) {
		_pathkey.setTo(pathkey);
	};
	const UnicodeString& getPathKey() {
		return _pathkey;
	};
	
	EntryMap::iterator begin() {
		return _entries.begin();
	};
	EntryMap::const_iterator begin() const {
		return _entries.begin();
	};
	EntryMap::iterator end() {
		return _entries.end();
	};
	EntryMap::const_iterator end() const {
		return _entries.end();
	};
	EntryMap::iterator find(unsigned int id) {
		return _entries.find(id);
	};
	EntryMap::const_iterator find(unsigned int id) const {
		return _entries.find(id);
	};
	
	Entry* findEntry(unsigned int id);
	const Entry* findEntry(unsigned int id) const;
	
	void clear() {
		clearEntries();
	};
	
	void clearEntries();
	void deserializeInfo(Stream* stream);
	bool open();
	void close();
	void printInfo(unsigned int tabcount=0, bool newline=true) const;
	
protected:
	EndianStream* _stream;
	UnicodeString _pathkey;
	IDSet _compressed;
	IDSet _uncompressed;
	NameSet _names;
	FileSet _files;
	EntryMap _entries;
};

} // namespace PK2Unpack

#endif // _PK2UNPACK_SDPK2_HPP_

