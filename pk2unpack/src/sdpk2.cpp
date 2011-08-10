/*
Copyright (c) 2011 Tim Howard

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

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

bool MD5Hash::set(const char* str) {
	if (strnlen(str, 32)==32) {
		unsigned short d[4];
		for (int i=0; i<16; i+=4) {
			if (sscanf(str+(i*2), "%2hx%2hx%2hx%2hx", d, d+1, d+2, d+3)!=4) {
				clear();
				return false;
			}
			_data[i]=(unsigned char)d[0];
			_data[i+1]=(unsigned char)d[1];
			_data[i+2]=(unsigned char)d[2];
			_data[i+3]=(unsigned char)d[3];
		}
		return true;
	}
	return false;
}

const char __hash_null[16]={0x00};

bool MD5Hash::isNull() const {
	return memcmp(_data, __hash_null, 16)==0;
}

void MD5Hash::getExisting(char* str, bool nullterm) const {
	for (int i=0; i<16; i+=4) {
		sprintf(str+(i*2), "%02x%02x%02x%02x", _data[i], _data[i+1], _data[i+2], _data[i+3]);
	}
	if (nullterm) {
		str[32]=0;
	}
}

char* MD5Hash::get(char** out, bool nullterm) const {
	char* str=(char*)malloc((nullterm) ? 33 : 32);
	debug_assertp(str, this, "failed to allocate 33- or 32-byte buffer");
	if (out!=NULL) {
		*out=str;
	}
	return str;
}

void MD5Hash::printInfo(unsigned int tabcount, bool newline) const {
	char str[32];
	getExisting(str, false);
	printf("%.*s[%.*s]%.*s", tabcount, CONST_TAB_STR, 32, str, (newline) ? 1 : 0, "\n");
}

// class Entry implementation

#define __read_buf_size 0x10000
char __read_buf_out[__read_buf_size], __read_buf_in[__read_buf_size];

int Entry::readToStream(Stream* instream, Stream* outstream, const SDPK2& pak) const {
	debug_assertp(pak.getCompressionMethod()==COMPMETHOD_ZLIB, this, "unsupported compression method");
	if (_size>0) {
		z_stream strm;
		strm.zalloc=Z_NULL;
		strm.zfree=Z_NULL;
		strm.opaque=Z_NULL;
		strm.next_in=(Bytef*)Z_NULL;
		strm.avail_in=0;
		strm.next_out=(Bytef*)Z_NULL;
		strm.avail_out=0;
		int status=inflateInit2(&strm, 15);
		debug_assertp(status==Z_OK, this, "failed to init zip stream");
		instream->seek(_offset);
		size_t w_sizeleft, w_size, uc_size, c_size, c_blocksize, uc_blocksize;
		unsigned int b_index=_blocksize_index;
		uc_size=_size;
		c_size=0;
		while (uc_size!=0) {
			uc_blocksize=(uc_size<pak.getBlockSize()) ? uc_size : pak.getBlockSize();
			c_blocksize=pak.getBlockSizeTable()[b_index++];
			c_size+=c_blocksize;
			//printf("begin uc_size=%li uc_blocksize=%lu c_blocksize=%lu\n", uc_size, uc_blocksize, c_blocksize);
			debug_assertp(c_blocksize<=uc_blocksize, this, "compressed block size is larger than uncompressed block size");
			if (c_blocksize==0 || uc_size==c_blocksize) {
				if (c_blocksize==0) {
					c_blocksize=pak.getBlockSize();
				}
				w_sizeleft=c_blocksize;
				while (w_sizeleft!=0) {
					w_size=(w_sizeleft<__read_buf_size) ? w_sizeleft : __read_buf_size;
					instream->read(__read_buf_in, w_size);
					outstream->write(__read_buf_in, w_size);
					w_sizeleft-=w_size;
				}
			} else { // inflate
				w_sizeleft=c_blocksize;
				//printf("inflate block: w_sizeleft=%lu\n", w_sizeleft);
				do {
					w_size=(w_sizeleft<__read_buf_size) ? w_sizeleft : __read_buf_size;
					instream->read(__read_buf_in, w_size);
					strm.next_in=(Bytef*)__read_buf_in;
					strm.avail_in=w_size;
					do {
						strm.next_out=(Bytef*)__read_buf_out;
						strm.avail_out=__read_buf_size;
						status=inflate(&strm, Z_NO_FLUSH);
						//printf("status=%d strm.avail_out=%u strm.avail_in=%u chunk_size=%lu\n", status, strm.avail_out, strm.avail_in, __read_buf_size-(size_t)strm.avail_out);
						debug_assert(status>=Z_OK, "zlib error");
						outstream->write(__read_buf_out, __read_buf_size-strm.avail_out);
					} while (strm.avail_out==0);
					w_sizeleft-=w_size;
				} while (status>=Z_OK && w_sizeleft>0);
				debug_assertp((status==Z_OK || status==Z_STREAM_END), this, "failed to handle/decompress block");
				inflateReset(&strm);
			}
			uc_size-=uc_blocksize;
		}
		inflateEnd(&strm);
		//printf("leftover: %lu: %lu, %lu\n", _offset+c_size-instream->pos(), _offset+c_size, instream->pos());
		debug_assertp(instream->pos()==_offset+c_size, this, "read either too little or too much data");
	}
	return 0;
}

#define __uint40_make(o, b, i) ((o=((size_t)b<<32)|i))
#define __uint40_split(o, b, i) (({b=o&0x00FF00000000; i=o&0x0000FFFFFFFF;}))

void Entry::deserialize(Stream* stream) {
	_hash.deserialize(stream);
	_blocksize_index=stream->readUInt32();
	uint8_t b;
	uint32_t i;
	b=stream->readUInt8();
	i=stream->readUInt32();
	__uint40_make(_size, b, i);
	b=stream->readUInt8();
	i=stream->readUInt32();
	__uint40_make(_offset, b, i);
}

void Entry::serialize(Stream* stream) const {
	_hash.serialize(stream);
	stream->writeUInt32(_blocksize_index);
	uint8_t b;
	uint32_t i;
	__uint40_split(_size, b, i);
	stream->writeUInt8(b);
	stream->writeUInt32(i);
	__uint40_split(_offset, b, i);
	stream->writeUInt8(b);
	stream->writeUInt32(i);
}

void Entry::printInfo(unsigned int tabcount, bool newline) const {
	printf("%.*s[hash:", tabcount, CONST_TAB_STR);
	_hash.printInfo(0, false);
	printf(", blocksize_index:%6u, size:%8lu, offset:%10lu]%.*s", _blocksize_index, _size, _offset, (newline) ? 1 : 0, "\n");
}

// class SDPK2 implementation

Entry* SDPK2::findEntry(const MD5Hash& hash) {
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
	//clear(); // resizing without clearing is faster
	int temp;
	stream->read(&temp, 4);
	debug_assertp(temp==0x52415350 /*50534152 "PSAR" */, this, "unrecognized header");
	temp=stream->readInt16();
	debug_assertp(temp==1, this, "TODO: unrecognized version");
	temp=stream->readInt16();
	debug_assertp(temp==4, this, "TODO: unrecognized _unk");
	stream->read(&temp, 4);
	_comp_method=COMPMETHOD_UNKNOWN;
	for (unsigned int i=COMPMETHOD_FIRST; i<=COMPMETHOD_LAST; ++i) {
		if (strncmp((const char*)(&temp), __comp_methods[i], 4)==0) {
			_comp_method=(CompressionMethod)i;
			break;
		}
	}
	size_t header_size=stream->readUInt32();
	size_t size=stream->readUInt32(); // entry_size
	debug_assertp(size==30, this, "entry_size!=30");
	size=stream->readUInt32(); // entry_count
	_block_size=stream->readUInt32();
	int block_blocksize=stream->readInt32(); // size of elements in comp_block_sizes
	debug_assertp(block_blocksize==2, this, "block_block_size!=2");
	_entries.resize(size);
	unsigned int i;
	for (i=0; i<size; ++i) {
		_entries[i].deserialize(stream);
	}
	unsigned int count=(header_size-stream->pos())/2;
	_c_blocksize_table.resize(count);
	for (i=0; i<count; ++i) {
		_c_blocksize_table[i]=stream->readUInt16();
	}
	if (stream->pos()!=header_size) {
		printf("(SDPK2) stream position does not match header_size: %lu != %lu\n", stream->pos(), header_size);
		assert(false);
	}
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

