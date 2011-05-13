
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include <duct/debug.hpp>
#include <duct/filestream.hpp>
#include <duct/endianstream.hpp>
#include "misc.hpp"
#include "sdpk2.hpp"

namespace PK2Unpack {

// class MD5Hash implementation

const char __hash_null[]={0x00};

bool MD5Hash::isNull() const {
	return memcmp(_data, __hash_null, 16)==0;
}

void MD5Hash::printInfo(unsigned int tabcount, bool newline) const {
	//printf("%.*s[%016lx%016lx]%.*s", tabcount, CONST_TAB_STR, ((unsigned long*)_data)[0], ((unsigned long*)_data)[1], (newline) ? 1 : 0, "\n");
	printf("%.*s[", tabcount, CONST_TAB_STR);
	for (int i=0; i<16; i+=4) {
		printf("%02x%02x%02x%02x", _data[i], _data[i+1], _data[i+2], _data[i+3]);
	}
	printf("]%.*s", (newline) ? 1 : 0, "\n");
}

// class Entry implementation

#define __uint40_make(o, b, i) ((o=((size_t)b<<32)|i))
#define __uint40_split(o, b, i) (({b=o&0x00FF00000000; i=o&0x0000FFFFFFFF;}))

void* Entry::read(Stream* stream, CompressionMethod comp_method) const {
	debug_assertp(comp_method==COMPMETHOD_ZLIB, this, "unsupported compression");
	return NULL;
}

void Entry::deserialize(Stream* stream) {
	_hash.deserialize(stream);
	_blocksize_index=stream->readInt();
	unsigned int b, i;
	b=stream->readByte();
	i=stream->readInt();
	__uint40_make(_size, b, i);
	b=stream->readByte();
	i=stream->readInt();
	__uint40_make(_offset, b, i);
}

void Entry::deserializeBlockSize(Stream* stream) {
	_blocksize=(unsigned short)stream->readShort();
}

void Entry::serialize(Stream* stream) const {
	_hash.serialize(stream);
	stream->writeInt(_blocksize_index);
	unsigned char b;
	unsigned int i;
	__uint40_split(_size, b, i);
	stream->writeByte(b);
	stream->writeInt(i);
	__uint40_split(_offset, b, i);
	stream->writeByte(b);
	stream->writeInt(i);
}

void Entry::printInfo(unsigned int tabcount, bool newline) const {
	printf("%.*s[hash:", tabcount, CONST_TAB_STR);
	_hash.printInfo(0, false);
	printf(", blocksize_index:%6u, blocksize:%8u, size:%8lu, offset:%10lu]%.*s", _blocksize_index, _blocksize, _size, _offset, (newline) ? 1 : 0, "\n");
}

// class SDPK2 implementation

Entry* SDPK2::findEntry(const MD5Hash& hash) {
	/*EntryMap::iterator iter=_entries.find(hash);
	if (iter!=end()) {
		return iter->second;
	}
	return NULL;*/
	for (size_t i=0; i<_entries.size(); ++i) {
		Entry& e=_entries[i];
		if (e.hash().compare(hash)==0) {
			return &e;
		}
	}
	return NULL;
}

const Entry* SDPK2::findEntry(const MD5Hash& hash) const {
	for (size_t i=0; i<_entries.size(); ++i) {
		const Entry& e=_entries[i];
		if (e.hash().compare(hash)==0) {
			return &e;
		}
	}
	return NULL;
}

void SDPK2::clearEntries() {
	_entries.clear();
};

const char* __comp_methods[]={
	"UNKNOWN",
	"zlib",
	"lzx "
};

void SDPK2::deserializeInfo(Stream* stream) {
	clear();
	int temp;
	stream->read(&temp, 4);
	debug_assertp(temp==0x52415350 /*50534152 "PSAR" */, this, "unrecognized header");
	temp=stream->readShort();
	debug_assertp(temp==1, this, "TODO: unrecognized version");
	temp=stream->readShort();
	debug_assertp(temp==4, this, "TODO: unrecognized _unk");
	stream->read(&temp, 4);
	for (unsigned int i=COMPMETHOD_FIRST; i<=COMPMETHOD_LAST; ++i) {
		if (strncmp((const char*)(&temp), __comp_methods[i], 4)==0) {
			_comp_method=(CompressionMethod)i;
			break;
		}
	}
	/*size_t header_size=(size_t)*/stream->readInt();
	size_t size=(size_t)stream->readInt();
	debug_assertp(size==30, this, "entry_size!=30");
	size=(size_t)stream->readInt();
	_block_size=(size_t)stream->readInt();
	int block_blocksize=stream->readInt();
	debug_assertp(block_blocksize==2, this, "block_block_size!=2");
	_entries.resize(size);
	unsigned int i;
	for (i=0; i<size; ++i) {
		_entries[i].deserialize(stream);
	}
	size_t begin=32+(size*30); // Order is not sequential - positions have to be seeked
	for (i=0; i<size; ++i) {
		stream->seek(begin+(_entries[i].getBlockSizeIndex()*2));
		_entries[i].deserializeBlockSize(stream);
	}
	/*if (stream->pos()!=header_size) {
		printf("(SDPK2) stream position does not match header_size: %lu != %lu\n", stream->pos(), header_size);
		assert(false);
	}*/
}

bool SDPK2::open() {
	if (!_stream) {
		Stream* s=FileStream::readFile(_path);
		if (s) {
			EndianStream* es2=new EndianStream(s, true, DUCT_BIG_ENDIAN);
			_stream=es2;
			deserializeInfo(es2);
		} else {
			printf("ERROR: Failed to open SDPK2 file: %s\n", _path);
			return false;
		}
	}
	return true;
}

void SDPK2::close() {
	if (_stream) {
		Stream* s=_stream->getStream();
		_stream->close();
		delete s;
		delete _stream;
		_stream=NULL;
	}
}

void SDPK2::printInfo(unsigned int tabcount, bool newline) const {
	printf("%.*s[comp_method:%s, path(%lu):\"%s\",\n", tabcount, CONST_TAB_STR, __comp_methods[_comp_method], strlen(_path), _path);
	for (size_t i=0; i<_entries.size(); ++i) {
		_entries[i].printInfo(tabcount+1, true);
	}
	printf("%.*s]%.*s", tabcount, CONST_TAB_STR, (newline) ? 1 : 0, "\n");
}

} // namespace PK2Unpack

