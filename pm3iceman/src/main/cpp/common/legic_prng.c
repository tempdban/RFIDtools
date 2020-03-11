//-----------------------------------------------------------------------------
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// LEFIC's obfuscation function
//-----------------------------------------------------------------------------

#include "legic_prng.h"
// the prng is a muxed value from two lsfr a, b
// a is 7bit lsfr
// b is 8bit lsfr
// c keeps track on which step the prng is.
// legic_prng_get_bit() = gets a bit muxed from a and b. 
struct lfsr {
	uint8_t  a;
	uint8_t  b;
	uint32_t c;
} lfsr;

// Normal init is set following variables with a random value IV
// a == iv
// b == iv << 1 | 1
// * someone mentioned iv must be ODD.
// Hack:
// Now we have a special case with iv == 0
// it sets b to 0 aswell to make sure we get a all zero keystream out
// which is used in the initialisation phase sending the IV
// 
void legic_prng_init(uint8_t iv) {
	lfsr.a = iv;
	lfsr.b = 0;  // hack to get a always 0 keystream 
	lfsr.c = 0;
	if(iv)
		lfsr.b = (iv << 1) | 1;
}

void legic_prng_forward(int count) {
	if (count == 0) return;
	
	lfsr.c += count;
	while(count--) {
		// According: http://www.proxmark.org/forum/viewtopic.php?pid=5437#p5437
		lfsr.a = (lfsr.a >> 1 | (lfsr.a ^ lfsr.a >> 6) << 6) & 0x7F;
		lfsr.b = lfsr.b >> 1 | (lfsr.b ^ lfsr.b >> 2 ^ lfsr.b >> 3 ^ lfsr.b >> 7) << 7;
	}
}

uint32_t legic_prng_count() {
	return lfsr.c;
}

uint8_t legic_prng_get_bit() {
	uint8_t idx = 7 - ( (lfsr.a & 4) | (lfsr.a >> 2 & 2) | (lfsr.a >> 4 & 1) );
	return lfsr.b >> idx & 1;
}

uint32_t legic_prng_get_bits(uint8_t len){
	uint32_t a = 0;
	for(uint8_t i = 0; i < len; ++i) {
		a |= legic_prng_get_bit() << i;
		legic_prng_forward(1);
	}
	return a;
}