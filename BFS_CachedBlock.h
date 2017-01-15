/* CachedBlock - interface for the block cache
 *
 * Copyright 2001-2004, Axel Dörfler, axeld@pinc-software.de.
 * This file may be used under the terms of the MIT License.
 */
#ifndef BFS_CACHED_BLOCK_H
#define BFS_CACHED_BLOCK_H


#include "BOS_System_Dependencies.h"

#include "BFS_Volume.h"
#include "BFS_Journal.h"
#include "BFS_Lock.h"
#include "BFS_Chain.h"
#include "BFS_Debug.h"


// The CachedBlock class is completely implemented as inlines.
// It should be used when cache single blocks to make sure they
// will be properly released after use (and it's also very
// convenient to use them).

class CachedBlock {
	public:
		CachedBlock(Volume *volume);
		CachedBlock(Volume *volume, uint64 block);
		CachedBlock(Volume *volume, block_run run);
		CachedBlock(CachedBlock *cached);
		~CachedBlock();

		inline void Keep();
		inline void Unset();

		inline const uint8 *SetTo(uint64 block, uint64 base, size_t length);
		inline const uint8 *SetTo(uint64 block);
		inline const uint8 *SetTo(block_run run);
		inline uint8 *SetToWritable(Transaction &transaction, uint64 block, uint64 base, size_t length, bool empty = false);
		inline uint8 *SetToWritable(Transaction &transaction, uint64 block, bool empty = false);
		inline uint8 *SetToWritable(Transaction &transaction, block_run run, bool empty = false);
		inline status_t MakeWritable(Transaction &transaction);

		const uint8 *Block() const { return fBlock; }
		uint64 BlockNumber() const { return fBlockNumber; }
		uint32 BlockSize() const { return fVolume->BlockSize(); }
		uint32 BlockShift() const { return fVolume->BlockShift(); }

	private:
		CachedBlock(const CachedBlock &);
		CachedBlock &operator=(const CachedBlock &);
			// no implementation

	protected:
		Volume	*fVolume;
		uint64	fBlockNumber;
		uint8	*fBlock;
};


//--------------------------------------
// inlines


inline
CachedBlock::CachedBlock(Volume *volume)
	:
	fVolume(volume),
	fBlockNumber(0),
	fBlock(NULL)
{
}


inline
CachedBlock::CachedBlock(Volume *volume, uint64 block)
	:
	fVolume(volume),
	fBlockNumber(0),
	fBlock(NULL)
{
	SetTo(block);
}


inline
CachedBlock::CachedBlock(Volume *volume, block_run run)
	:
	fVolume(volume),
	fBlockNumber(0),
	fBlock(NULL)
{
	SetTo(volume->ToBlock(run));
}


inline 
CachedBlock::CachedBlock(CachedBlock *cached)
	:
	fVolume(cached->fVolume),
	fBlockNumber(cached->BlockNumber()),
	fBlock(cached->fBlock)
{
	cached->Keep();
}


inline
CachedBlock::~CachedBlock()
{
	Unset();
}


inline void
CachedBlock::Keep()
{
	fBlock = NULL;
}


inline void
CachedBlock::Unset()
{
	if (fBlock != NULL)
		block_cache_put(fVolume->BlockCache(), fBlockNumber);
}


inline const uint8 *
CachedBlock::SetTo(uint64 block, uint64 base, size_t length)
{
	Unset();
	fBlockNumber = block;
	return fBlock = (uint8 *)block_cache_get_etc(fVolume->BlockCache(), block, base, length/*, offset*/);
}


inline const uint8 *
CachedBlock::SetTo(uint64 block)
{
	return SetTo(block, block, 1);
}


inline const uint8 *
CachedBlock::SetTo(block_run run)
{
	return SetTo(fVolume->ToBlock(run));
}


inline uint8 *
CachedBlock::SetToWritable(Transaction &transaction, uint64 block, uint64 base, size_t length, bool empty)
{
	Unset();
	fBlockNumber = block;
	
	if (empty)
		return fBlock = (uint8 *)block_cache_get_empty(fVolume->BlockCache(), block, transaction.ID());

	return fBlock = (uint8 *)block_cache_get_writable_etc(fVolume->BlockCache(),
		block, base, length, transaction.ID());
}


inline uint8 *
CachedBlock::SetToWritable(Transaction &transaction, uint64 block, bool empty)
{
	return SetToWritable(transaction, block, block, 1, empty);
}


inline uint8 *
CachedBlock::SetToWritable(Transaction &transaction, block_run run, bool empty)
{
	return SetToWritable(transaction, fVolume->ToBlock(run), empty);
}


inline status_t
CachedBlock::MakeWritable(Transaction &transaction)
{
	if (fBlock == NULL)
		return B_NO_INIT;

	return block_cache_make_writable(fVolume->BlockCache(), fBlockNumber, transaction.ID());
}


#endif	/* CACHED_BLOCK_H */