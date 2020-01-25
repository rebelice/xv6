// On-disk file system format.
// Both the kernel and user programs use this header file.

#ifndef KERN_FS_H
#define KERN_FS_H

#define ROOTINO 1  // root i-number

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint32_t))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint32_t size;            // Size of file (bytes)
  uint32_t addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  uint16_t inum;
  char name[DIRSIZ];
};

#define NINODE  50      // Maximum number of active i-nodes
#define ROOTDEV 1       // Device number of file system root disk

int             dirlink(struct inode*, char*, uint32_t);
struct inode*   dirlookup(struct inode*, char*, uint32_t*);
struct inode*   ialloc(uint32_t, short);
struct inode*   idup(struct inode*);
void            iinit(int dev);
void            ilock(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, char*, uint32_t, uint32_t);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, char*, uint32_t, uint32_t);

#endif /* !KERN_FS_H */