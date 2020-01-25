#include <inc/types.h>
#include <inc/mmu.h>

#include <kern/stat.h>
#include <kern/proc.h>
#include <kern/spinlock.h>
#include <kern/sleeplock.h>
#include <kern/fs.h>
#include <kern/buf.h>
#include <kern/file.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
static void itrunc(struct inode*);
// Inodes.
//
// An inode describes a single unnamed file.
// The inode disk structure holds metadata: the file's type,
// its size, the number of links referring to it, and the
// list of blocks holding the file's content.
//
// The inodes are laid out sequentially on disk at
// sb.startinode. Each inode has a number, indicating its
// position on the disk.
//
// The kernel keeps a cache of in-use inodes in memory
// to provide a place for synchronizing access
// to inodes used by multiple processes. The cached
// inodes include book-keeping information that is
// not stored on disk: ip->ref and ip->valid.
//
// An inode and its in-memory representation go through a
// sequence of states before they can be used by the
// rest of the file system code.
//
// * Allocation: an inode is allocated if its type (on disk)
//   is non-zero. ialloc() allocates, and iput() frees if
//   the reference and link counts have fallen to zero.
//
// * Referencing in cache: an entry in the inode cache
//   is free if ip->ref is zero. Otherwise ip->ref tracks
//   the number of in-memory pointers to the entry (open
//   files and current directories). iget() finds or
//   creates a cache entry and increments its ref; iput()
//   decrements ref.
//
// * Valid: the information (type, size, &c) in an inode
//   cache entry is only correct when ip->valid is 1.
//   ilock() reads the inode from
//   the disk and sets ip->valid, while iput() clears
//   ip->valid if ip->ref has fallen to zero.
//
// * Locked: file system code may only examine and modify
//   the information in an inode and its content if it
//   has first locked the inode.
//
// Thus a typical sequence is:
//   ip = iget(dev, inum)
//   ilock(ip)
//   ... examine and modify ip->xxx ...
//   iunlock(ip)
//   iput(ip)
//
// ilock() is separate from iget() so that system calls can
// get a long-term reference to an inode (as for an open file)
// and only lock it for short periods (e.g., in read()).
// The separation also helps avoid deadlock and races during
// pathname lookup. iget() increments ip->ref so that the inode
// stays cached and pointers to it remain valid.
//
// Many internal file system functions expect the caller to
// have locked the inodes involved; this lets callers create
// multi-step atomic operations.
//
// The icache.lock spin-lock protects the allocation of icache
// entries. Since ip->ref indicates whether an entry is free,
// and ip->dev and ip->inum indicate which i-node an entry
// holds, one must hold icache.lock while using any of those fields.
//
// An ip->lock sleep-lock protects all ip-> fields other than ref,
// dev, and inum.  One must hold ip->lock in order to
// read or write that inode's ip->valid, ip->size, ip->type, &c.

struct {
	struct spinlock lock;
	struct inode inode[NINODE];
} icache;

/* Init the icache
 * todo:
 * 1. init the lock
 * 2. (optional) init the superblock here
 */
void
iinit(int dev)
{
}

static struct inode* iget(uint32_t dev, uint32_t inum);

/* Allocate an inode on device dev.
 * Mark it as allocated by  giving it type type.
 * Returns an unlocked but allocated and referenced inode.
 */
struct inode*
ialloc(uint32_t dev, short type)
{
}

/* Copy a modified in-memory inode to disk.
 * Must be called after every change to an ip->xxx field
 * that lives on disk, since i-node cache is write-through.
 * Caller must hold ip->lock.
 */
void
iupdate(struct inode *ip)
{
}

/* Find the inode with number inum on device dev
 * and return the in-memory copy. Does not lock
 * the inode and does not read it from disk.
 * todo:
 * 1. return the inode if it is already cached
 * 2. reuse a empty slot.
 * hint:
 * 1. return an unlocked inode.
 */
static struct inode*
iget(uint32_t dev, uint32_t inum)
{
}

/* Increment reference count for ip.
 * Returns ip to enable ip = idup(ip1) idiom.
 */
struct inode*
idup(struct inode *ip)
{
}

/* Lock the given inode.
 * Reads the inode from disk if necessary.
 */
void
ilock(struct inode *ip)
{
}

// Unlock the given inode.
void
iunlock(struct inode *ip)
{
}

/* Drop a reference to an in-memory inode.
 * If that was the last reference, the inode cache entry can
 * be recycled.
 * If that was the last reference and the inode has no links
 * to it, free the inode (and its content) on disk.
 * All calls to iput() must be inside a transaction in
 * case it has to free the inode.
 */
void
iput(struct inode *ip)
{
}

// Common idiom: unlock, then put.
void
iunlockput(struct inode *ip)
{
	iunlock(ip);
	iput(ip);
}

// Inode content
//
// The content (data) associated with each inode is stored
// in blocks on the disk. The first NDIRECT block numbers
// are listed in ip->addrs[].  The next NINDIRECT blocks are
// listed in block ip->addrs[NDIRECT].


/* Return the disk block address of the bnth block in inode ip.
 * If there is no such block, bmap allocates one.
 */
static uint32_t
bmap(struct inode *ip, uint32_t bn)
{
}

/* Truncate inode (discard contents).
 * Only called when the inode has no links
 * to it (no directory entries referring to it)
 * and has no in-memory reference to it (is
 * not an open file or current directory).
 * todo:
 * 1. free every direct content block
 * 2. free every indirect content block via indirect block
 * 3. free the indirect block itself
 */
static void
itrunc(struct inode *ip)
{
}

/* Copy stat information from inode.
 * Caller must hold ip->lock.
 */
void
stati(struct inode *ip, struct stat *st)
{
}

/* Read data from inode.
 * Caller must hold ip->lock.
 */
int
readi(struct inode *ip, char *dst, uint32_t off, uint32_t n)
{
}

/* Write data to inode.
 * Caller must hold ip->lock.
 */
int
writei(struct inode *ip, char *src, uint32_t off, uint32_t n)
{
}

// Directories

int
namecmp(const char *s, const char *t)
{
	return strncmp(s, t, DIRSIZ);
}

/* Look for a directory entry in a directory.
 * If found, set *poff to byte offset of entry.
 */
struct inode*
dirlookup(struct inode *dp, char *name, uint32_t *poff)
{
}

/* Write a new directory entry (name, inum) into the directory dp.
 */
int
dirlink(struct inode *dp, char *name, uint32_t inum)
{
}

// Paths

/* Copy the next path element from path into name.
 * Return a pointer to the element following the copied one.
 * The returned path has no leading slashes,
 * so the caller can check *path=='\0' to see if the name is the last one.
 * If no name to remove, return 0.
 *
 * Examples:
 *   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
 *   skipelem("///a//bb", name) = "bb", setting name = "a"
 *   skipelem("a", name) = "", setting name = "a"
 *   skipelem("", name) = skipelem("////", name) = 0
 */
static char*
skipelem(char *path, char *name)
{
	char *s;
	int len;

	while(*path == '/')
		path++;
	if(*path == 0)
		return 0;
	s = path;
	while(*path != '/' && *path != 0)
		path++;
	len = path - s;
	if(len >= DIRSIZ)
		memmove(name, s, DIRSIZ);
	else {
		memmove(name, s, len);
		name[len] = 0;
	}
	while(*path == '/')
		path++;
	return path;
}

/* Look up and return the inode for a path name.
 * If parent != 0, return the inode for the parent and copy the final
 * path element into name, which must have room for DIRSIZ bytes.
 * Must be called inside a transaction since it calls iput().
 */
static struct inode*
namex(char *path, int nameiparent, char *name)
{
}

struct inode*
namei(char *path)
{
	char name[DIRSIZ];
	return namex(path, 0, name);
}

struct inode*
nameiparent(char *path, char *name)
{
	return namex(path, 1, name);
}
