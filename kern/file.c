//
// File descriptors
//

#include <inc/types.h>

#include <kern/fs.h>
#include <kern/spinlock.h>
#include <kern/sleeplock.h>
#include <kern/file.h>

struct devsw devsw[NDEV];
struct {
	struct spinlock lock;
	struct file file[NFILE];
} ftable;

void
fileinit(void)
{
	initlock(&ftable.lock, "ftable");
}

// Allocate a file structure.
struct file*
filealloc(void)
{
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
}

// Read from file f.
int
fileread(struct file *f, char *addr, int n)
{
}

// Write to file f.
int
filewrite(struct file *f, char *addr, int n)
{
}

