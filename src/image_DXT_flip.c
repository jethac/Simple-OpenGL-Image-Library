#include "image_DXT_flip.h"

void FlipDXT1BlockFull(unsigned char *data)
	// A DXT1 block layout is:
	// [0-1] color0.
	// [2-3] color1.
	// [4-7] color bitmap, 2 bits per pixel.
	// So each of the 4-7 bytes represents one line, flipping a block is just
	// flipping those bytes.
	// Note that http://src.chromium.org/viewvc/chrome/trunk/src/o3d/core/cross/bitmap_dds.cc?view=markup&pathrev=21227
	// contains an error in the last line: data[6] = data[5] is a bug!
{
	unsigned char tmp;

	tmp = data[4];
	data[4] = data[7];
	data[7] = tmp;

	tmp = data[5];
	data[5] = data[6];
	data[6] = tmp;
}

void FlipDXT3BlockFull(unsigned char *block)
{
	// A DXT3 block layout is:
	// [0-7]  alpha bitmap, 4 bits per pixel.
	// [8-15] a DXT1 block.

	// We can flip the alpha bits at the byte level (2 bytes per line).
	unsigned char tmp = block[0];
	block[0] = block[6];
	block[6] = tmp;
	tmp = block[1];
	block[1] = block[7];
	block[7] = tmp;
	tmp = block[2];
	block[2] = block[4];
	block[4] = tmp;
	tmp = block[3];
	block[3] = block[5];
	block[5] = tmp;

	// And flip the DXT1 block using the above function.
	FlipDXT1BlockFull(block + 8);
}

void FlipDXT5BlockFull(unsigned char *block)
// From http://src.chromium.org/viewvc/chrome/trunk/src/o3d/core/cross/bitmap_dds.cc?view=markup&pathrev=21227
// Original source contained bugs; fixed here.
{
	// A DXT5 block layout is:
	// [0]    alpha0.
	// [1]    alpha1.
	// [2-7]  alpha bitmap, 3 bits per pixel.
	// [8-15] a DXT1 block.

	// The alpha bitmap doesn't easily map lines to bytes, so we have to
	// interpret it correctly.  Extracted from
	// http://www.opengl.org/registry/specs/EXT/texture_compression_s3tc.txt :
	//
	//   The 6 "bits" bytes of the block are decoded into one 48-bit integer:
	//
	//     bits = bits_0 + 256 * (bits_1 + 256 * (bits_2 + 256 * (bits_3 +
	//                   256 * (bits_4 + 256 * bits_5))))
	//
	//   bits is a 48-bit unsigned integer, from which a three-bit control code
	//   is extracted for a texel at location (x,y) in the block using:
	//
	//       code(x,y) = bits[3*(4*y+x)+1..3*(4*y+x)+0]
	//
	//   where bit 47 is the most significant and bit 0 is the least
	//   significant bit.
	//QBitDump(block+2,6);

	// From Chromium (source was buggy)
	unsigned int line_0_1 = block[2] + 256 * (block[3] + 256 * block[4]);
	unsigned int line_2_3 = block[5] + 256 * (block[6] + 256 * block[7]);
	// swap lines 0 and 1 in line_0_1.
	unsigned int line_1_0 = ((line_0_1 & 0x000fff) << 12) |
		((line_0_1 & 0xfff000) >> 12);
	// swap lines 2 and 3 in line_2_3.
	unsigned int line_3_2 = ((line_2_3 & 0x000fff) << 12) |
		((line_2_3 & 0xfff000) >> 12);
	block[2] = line_3_2 & 0xff;
	block[3] = (line_3_2 & 0xff00) >> 8;
	block[4] = (line_3_2 & 0xff0000) >> 16;
	block[5] = line_1_0 & 0xff;
	block[6] = (line_1_0 & 0xff00) >> 8;
	block[7] = (line_1_0 & 0xff0000) >> 16;

	// And flip the DXT1 block using the above function.
	FlipDXT1BlockFull(block+8);
}

