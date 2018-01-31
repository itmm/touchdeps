#pragma once

/**
 * \file base64.h
 * \brief compact implementation of Base64 encoding (decoding not needed yet)
 */

#if BASE64_IMPL
	#include <stdlib.h>
#endif

/// container to keep the state of encoding
typedef struct {
    unsigned last; ///< last fully encoded block
    unsigned buffer; ///< current partially encoded block
    unsigned count; ///< numbers of bytes in state
} base64_t;

/**
 * \brief initialize encoder state as literal
 *
 * \return encoder state as literal
 */
#define BASE64() {}

/**
 * \brief initialize the encoder state
 *
 * \param b state to populate
 */
static inline void base64_init(base64_t *b) {
	if (b) {
		b->last = b->buffer = b->count = 0;
	}
}

/**
 * \brief add one char to the encoder
 *
 * If a full block is encoded, it could be that data will be written to result
 * (if there is enough space).
 *
 * \todo return end instead of `NULL` on failure
 *
 * \param b encoding state
 * \param ch character to encode
 * \param result iterator to buffer where outputs are written
 * \param end iterator to end of buffer
 * \return iterator to the new result or `NULL` if the buffer is too small
 */
char *base64_add(base64_t *b, char ch, char *result, const char *end);

#if BASE64_IMPL
	/**
	 * \brief write a buffer of one to three bytes encoded as four bytes
	 *
	 * \param buffer buffer that contains the bytes
	 * \param count number of bytes in the buffer
	 * \param result iterator where to store the result
	 * \param end iterator to the end of the buffer
	 * \return iterator to the new beginning or `NULL` if the buffer is too small
	 */
	static char *write_out( unsigned buffer, int count, char *result, const char *end) {
		static const char mapping[] = 
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		if (result) for (int shifter = 3 * 6; shifter >= (4 - count) * 6; shifter -= 6) {
			if (result < end) {
				*result++ = mapping[(buffer >> shifter) % (sizeof(mapping) - 1)];
			} else { return NULL; }
		}
		return result;
	}

	char *base64_add(base64_t *b, char ch, char *result, const char *end) {
		if (! b) { return NULL; }
		b->buffer = (b->buffer << 8) + (ch & 0xff);
		++b->count;
		if (b->count == 3) {
			b->last = b->buffer;
			b->buffer = 0;
		} else if (b->count == 6) {
			result = write_out(b->last, 4, result, end);
			b->last = b->buffer;
			b->buffer = 0;
			b->count = 3;
		}
		return result;
	}
#endif

/**
 * \brief finish the encoding
 *
 * \param b encoding state
 * \param result iterator to buffer where outputs are written
 * \param end iterator to end of buffer
 * \return iterator to new result or `NULL` if the buffer is too small
 */
char *base64_finish(base64_t *b, char *result, const char *end);

#if AH_BASE64_IMPL
	char *base64_finish(base64_t *b, char *result, const char *end) {
		if (! b) { return NULL; }
		switch (b->count) {
			case 0: {
				break;
			}
			case 3: {
				result = write_out(b->last, 4, result, end);
				break;
			}
			case 1:
			case 4: {
				result = base64_add(b, 0, result, end);
				result = base64_add(b, 0, result, end);
				result = write_out(b->last, 2, result, end);
				if (result < end) { *result++ = '='; }
				if (result < end) { *result++ = '='; } else { result = NULL; }
				break;
			}
			case 2:
			case 5: {
				result = base64_add(b, 0, result, end);
				result = write_out(b->last, 3, result, end);
				if (result < end) { *result++ = '='; } else { result = NULL; }
				break;
			}
			default: break;
		}
		return result;
	}
#endif
