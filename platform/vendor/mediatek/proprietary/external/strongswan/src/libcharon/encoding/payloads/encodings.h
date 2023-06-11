/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */


#ifndef ENCODINGS_H_
#define ENCODINGS_H_

typedef enum encoding_type_t encoding_type_t;
typedef struct encoding_rule_t encoding_rule_t;

#include <library.h>

enum encoding_type_t {

	/**
	 * Representing a 4 Bit unsigned int value.
	 *
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 4 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 4 bit forward afterwards.
	 */
	U_INT_4,

	/**
	 * Representing a 8 Bit unsigned int value.
	 *
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 8 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 8 bit forward afterwards.
	 */
	U_INT_8,

	/**
	 * Representing a 16 Bit unsigned int value.
	 *
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 16 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 16 bit forward afterwards.
	 */
	U_INT_16,

	/**
	 * Representing a 32 Bit unsigned int value.
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 32 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 32 bit forward afterwards.
	 */
	U_INT_32,

	/**
	 * represents a RESERVED_BIT used in FLAG-Bytes.
	 *
	 * When generating, the next bit is set to zero and the current write
	 * position is moved one bit forward.
	 * No value is read from the associated data struct.
	 * The current write position is moved 1 bit forward afterwards.
	 *
	 * When parsing, the current read pointer is moved one bit forward.
	 * No value is written to the associated data struct.
	 * The current read pointer is moved 1 bit forward afterwards.
	 */
	RESERVED_BIT,

	/**
	 * represents a RESERVED_BYTE.
	 *
	 * When generating, the next byte is set to zero and the current write
	 * position is moved one byte forward.
	 * No value is read from the associated data struct.
	 * The current write position is moved 1 byte forward afterwards.
	 *
	 * When parsing, the current read pointer is moved one byte forward.
	 * No value is written to the associated data struct.
	 * The current read pointer is moved 1 byte forward afterwards.
	 */
	RESERVED_BYTE,

	FLAG,

	/**
	 * Representating a length field of a payload.
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 16 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 16 bit forward afterwards.
	 */
	PAYLOAD_LENGTH,

	/**
	 * Representating a length field of a header.
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 32 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 32 bit forward afterwards.
	 */
	HEADER_LENGTH,

	/**
	 * Representating a spi size field.
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 8 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 8 bit forward afterwards.
	 */
	SPI_SIZE,

	/**
	 * Representating a spi field.
	 *
	 * When generating the content of the chunkt pointing to
	 * is written.
	 *
	 * When parsing SPI_SIZE bytes are read and written into the chunk pointing to.
	 */
	SPI,

	ATTRIBUTE_FORMAT,

	/**
	 * Representing a 15 Bit unsigned int value used as attribute type
	 * in an attribute transform.
	 *
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 15 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 15 bit forward afterwards.
	 */
	ATTRIBUTE_TYPE,

	/**
	 * Depending on the field of type ATTRIBUTE_FORMAT
	 * this field contains the length or the value of an transform attribute.
	 * Its stored in a 16 unsigned integer field.
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 16 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 16 bit forward afterwards.
	 */
	ATTRIBUTE_LENGTH_OR_VALUE,

	/**
	 * This field contains the length or the value of an configuration attribute.
	 * Its stored in a 16 unsigned integer field.
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 16 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 16 bit forward afterwards.
	 */
	ATTRIBUTE_LENGTH,

	/**
	 * Depending on the field of type ATTRIBUTE_FORMAT
	 * this field is available or missing and so parsed/generated
	 * or not parsed/not generated.
	 *
	 * When generating the content of the chunkt pointing to
	 * is written.
	 *
	 * When parsing SPI_SIZE bytes are read and written into the chunk pointing to.
	 */
	ATTRIBUTE_VALUE,

	/**
	 * Representating a Traffic selector type field.
	 *
	 * When generating it must be changed from host to network order.
	 * The value is read from the associated data struct.
	 * The current write position is moved 16 bit forward afterwards.
	 *
	 * When parsing it must be changed from network to host order.
	 * The value is written to the associated data struct.
	 * The current read pointer is moved 16 bit forward afterwards.
	 */
	TS_TYPE,

	/**
	 * Representating an address field in a traffic selector.
	 *
	 * Depending on the last field of type TS_TYPE
	 * this field is either 4 or 16 byte long.
	 *
	 * When generating the content of the chunkt pointing to
	 * is written.
	 *
	 * When parsing 4 or 16 bytes are read and written into the chunk pointing to.
	 */
	ADDRESS,

	CHUNK_DATA,

	/**
	 * Representating an IKE_SPI field in an IKEv2 Header.
	 *
	 * When generating the value of the u_int64_t pointing to
	 * is written (host and networ order is not changed).
	 *
	 * When parsing 8 bytes are read and written into the u_int64_t pointing to.
	 */
	IKE_SPI,

	ENCRYPTED_DATA,

	PAYLOAD_LIST = 1000 
};

extern enum_name_t *encoding_type_names;

struct encoding_rule_t {

	encoding_type_t type;

	/**
	 * Offset in the data struct.
	 *
	 * When parsing, data are written to this offset of the
	 * data struct.
	 *
	 * When generating, data are read from this offset in the
	 * data struct.
	 */
	u_int32_t offset;
};

#endif 
