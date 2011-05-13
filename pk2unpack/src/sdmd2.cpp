
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unicode/ustream.h>
#include <duct/debug.hpp>
#include <duct/filestream.hpp>
#include <duct/endianstream.hpp>
#include "misc.hpp"
#include "sdmd2.hpp"

namespace PK2Unpack {

// class IDSet implementation

void IDSet::deserialize(Stream* stream) {
	release();
	_count=(unsigned int)stream->readInt();
	_unk=(unsigned char)stream->readByte();
	_data=(int*)malloc(sizeof(int)*_count);
	debug_assertp(_data, this, "failed to allocate buffer");
	unsigned int i;
	for (i=0; i<_count; ++i) {
		_data[i]=stream->readInt();
	}
}

void IDSet::serialize(Stream* stream) const {
	stream->writeInt(_count);
	stream->writeByte(_unk);
	if (_data) {
		unsigned int i;
		for (i=0; i<_count; ++i) {
			stream->writeInt(_data[i]);
		}
	}
}

void IDSet::printInfo(const char* name, unsigned int tabcount, bool newline) const {
	printf("%.*s%s[_unk:%u, data(%u):{\n", tabcount, CONST_TAB_STR, name, _unk, _count);
	unsigned int i, c;
	if (_data) {
		for (i=0, c=0; i<_count; ++i) {
			if (_data[i]!=-1) {
				if (c==0) {
					printf("%.*s", tabcount+1, CONST_TAB_STR);
				}
				printf("(%-3u : %-4d) ", i, _data[i]);
				++c;
			}
			if (c==8) {
				c=0;
				printf("\n");
			}
		}
		if (c!=0) {
			printf("\n");
		}
	}
	printf("%.*s}]%.*s", tabcount, CONST_TAB_STR, (newline) ? 1 : 0, "\n");
}

// class FileInfo implementation

void FileInfo::deserialize(Stream* stream) {
	_dir_index=(unsigned int)stream->readInt();
	_index=(unsigned int)stream->readInt();
	/*_i1=(unsigned int)stream->readInt();
	_i2=stream->readInt();*/
	_dc.deserialize(stream, 8);
	_time_modified=(time_t)stream->readInt();
}

void FileInfo::serialize(Stream* stream) const {
	stream->writeInt(_dir_index);
	stream->writeInt(_index);
	_dc.serialize(stream);
}

DataFormat __fmt_temp[]={
	{FORMATTYPE_UINT,	0, DUCT_BIG_ENDIAN},
	{FORMATTYPE_SKIP,	4, NULL},
	//{FORMATTYPE_UINT,	0, DUCT_BIG_ENDIAN},
	{FORMATTYPE_SET,	0, NULL},
	{FORMATTYPE_HEX,	0, NULL},
	{NULL, NULL, NULL}
};

char __time_buf[80];

void FileInfo::printInfo(unsigned int tabcount, bool newline) const {
	/*printf("%.*s[index:%4u, dir_index:%4u, _i1:%12u, _i2:%12d, _i3:%12d]%.*s", tabcount, CONST_TAB_STR,
	_index, _dir_index, _i1, _i2, _i3, (newline) ? 1 : 0, "\n");*/
	struct tm* ts=localtime(&_time_modified);
	strftime(__time_buf, sizeof(__time_buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
	printf("%.*s[index:%4u, dir_index:%4u, time_modified:%s, _dc(%lu):{", tabcount, CONST_TAB_STR, _index, _dir_index, __time_buf, _dc.getSize());
	printf("}#%d", _dc.print(__fmt_temp));
	printf("]%.*s", (newline) ? 1 : 0, "\n");
}

// class EntryInfoSet implementation

void EntryInfoSet::deserialize(Stream* stream) {
	release();
	_i1=stream->readInt();
	_i2=stream->readInt();
	_name_count=(size_t)stream->readInt();
	_offsets=(unsigned int*)malloc(sizeof(unsigned int)*_name_count);
	size_t i;
	for (i=0; i<_name_count; ++i) {
		_offsets[i]=(unsigned int)stream->readInt();
	}
	_names_size=(unsigned int)stream->readInt();
	_names=(char**)malloc(sizeof(char*)*_name_count);
	debug_assertp(_names, this, "failed to allocate buffer");
	size_t size;
	for (i=0; i<_name_count; ++i) {
		size=(i+1==_name_count) ? _names_size-_offsets[i] : _offsets[i+1]-_offsets[i];
		size*=sizeof(char);
		_names[i]=(char*)malloc(size);
		debug_assertp(_names[i], this, "failed to allocate buffer");
		stream->read((void*)_names[i], size);
	}
	size=(size_t)stream->readInt();
	_files.resize(size);
	for (i=0; i<size; ++i) {
		_files[i].deserialize(stream);
	}
}

void EntryInfoSet::serialize(Stream* stream) const {
	stream->writeInt(_i1);
	stream->writeInt(_i2);
	stream->writeInt(_name_count);
	size_t i;
	if (_offsets) {
		for (i=0; i<_name_count; ++i) {
			stream->writeInt(_offsets[i]);
		}
	}
	stream->writeInt(_names_size);
	if (_names) {
		size_t size;
		for (i=0; i<_name_count; ++i) {
			if (_names[i]) {
				size=(i+1==_name_count) ? _names_size-_offsets[i] : _offsets[i+1]-_offsets[i];
				size*=sizeof(char);
				stream->write(_names[i], size);
			} else {
				debug_printp_source(this, "no character data for index");
				stream->writeByte(0x00);
			}
		}
	}
	stream->writeInt(_files.size());
	for (i=0; i<_files.size(); ++i) {
		_files[i].serialize(stream);
	}
}

void EntryInfoSet::printInfo(const char* name, unsigned int tabcount, bool newline) const {
	printf("%.*s%s[_i1:%u, _i2:%u, names_size:%lu, names(%lu):{\n", tabcount, CONST_TAB_STR, name, _i1, _i2, _names_size, _name_count);
	size_t i;
	for (i=0; i<_name_count; ++i) {
		if (_names[i]) {
			printf("%.*s[%4lu: \"%s\"]\n", tabcount+1, CONST_TAB_STR, i, _names[i]);
		} else {
			printf("%.*s[%4lu: (nil)]\n", tabcount+1, CONST_TAB_STR, i);
		}
	}
	printf("%.*s},\n%.*sfiles(%lu):{\n", tabcount, CONST_TAB_STR, tabcount, CONST_TAB_STR, _files.size());
	for (i=0; i<_files.size(); ++i) {
		_files[i].printInfo(tabcount+1, true);
	}
	printf("%.*s}]%.*s", tabcount, CONST_TAB_STR, (newline) ? 1 : 0, "\n");
}

// class SDMD2 implementation

void SDMD2::deserialize(Stream* stream) {
	_ids1.deserialize(stream);
	_ids2.deserialize(stream);
	_entryinfo.deserialize(stream);
}

bool SDMD2::load() {
	Stream* s=FileStream::readFile(_path);
	if (s) {
		EndianStream es(s, false, DUCT_BIG_ENDIAN);
		deserialize(&es);
		es.close();
		s->close();
		delete s;
	} else {
		printf("ERROR: Failed to open SDMD2 file: %s\n", _path);
		return false;
	}
	return true;
}

void SDMD2::printInfo(unsigned int tabcount, bool newline) const {
	printf("%.*sSDMD2[path(%lu):\"%s\",\n", tabcount, CONST_TAB_STR, strlen(_path), _path);
	tabcount++;
	_ids1.printInfo("IDS1", tabcount, false);
	printf(",\n");
	_ids2.printInfo("IDS2", tabcount, false);
	printf(",\n");
	_entryinfo.printInfo("EntryInfoSet", tabcount, true);
	tabcount--;
	printf("%.*s]%.*s", tabcount, CONST_TAB_STR, (newline) ? 1 : 0, "\n");
}

} // namespace PK2Unpack

