#pragma once

/**
 * \file sha1.h
 * \brief compact implementation of SHA-1
 */

/// state for storing the hash
typedef struct {
    unsigned buffer[16]; ///< buffer with temporary data
    unsigned hash[5]; ///< current hash value
    unsigned count; ///< number of bytes processed so far
} sha1_t;

#define SHA1_INIT_0 0x67452301 ///< first initialization value
#define SHA1_INIT_1 0xefcdab89 ///< second initialization value
#define SHA1_INIT_2 0x98badcfe ///< third initialization value
#define SHA1_INIT_3 0x10325476 ///< fourth initialization value
#define SHA1_INIT_4 0xc3d2e1f0 ///< fifth initialization value

/**
 * \brief initialize SHA-1 state as a constant literal for static initialization
 * \return static literal expression
 */
#define SHA1() { \
	.hash = { SHA1_INIT_0, SHA1_INIT_1, SHA1_INIT_2, SHA1_INIT_3, SHA1_INIT_4 } \
}

/**
 * \brief initialize the state
 *
 * \param s state to initialize
 */
void sha1_init(sha1_t *s);

#if SHA1_IMPL
	#include <string.h>

	/**
	 * \brief clear the buffer in the state
	 *
	 * \param s state where to clear the buffer
	 */
	static inline void clear_buffer(sha1_t *s) {
		if (! s) { return; }
		memset(s->buffer, 0, sizeof(s->buffer));
	}

	void sha1_init(sha1_t *s) {
		if (! s) { return; }

		clear_buffer(s);
		s->hash[0] = SHA1_INIT_0;
		s->hash[1] = SHA1_INIT_1;
		s->hash[2] = SHA1_INIT_2;
		s->hash[3] = SHA1_INIT_3;
		s->hash[4] = SHA1_INIT_4;
		s->count = 0;
	}
#endif

/**
 * \brief add some bytes to the hash
 *
 * \param s state of hash calculation
 * \param begin iterator to start of data
 * \param end iterator to end of data
 */
void sha1_append(sha1_t *s, const char *begin, const char *end);

#if SHA1_IMPL
	/**
	 * \brief rotate bits to the right
	 *
	 * \attention bits must be between 0 and 32
	 *
	 * \param VALUE value to rotate
	 * \param BITS number of bits to rotate
	 * \return rotated value
	 */
	static inline unsigned rotl(unsigned value, unsigned bits) {
		return (value << bits) | (value >> (32 - bits));
	}

	/**
	 * \brief first sha function to build the mixture of three words
	 * \param state array of which entries 1, 2 and 3 will be used
	 * \return mixed up word
	 */
	static inline unsigned f1(const unsigned *state) {
		return (state[1] & state[2]) | (~state[1] & state[3]);
	}

	/**
	 * \brief second sha function to build the mixture of three words
	 * \param state array of which entries 1, 2 and 3 will be used
	 * \return mixed up word
	 */
	static inline unsigned f2(const unsigned *state) {
		return state[1] ^ state[2] ^ state[3];
	}

	/**
	 * \brief third sha function to build the mixture of three words
	 * \param state array of which entries 1, 2 and 3 will be used
	 * \return mixed up word
	 */
	static inline unsigned f3(const unsigned *state) {
		return (state[1] & state[2]) | (state[1] & state[3]) | (state[2] & state[3]);
	}

	/**
	 * \brief perform one round in the block encoding
	 *
	 * \param s state of the sha encoding
	 * \param t round number
	 * \param state temporary buffer for the block encoding
	 * \param k constant to shuffle the bits better
	 * \param f function to perform some mixing on three blocks of the state
	 */
	static inline void do_round(
		sha1_t *s,
		int t,
		unsigned *state,
		unsigned k,
		unsigned (*f)(const unsigned *)
	) {
		if (! s) { return; }

		int ss = t & 0xf;
		if (t >= sizeof(s->buffer)/sizeof(*s->buffer)) {
			s->buffer[ss] = rotl(
				s->buffer[(ss + 13) & 0xf] ^
				s->buffer[(ss + 8) & 0xf] ^
				s->buffer[(ss + 2) & 0xf] ^
				s->buffer[ss],
				1
			);
		}
		unsigned tmp = rotl(state[0], 5) + f(state) + state[4] + s->buffer[ss] + k;
		state[4] = state[3];
		state[3] = state[2];
		state[2] = rotl(state[1], 30);
		state[1] = state[0];
		state[0] = tmp;
	}

	/**
	 * \brief encode one block and calculate a new hash
	 * \param s state that contains the block and the current hash
	 */
	static inline void encode_block(sha1_t *s) {
		if (! s) { return; }

		unsigned state[sizeof(s->hash)/sizeof(*s->hash)];
		memcpy(state, s->hash, sizeof(state));
		for (int t = 0; t < 20; ++t) { do_round(s, t, state, 0x5a827999, f1); }
		for (int t = 20; t < 40; ++t) { do_round(s, t, state, 0x6ed9eba1, f2); }
		for (int t = 40; t < 60; ++t) { do_round(s, t, state, 0x8f1bbcdc, f3); }
		for (int t = 60; t < 80; ++t) { do_round(s, t, state, 0xca62c1d6, f2); }
		for (int i = 0; i < sizeof(s->hash)/sizeof(*s->hash); ++i) { s->hash[i] += state[i]; }
		clear_buffer(s);
	}

	void sha1_append(sha1_t *s, const char *begin, const char *end) {
		if (! s) { return; }

		static const unsigned mults[] = { 0x01000000, 0x00010000, 0x00000100, 0x00000001 };

		while (begin < end) {
			int word_index = (s->count % sizeof(s->buffer)) / sizeof(*s->buffer);
			int byte_index = s->count % sizeof(*s->buffer);
			s->buffer[word_index] += (*begin++ & 0xff) * mults[byte_index];
			if (++s->count % sizeof(s->buffer) == 0) { encode_block(s); }
		}
	}
#endif

/**
 * \brief finish the hash calculation and store the result
 *
 * \attention result must hold space for at least 20 bytes
 *
 * \param s state of hash calculation
 * \param result iterator to store the hash
 */
void sha1_finish(sha1_t *s, char *result);

#if SHA1_IMPL
	void sha1_finish(sha1_t *s, char *result) {
		if (! s) { return; }

		unsigned count = s->count;
		int padding = sizeof(s->buffer) - (s->count % sizeof(s->buffer)) - 2 * sizeof(int);
		if (padding <= 0) { padding += sizeof(s->buffer); }
		char ch = (char) 0x80;
		for (; padding > 0; --padding) { sha1_append(s, &ch, &ch + 1); ch = 0x00; }

		{
			unsigned j = count * 8;
			char len[2 * sizeof(int)];
			for (int i = sizeof(len) - 1; i >= 0; --i) { len[i] = (char) j; j >>= 8; }
			sha1_append(s, len, len + sizeof(len));
		}

		if (result) {
			for (unsigned *i = s->hash; i < s->hash + sizeof(s->hash)/sizeof(*s->hash); ++i) {
				unsigned v = *i;
				for (int j = sizeof(unsigned) - 1; j >= 0; --j) {
					result[j] = (char) v;
					v >>= 8;
				}
				result += sizeof(unsigned);
			}
		}
	}
#endif
