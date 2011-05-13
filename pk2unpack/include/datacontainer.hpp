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

#ifndef _PK2UNPACK_DATACONTAINER_HPP_
#define _PK2UNPACK_DATACONTAINER_HPP_

#include <stdlib.h>
#include <duct/debug.hpp>
#include <duct/stream.hpp>

namespace PK2Unpack {

using namespace duct;

/**
	Format types for the DataFormat struct and DataContainer class.
*/
enum FormatType {
	/** NULL format type. This is used to end an array of DataFormats. */
	FORMATTYPE_NULL=NULL,
	/** Singed byte. */
	FORMATTYPE_SBYTE=1,
	/** Unsigned byte. */
	FORMATTYPE_UBYTE,
	/** Character. */
	FORMATTYPE_CHAR,
	/** Singed short (2-byte). */
	FORMATTYPE_SSHORT,
	/** Unsigned short (2-byte). */
	FORMATTYPE_USHORT,
	/** Signed integer (4-byte). */
	FORMATTYPE_SINT,
	/** Unsigned integer (4-byte). */
	FORMATTYPE_UINT,
	/** Signed long (8-byte). */
	FORMATTYPE_SLONG,
	/** Unsigned long (8-byte). */
	FORMATTYPE_ULONG,
	/** Floating-point number (4-byte). */
	FORMATTYPE_FLOAT,
	/** Double precision floating-point number (8-byte). */
	FORMATTYPE_DOUBLE,
	/** C string (stops at maxsize or nothing with maxsize=0). */
	FORMATTYPE_STRING,
	/** C string (stops at NULL or maxsize). */
	FORMATTYPE_STRING_NULLTERM,
	/** Plain/hexadecimal format (form is "12 AB CD"). */
	FORMATTYPE_HEX,
	/** Plain/hexadecimal format (form is "12ABCD"). */
	FORMATTYPE_HEX_SPACELESS,
	/** Plain/hexadecimal format (form is "12 ab cd"). */
	FORMATTYPE_HEX_LOWER,
	/** Plain/hexadecimal format (form is "12abcd"). */
	FORMATTYPE_HEX_LOWER_SPACELESS,
	/** Skip maxsize. */
	FORMATTYPE_SKIP,
	/** Set the position to maxsize. */
	FORMATTYPE_SET
};

typedef struct {
	unsigned int type;
	size_t maxsize;
	int endian;
} DataFormat;

/**
	Data container with multiple output options.
	A container takes ownership of any data pointer given to it.
*/
class DataContainer {
public:
	/**
		Constructor.
	*/
	DataContainer() : _size(0), _data(NULL) {
	};
	/**
		Constructor with data and size.
		@param data The data pointer.
		@param size The container's size.
	*/
	DataContainer(void* data, size_t size) : _size(size), _data((char*)data) {
	};
	/**
		Destructor.
	*/
	~DataContainer() {
		release();
	};
	/**
		Set the container's size.
		Note that if this is changed without changing the data pointer, it will not reflect that actual size of the container's data.
		@returns Nothing.
		@param size The new size.
	*/
	void setSize(size_t size) {
		_size=size;
	};
	/**
		Get the size of the container's data.
		@returns The container's size.
	*/
	size_t getSize() const {
		return _size;
	};
	/**
		Set the container's data.
		The class takes ownership of the given pointer.
		@returns Nothing.
		@param data The new data pointer.
	*/
	void setData(void* data) {
		release();
		_data=(char*)data;
	};
	/**
		Set the container's data and size.
		The class takes ownership of the given pointer.
		@returns Nothing.
		@param data The new data pointer.
		@param size The new size.
	*/
	void setData(void* data, size_t size) {
		release();
		_data=(char*)data;
		_size=size;
	};
	/**
		Get the container's data.
		@returns The container's data (may be NULL).
	*/
	void* getData() {
		return _data;
	};
	const void* getData() const {
		return _data;
	};
	/**
		Release the container's data.
		The container's size is set to 0.
		@returns Nothing.
	*/
	void release() {
		if (_data) {
			free(_data);
			_data=NULL;
		}
		_size=0;
	};
	/**
		Deserialize data from the given stream.
		@returns Nothing.
		@param stream The stream to read from.
		@param size Number of bytes to read (also sets container's size).
	*/
	void deserialize(Stream* stream, size_t size) {
		void* d=malloc(size);
		debug_assertp(d!=NULL, this, "failed to allocate buffer");
		stream->read(d, size);
		setData(d, size);
	};
	/**
		Serialize the container's data to the given stream.
		@returns Nothing.
		@param stream The stream to write to.
	*/
	void serialize(Stream* stream) const {
		if (_data && _size>0) {
			size_t sw=stream->write(_data, _size);
			debug_assertp(sw!=_size, this, "written size does not equal container size");
		}
	};
	/**
		Print the container's data with the given format.
		@returns 0 if the entire format was printed and covered the entire container, -2 if nothing was printed (no data or no format), -1 if the format was partially printed (insufficient data to complete), or the size left (greater than 0) if the format was printed but did not cover the entire size of the container.
		@param format The array of format elements. The last element must use FORMATTYPE_NULL.
	*/
	int print(const DataFormat* format) const;
	
protected:
	size_t _size;
	char* _data;
	
	bool print_number(const DataFormat& f, size_t& pos, int type) const;
	bool print_float(const DataFormat& f, size_t& pos) const;
	bool print_string(const DataFormat& f, size_t& pos) const;
	void print_hex(const DataFormat& f, size_t& pos) const;
};

} // namespace PK2Unpack

#endif // _PK2UNPACK_DATACONTAINER_HPP_

