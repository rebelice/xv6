#include <inc/types.h>

#include <kern/block.h>
#include <kern/spinlock.h>
#include <kern/sleeplock.h>
#include <kern/buf.h>
//There should be one superblock per disk device, but we run with only one device
struct superblock sb;

/* Read the super block.
 */
void readsb(int dev, struct superblock* sb)
{
}


/* Zero a block.
 */
static void
bzero(int dev, int bno)
{
}

/* Allocate a zeroed disk block.
 * todo:
 * 1. find a free block
 * 2. mark it in use
 * 3. bzero the block
 */
static uint32_t
balloc(uint32_t dev)
{
}

/* Free a disk block
 * todo:
 * mark it unused
 */
static void
bfree(int dev, uint32_t b)
{
}
