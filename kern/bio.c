// Buffer cache.
//
// The buffer cach hold cached copies of disk block contents.  
// Caching disk blocks in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.
//
// The implementation uses two state flags internally:
// * B_VALID: the buffer data has been read from the disk.
// * B_DIRTY: the buffer data has been modified
//     and needs to be written to disk.

#include <inc/types.h>

#include <kern/block.h>
#include <kern/spinlock.h>
#include <kern/sleeplock.h>
#include <kern/buf.h>

/* Init the block cache
 * todo:
 * init the cache and its lock.
 * init each buf and its lock
 * let main call this function
 */
void
binit(void)
{
}
// Hint: Remember to get the lock, if you need change the block cache. 


/* Return a locked buffer for given sector.
 * todo:(Recommended)
 * return it in following order:
 * 1. there is such a buffer
 * 2. there is a free buffer, reuse it
 * 3. there is an unlocked and undirty buffer, reuse it
 * 4. wait for a buffer to become ready(or simply panic here)
 */
static struct buf*
bget(uint32_t dev, uint32_t blockno)
{

}

/* Return a locked buffer with the contents of the indicated block.
 * todo:
 * 1. get a locked buffer by calling bget.
 * 2. call iderw if necessary.
 */
struct buf*
bread(uint32_t dev, uint32_t blockno)
{
}

/* Write b's contents to disk.
 * todo:
 * 1. write the buffer to disk.
 */
void
bwrite(struct buf *b)
{
}

/* Release a locked buffer.
 * todo:
 * 1. release the buffer's lock.
 */
void
brelse(struct buf *b)
{
}
