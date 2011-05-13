
#include <string.h>
#include <duct/debug.hpp>
#include "datacontainer.hpp"

namespace PK2Unpack {

// class DataContainer implementation

#define __TSHORT 1
#define __TINT 2

bool DataContainer::print_number(const DataFormat& f, size_t& pos, int type) const {
	bool issigned=false;
	bool swap=!f.endian || f.endian!=DUCT_BYTEORDER;
	if (type==__TSHORT) {
		if (pos+2>_size) {
			printf("err");
			return false;
		}
		issigned=f.type==FORMATTYPE_SSHORT;
		short value=*(const short*)(_data+pos);
		if (swap) {
			value=bswap_16(value);
		}
		printf((issigned) ? "%6hds" : "%5huus", value);
		pos+=2;
	} else if (type==__TINT) {
		if (pos+4>_size) {
			printf("err");
			return false;
		}
		issigned=f.type==FORMATTYPE_SINT;
		int value=*(const int*)(_data+pos);
		if (swap) {
			value=bswap_32(value);
		}
		printf((issigned) ? "%11di" : "%10uui", value);
		pos+=4;
	}
	return true;
}

bool DataContainer::print_float(const DataFormat& f, size_t& pos) const {
	return false;
}

bool DataContainer::print_string(const DataFormat& f, size_t& pos) const {
	if (f.type==FORMATTYPE_STRING_NULLTERM) {
		size_t size=strnlen(_data+pos, (f.maxsize>0) ? f.maxsize : _size-pos);
		if (size==_size && _data[_size-1]!=0x00) {
			printf("nz%.*s", (unsigned int)size, _data+pos);
		} else {
			printf("nz%.*s", (unsigned int)size, _data+pos);
		}
		pos+=size;
	} else {
		if (pos+f.maxsize>_size) {
			return false;
		}
		printf("%.*s", (unsigned int)f.maxsize, _data);
		pos+=f.maxsize;
	}
	return true;
}

const char* __hex_fmt_normal="%02X%.*s";
const char* __hex_fmt_lower="%02x%.*s";

void DataContainer::print_hex(const DataFormat& f, size_t& pos) const {
	const char* fmt=(f.type==FORMATTYPE_HEX || f.type==FORMATTYPE_HEX_SPACELESS) ? __hex_fmt_normal : __hex_fmt_lower;
	bool nsp=f.type==FORMATTYPE_HEX_SPACELESS || f.type==FORMATTYPE_HEX_LOWER_SPACELESS;
	size_t count=(f.maxsize!=0) ? pos+f.maxsize : _size-pos;
	size_t i=0;
	while (i++<count && pos!=_size) {
		printf(fmt, (unsigned char)_data[pos++], (nsp || i==count) ? 0 : 1, " ");
	}
}

int DataContainer::print(const DataFormat* format) const {
	if (!_data || _size==0) {
		//debug_printp_source("no data");
		printf("(nil)");
		return -2;
	}
	unsigned int fsize=0;
	const DataFormat* fp=format;
	while (fp->type!=FORMATTYPE_NULL) {
		fsize++; fp++;
	}
	if (fsize==0) {
		//debug_printp_source("no format given");
		printf("(noformat)");
		return -2;
	}
	unsigned int i=0;
	size_t pos=0;
	do {
		const DataFormat& f=format[i];
		if (pos==_size && f.type!=FORMATTYPE_SET) {
			return -1;
		}
		switch (f.type) {
			case FORMATTYPE_SBYTE:
				printf("%db", _data[pos++]);
				break;
			case FORMATTYPE_UBYTE:
				printf("%uub", _data[pos++]);
				break;
			case FORMATTYPE_CHAR:
				printf("'%c'", _data[pos++]);
				break;
			case FORMATTYPE_SSHORT:
			case FORMATTYPE_USHORT:
				//printf("#%lu#", pos);
				if (!print_number(f, pos, __TSHORT)) {
					return -1;
				}
				break;
			case FORMATTYPE_SINT:
			case FORMATTYPE_UINT:
				if (!print_number(f, pos, __TINT)) {
					return -1;
				}
				break;
			case FORMATTYPE_FLOAT:
				print_float(f, pos);
				break;
			case FORMATTYPE_STRING:
			case FORMATTYPE_STRING_NULLTERM:
				if (!print_string(f, pos)) {
					return -1;
				}
				break;
			case FORMATTYPE_HEX:
			case FORMATTYPE_HEX_SPACELESS:
			case FORMATTYPE_HEX_LOWER:
			case FORMATTYPE_HEX_LOWER_SPACELESS:
				print_hex(f, pos);
				break;
			case FORMATTYPE_SKIP:
				pos+=f.maxsize;
				printf(">%lu", f.maxsize);
				break;
			case FORMATTYPE_SET:
				pos=f.maxsize;
				printf("#%lu", f.maxsize);
			default:
				debug_assertp(false, this, "unhandled format type");
				break;
		}
		if (i+1<fsize) {
			printf(", ");
		}
	} while (++i<fsize);
	return (pos==_size) ? 0 : _size-pos;
}

} // namespace PK2Unpack

