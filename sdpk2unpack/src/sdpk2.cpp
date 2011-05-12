
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unicode/ustream.h>
#include <duct/debug.hpp>
#include <duct/filestream.hpp>
#include <duct/endianstream.hpp>
#include "misc.hpp"
#include "sdpk2.hpp"

namespace PK2Unpack {

// class Entry implementation

void Entry::deserializeId(Stream* stream) {
	_id=(unsigned int)stream->readInt();
}

void Entry::serializeId(Stream* stream) const {
	stream->writeInt(_id);
}

void Entry::deserializeInfo(Stream* stream) {
	_dirid=(unsigned int)stream->readInt();
	_id=(unsigned int)stream->readInt();
	_i1=stream->readInt();
	_i2=stream->readInt();
	_i3=stream->readInt();
}

void Entry::serializeInfo(Stream* stream) const {
	stream->writeInt(_dirid);
	stream->writeInt(_id);
	stream->writeInt(_i1);
	stream->writeInt(_i2);
	stream->writeInt(_i3);
}

//void* Entry::read() const;
//void Entry::write();

void Entry::printInfo(unsigned int tabcount, bool newline) const {
	char flags[3]={'-'};
	if (_flags!=ENTRYFLAG_NONE) {
		int i=0;
		if (_flags&ENTRYFLAG_TABLE) flags[i++]='T';
		if (_flags&ENTRYFLAG_COMPRESSED) flags[i++]='C';
		if (_flags&ENTRYFLAG_UNCOMPRESSED) flags[i++]='U';
	}
	printf("%.*s%s[id: %-5u, dir: %-5u, flags: %.*s,", tabcount, CONST_TAB_STR, (_type==ENTRYTYPE_DIR) ? "dir " : "file", _id, _dirid, 3, flags);
	printf("i1: %d, i2: %d, i3: %d]", _i1, _i2, _i3);
	if (newline) {
		printf("\n");
	}
}

// class IDSet implementation

void IDSet::deserialize(Stream* stream) {
	release();
	_count=(unsigned int)stream->readInt();
	_unk=(unsigned char)stream->readByte();
	_data=(int*)malloc(sizeof(int)*_count);
	debug_assertp(_data!=NULL, this, "failed to allocate buffer");
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

void IDSet::printInfo(unsigned int tabcount, bool newline) const {
}

// class NameSet implementation

void NameSet::deserialize(Stream* stream) {
	release();
	_i1=stream->readInt();
	_i2=stream->readInt();
	_count=(unsigned int)stream->readInt();
	_offsets=(int*)malloc(sizeof(int)*_count);
	unsigned int i;
	for (i=0; i<_count; ++i) {
		_offsets[i]=(unsigned int)stream->readInt();
	}
	_names_size=(unsigned int)stream->readInt();
	_data=(char**)=malloc(sizeof(char*)*_count);
	debug_assertp(_data!=NULL, this, "failed to allocate buffer");
	size_t size;
	for (i=0; i<_count; ++i) {
		size=(i+1==_count) ? _offsets[i+1]-_offsets[i] : _names_size-_offsets[i];
		size*=sizeof(char);
		_data[i]=(char*)malloc(size);
		debug_assertp(_data[i]!=NULL, this, "failed to allocate buffer");
		stream->read((void*)_data[i], size);
	}
}

void NameSet::serialize(Stream* stream) const {
	stream->writeInt(_i1);
	stream->writeInt(_i2);
	stream->writeInt(_count);
	unsigned int i;
	if (_offsets) {
		for (i=0; i<_count; ++i) {
			stream->writeInt(_offsets[i]);
		}
	}
	stream->writeInt(_names_size);
	if (_data) {
		size_t size;
		for (i=0; i<_count; ++i) {
			if (_data[i]) {
				size=(i+1==_count) ? _offsets[i+1]-_offsets[i] : _names_size-_offsets[i];
				size*=sizeof(char);
				stream->write(_data[i], size);
			} else {
				debug_printp_source(this, "no character data for index");
				stream->writeByte(0x00);
			}
		}
	}
}

void NameSet::printInfo(unsigned int tabcount, bool newline) const {
	printf("%.*s[_i1:%u, _i2:%u, names_size:%u, names(%u):{\n", tabcount, CONST_TAB_STR, _i1, _i2, _names_size, _count);
	for (unsigned int i=0; i<_count; ++i) {
		if (_data[i]) {
			printf("%.*s\"%s\",\n", tabcount+1, CONST_TAB_STR, _data[i]);
		} else {
			printf("%.*s(nil),\n", tabcount+1, CONST_TAB_STR);
		}
	}
	printf("%.*s}]%.*s", tabcount, CONST_TAB_STR, (newline) ? 1 : 0, "\n");
}

// class FileInfo implementation

void FileInfo::deserialize(Stream* stream) {
	_dir_id=(unsigned int)stream->readInt();
	_id=(unsigned int)stream->readInt();
	_i1=stream->readInt();
	_i2=stream->readInt();
	_i3=stream->readInt();
}

void FileInfo::serialize(Stream* stream) const {
	stream->writeInt(_dir_id);
	stream->writeInt(_id);
	stream->writeInt(_i1);
	stream->writeInt(_i2);
	stream->writeInt(_i3);
}

void FileInfo::printInfo(unsigned int tabcount, bool newline) const {
	printf("%.*s[id:%-4u, dir_id:%-4u, _i1:%-6d, _i2:%-6d, _i3:%-6d]%.*s", tabcount, CONST_TAB_STR, _id, _dir_id, _i1, _i2, _i3, (newline) ? 1 : 0, "\n");
}

// class FileSet implementation

void FileSet::deserialize(Stream* stream) {
	_count=(unsigned int)stream->readInt();
	_data.resize(_count);
	for (unsigned int i=0; i<_count; ++i) {
		_data[i].deserialize(stream);
	}
}

void FileSet::serialize(Stream* stream) const {
	stream->writeInt(_count);
	for (unsigned int i=0; i<_count; ++i) {
		_data[i].serialize(stream);
	}
}

void FileSet::printInfo(unsigned int tabcount, bool newline) const {
	printf("%.*s[files(%u):{\n", tabcount, CONST_TAB_STR, _count);
	for (unsigned int i=0; i<_count; ++i) {
		_data[i].printInfo(tabcount+1, true);
	}
	printf("%.*s}]%.*s", tabcount, CONST_TAB_STR, (newline) ? 1 : 0, "\n");
}

// class Archive implementation

Entry* Archive::findEntry(unsigned int id) {
	EntryMap::iterator iter=find(id);
	if (iter!=end()) {
		return iter->second;
	}
	return NULL;
}

const Entry* Archive::findEntry(unsigned int id) const {
	EntryMap::const_iterator iter=find(id);
	if (iter!=end()) {
		return iter->second;
	}
	return NULL;
}

void Archive::clearEntries() {
	EntryMap::iterator iter;
	for (iter=begin(); iter!=end(); ++iter) {
		delete iter->second;
	}
	_entries.clear();
};

void Archive::deserializeInfo(Stream* stream) {
	clear();
	_compressed.deserialize(stream);
	_uncompressed.deserialize(stream);
	/*unsigned int id, i;
	unsigned int count=stream->readInt();
	_ids1_unk=stream->readByte();
	EntryMap::const_iterator iter;
	for (i=0; i<count; ++i) {
		id=stream->readInt();
		if (id!=0xFFFFFFFF) {
			iter=find(id);
			if (iter!=end()) {
				_entries[id]=new Entry(id, ENTRYFLAG_TABLE|ENTRYFLAG_COMPRESSED);
			} else {
				printf("WARNING: Entry ID %d was already added (IDSet 1)\n", id);
			}
		}
	}*/
	
	// FIXME MOAR
}

UnicodeString __arch_ext_sdmd2(".sdmd2");
UnicodeString __arch_ext_sdpk2(".sdpk2");

bool Archive::open() {
	if (!_stream) {
		UnicodeString path(_pathkey);
		path.append(__arch_ext_sdmd2);
		Stream* s=FileStream::readFile(_pathkey);
		EndianStream es(s, false, DUCT_BIG_ENDIAN);
		if (s) {
			deserializeInfo(&es);
			es.close();
			s->close();
			delete s;
		} else {
			printf("ERROR: Failed to open table: ");
			std::cout<<path<<std::endl;
			return false;
		}
		path.setTo(_pathkey);
		path.append(__arch_ext_sdpk2);
		s=FileStream::readFile(_pathkey);
		if (s) {
			EndianStream* es2=new EndianStream(s, true, DUCT_BIG_ENDIAN);
			_stream=es2;
		} else {
			printf("ERROR: Failed to open archive: ");
			std::cout<<path<<std::endl;
			return false;
		}
	}
	return true;
}

void Archive::close() {
	if (_stream) {
		Stream* s=_stream->getStream();
		_stream->close();
		delete s;
		delete _stream;
	}
}

void printInfo(unsigned int tabcount, bool newline) const {
}

} // namespace PK2Unpack

