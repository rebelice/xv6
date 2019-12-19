#include <inc/types.h>

#include <kern/log.h>
#include <kern/block.h>
#include <kern/spinlock.h>
#include <kern/sleeplock.h>
#include <kern/buf.h>

// Simple logging that allows concurrent FS system calls.
//
// A log transaction contains the updates of multiple FS system
// calls. The logging system only commits when there are
// no FS system calls active. Thus there is never
// any reasoning required about whether a commit might
// write an uncommitted system call's updates to disk.
//
// A system call should call begin_op()/end_op() to mark
// its start and end. Usually begin_op() just increments
// the count of in-progress FS system calls and returns.
// But if it thinks the log is close to running out, it
// sleeps until the last outstanding end_op() commits.
//
// The log is a physical re-do log containing disk blocks.
// The on-disk log format:
//   header block, containing block #s for block A, B, C, ...
//   block A
//   block B
//   block C
//   ...
// Log appends are synchronous.

// Contents of the header block, used for both the on-disk header block
// and to keep track in memory of logged block# before commit.
struct logheader {
  int n;
  int block[LOGSIZE];
};

struct log {
  struct spinlock lock;
  int start;
  int size;
  int outstanding; // how many FS sys calls are executing.
  int committing;  // in commit(), please wait.
  int dev;
  struct logheader lh;
};
struct log log;

static void recover_from_log(void);
static void commit();

/* Init struct log.
 * todo:
 * 1. init struct log according to superblock
 * 2. recover from log after log's initialization
 */
void
initlog(int dev)
{
}

/* Copy committed blocks from log to their destination
 * Be careful about the order of block release
 */
static void
install_trans(void)
{
}

/* Read the log header from disk into the in-memory log header
 */
static void
read_head(void)
{
}

/* Write in-memory log header to disk.
 */
static void
write_head(void)
{
  struct buf *buf = bread(log.dev, log.start);
  struct logheader *hb = (struct logheader *) (buf->data);
  int i;
  hb->n = log.lh.n;
  for (i = 0; i < log.lh.n; i++) {
    hb->block[i] = log.lh.block[i];
  }
  bwrite(buf);
  brelse(buf);
}

/* Recover from log which must be consistent with commit
 * todo:
 * 1. read the log header
 * 2. copy committed blocks from log
 * 3. set the count in head to zero
 * 4. write the log header
 */
static void
recover_from_log(void)
{
}

/* called at the start of each FS system call.
 * means that it join current transaction
 * todo:
 * 1. wait if it is committing now or not enough space for this op
 * 2. increase the count simply
 */
void
begin_op(void)
{
}

/* called at the end of each FS system call.
 * means that it leave current transaction but this op may not be commited immediately
 * todo:
 * 1. decrease the count
 * 2. commits if this was the last outstanding operation.
 * Hint:
 * 1. you should call commit without holding locks, since not allowed to sleep with locks
 * 2. however changing log.committing should hold locks.
 */
void
end_op(void)
{
}

/* Copy modified blocks from cache to log.
 */
static void
write_log(void)
{
}

/* Commit the transaction in four steps
 * todo:
 * 1. write modified blocks from cache to log
 * 2. write log header to disk -- the real commit
 * 3. write modified blocks from log to cache
 * 4. set the count in header to zero -- Erase the transaction from the log
 */
static void
commit()
{
}

/* log_write acts as a proxy for bwrite.
 * we assume that caller has modified b->data and is done with the buffer
 * we only record the block number and pin in the cache with B_DIRTY
 * commit()/write_log() will do the disk write.
 *
 * a typical use is:
 *   bp = bread(...)
 *   modify bp->data[]
 *   log_write(bp)
 *   brelse(bp)
 * 
 * todo:
 * 1. find if the buffer is already in the list of the blockno of modified buffer.
 * 2. if not append the blockno to the list
 * 3. set flag with B_DIRTY to prevent eviction
 */
void
log_write(struct buf *b)
{
}

