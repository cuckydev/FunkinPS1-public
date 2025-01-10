/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Compress.cpp -
	Decompression
*/

#include "Boot/Compress.h"

#include <CKSDK/TTY.h>

namespace Compress
{
	// Decompression functions
	KEEP void Decompress(const void *in, void *out)
	{
		// Based off of https://github.com/flamewing/mdcomp/blob/master/src/asm/Comper.asm
		INLINE_ASM(R"(
			# a0 = in
			# a1 = out
			#
			# t0 = description
			# t1 = displacement
			# t2 = copy length
			# t3 = bits counter
			# t4 = 0xFFFF
			# t5 = last read word, kept stale for RLE
			# t6 = -4
			# t7 = duff's address
			# t8 = duff's offset
			# t9 = temporary

			.set noreorder

			.set DUFF_BITS, 6

			.macro DUFF_MATCH i
				lw    $t5, 0($a2)
				addiu $a2, 4
				sw    $t5, \i($a1)
			.endm
			.macro DUFF_RLE i
				sw    $t5, \i($a1)
			.endm

			.Lnewblock:
				lui   $t4, 0xFFFF  # OR'd to sign a word
				li    $t6, -4      # RLE displacement

				lw    $t0, 0($a0)  # Fetch description field
				addi  $a0, 4
				li    $t3, 32 - 1  # Set bits counter

			.Lmainloop:
				bltz  $t0, .Lflag  # Roll description field
				addu  $t0, $t0     
				lw    $t5, 0($a0)  # Otherwise, do uncompressed data 
				addiu $a0, 4
				sw    $t5, 0($a1)
				addiu $a1, 4
				bnez  $t3, .Lmainloop  # If bits counter remains, parse the next bit
				addiu $t3, -1
				b     .Lnewblock       # Start a new block
			# ----------------------------------------------------------------------
			.Lflag:
				lw    $t2, 0($a0)    # Load displacement and copy length
				addiu $a0, 4
				beqz  $t2, .Lend
				or    $t1, $t2, $t4  # Make displacement signed
				beq   $t1, $t6, .Lrle
				srl   $t2, 16        # Get copy length
				addu  $a2, $a1, $t1  # Load start copy address

			.if (DUFF_BITS > 0)
				la    $t7, .Lloop  # Get duff's address
				not   $t8, $t2
				andi  $t8, (1 << DUFF_BITS) - 1
				
				sll   $t9, $t8, 2
				sll   $t8, 4
				subu  $t8, $t9
				
				addu  $t7, $t8
				subu  $a1, $t9
				
				jr    $t7            # Jump to duff's address
				srl   $t2, DUFF_BITS  # Shift copy length accordingly
			.endif

			.Lloop:
			.set DUFF_COUNTER, 0
			.rept (1 << DUFF_BITS)
				DUFF_MATCH DUFF_COUNTER  # Copy match
			.set DUFF_COUNTER, DUFF_COUNTER + 4
			.endr
				addiu $a1, (4 << DUFF_BITS)

				bnez  $t2, .Lloop      # Repeat
				addiu $t2, -1
				bnez  $t3, .Lmainloop  # If bits counter remains, parse the next bit
				addiu $t3, -1

				b     .Lnewblock       # Start a new block
				nop

			.Lrle:
			.if (DUFF_BITS > 0)
				la    $t7, .Lrleloop  # Get duff's address
				not   $t8, $t2
				andi  $t8, (1 << DUFF_BITS) - 1
				sll   $t8, 2
				
				subu  $a1, $t8
				addu  $t7, $t8
				
				jr    $t7            # Jump to duff's address
				srl   $t2, DUFF_BITS  # Shift copy length accordingly
			.endif

			.Lrleloop:
			.set DUFF_COUNTER, 0
			.rept (1 << DUFF_BITS)
				DUFF_RLE DUFF_COUNTER  # Copy RLE
			.set DUFF_COUNTER, DUFF_COUNTER + 4
			.endr
				addiu $a1, (4 << DUFF_BITS)

				bnez  $t2, .Lrleloop   # Repeat
				addiu $t2, -1
				bnez  $t3, .Lmainloop  # If bits counter remains, parse the next bit
				addiu $t3, -1

				b     .Lnewblock       # Start a new block
				nop
				
			.Lend:
			.set reorder
		)" ::: "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9", "a0", "a1");
	}
}
