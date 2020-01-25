# 7 File System 

[toc]

## 7.1 Introduction

In lab 7, you will write a Unix-like file system with a logging mechanism.

In part A, you will write the lowest three layer and a part of inode layer. You will finish the highest four in part B.

There are one challenge for you to  choose in part A if you like: 

1. write a "copy on write" mechanism instead of a logging mechanism.

### 7.1.1 Getting started

> git merge lab7a
>
> git merge lab7b

## 7.1.2 Lab requirements

In this part, implement a file system and answer the follow questions in a few words: Place the answers in a file called answers-lab7.txt in the top level of your lab directory before handing in your work.

## 7.2 Overview

### 7.2.1 File System Layout

There is a ﬁle system implementation organized in seven layers:![](D:\data\ScreenShots\2019-12\ApplicationFrameHost_7FWO4a4oA7.png)

**The disk layer** reads and writes blocks on an IDE hard drive, which is almost finished in lab 6.

**The buﬀer cache layer** caches disk blocks and synchronizes access to them, making sure that only one kernel process at a time can modify the data stored in any particular block. 

**The logging layer** allows higher layers to wrap updates to several blocks in a transaction, and ensures that the blocks are updated atomically in the face of crashes (i.e., all of them are updated or none). 

**The inode layer** provides individual ﬁles, each represented as an inode with a unique i-number and some blocks holding the ﬁle’s data. 

**The directory layer** implements each directory as a special kind of inode whose content is a sequence of directory entries, each of which contains a ﬁle’s name and i-number. 

**The pathname layer** provides hierarchical path names like /usr/local/bin/python, and resolves them with recursive lookup. 

**The ﬁle descriptor layer** abstracts many Unix resources (e.g., pipes, devices, ﬁles, etc.) using the ﬁle system interface, simplifying the lives of application programmers. 

In part A, we will finish the lowest four(the lowest part is almost finished).

### 7.2.2 Disk Layout

The ﬁle system must have a plan for where it stores inodes and content blocks on the disk. To do so, we divides the disk into several sections (which defines in fs.h): 

The ﬁle system does not use block 0 (it holds the boot sector).

Block 1 is called the superblock (The superblock is ﬁlled in by a separate program, called **mkfs**, which builds an initial ﬁle system).The superblock contains metadata about the ﬁle system (the ﬁle system size in blocks, the number of data blocks, the number of inodes, and the number of blocks in the log). Blocks starting at 2 hold the log. After the log are the inodes, with multiple inodes per block. After those come bitmap blocks tracking which data blocks are in use. The remaining blocks are data blocks; each is either marked free in the bitmap block, or holds content for a ﬁle or directory.

![](D:\data\ScreenShots\2019-12\ApplicationFrameHost_OPbXxy2WmT.png)

**Notice that:** we used to place our kernel's elf started from blocks 1, but now we should change either the  arrangement of file system, or the kernel's elf file placement. We strongly recommend that you use the second solution, which is based on a very simple fact that the kernel should also be managed by the file system. As for whether you want to modify the boot loader further to read the kernel's elf, or directly specify the kernel's elf file placement, it does not matter.

## 7.3 Implement File System Layer by Layer

### 7.3.1 Buffer Cache Layer

#### 7.3.1.1 Overview

The buﬀer cache has two jobs: 

1. synchronize access to disk blocks to ensure that only one copy of a block is in memory and that only one kernel thread at a time uses that copy.
2. cache popular blocks so that they don’t need to be re-read from the slow disk.

The code you should write is in bio.c. 

#### 7.3.1.2 Binit

The function **Binit**, called by main, initializes the data structure with the NBUF buﬀers. Make sure that all other access to the buﬀer cache should refer to this data structure. A buﬀer should have two state bits associated with it. B_VALID indicates that the buﬀer contains a copy of the block. B_DIRTY indicates that the buﬀer content has been modiﬁed and needs to be written to the disk.

#### 7.3.1.3 Bread and Bget 

**Bread** will return a buffer for given sector with the data ready. Bread  calls **Bget** to get a buﬀer for the given sector. If the buﬀer needs to be read from disk, **Bread** calls **iderw** to do that before returning the buﬀer. 

**Bget** should return the buffer directly if there is such a buffer. If not, we recommend that you reuse a buffer in following order:

1. A buffer with refcnt=0.
2. A buffer that not locked and not dirty. (Remember to clear B_VALID flag.)
3. Waiting for a buffer released by other Bxx.(Or simply panic instead.)

> Q: xv6 simply panic here，but why?

It is **important** to make sure that  there is at most one cached buﬀer per disk sector. To let the data structure of the buffer cache contain a lock may help you to reach this goal.

It is also **important** to make only one kernel thread at a time use that copy. Giving every buffer a lock may help you to reach this goal.

#### 7.3.1.4 Bwrite and Brelease

Once Bread has read the disk (if needed) and returned the buffer to its caller, the caller has exclusive use of the buffer and can read or write the data bytes. If the caller does modify the buffer, it must call **Bwrite** to write the changed data to disk before calling **Brelease** to release the buffer. Bwrite calls iderw to talk to the disk hardware. 

**Brelease**  should release the buffer, and may help the data structure of the buffer cache implement **LRU** mechanism. (Other mechanism is also allowed.)

### 7.3.2 Logging Layer

#### 7.3.2.1 OverView

 If the system crashs and reboots, the file system code will recover from the crash as follows, before running any processes. If the log is marked as containing a complete operation, then the recovery code copies the writes to where they belong in the on-disk file system. If the log is not marked as containing a complete operation, the recovery code ignores the log. The recovery code finishes by erasing the log. 

The log resides at a several blocks started with block 2. It consists of a header block followed by a sequence of updated block copies (‘‘logged blocks’’). The header block contains an array of sector numbers, one for each of the logged blocks. The header block also contains the count of logged blocks. (Xv6 writes the header block when a transaction commits, but not before, and sets the count to zero after copying the logged blocks to the file system. Thus a crash midway through a transaction will result in a count of zero in the log’s header block; a crash after a commit will result in a non-zero count.) 

#### 7.3.2.2 Group Commit

 Group commit is a idea to allow concurrent execution of file system operations by different processes. It requires the logging system to accumulate the writes of multiple system calls into one transaction. Only when there are no system call still in this transaction, this transaction will be committed. Because of a fixed amount of space on disk to hold the log, it will not suffer from starvation.

#### 7.3.2.3 Implement

   A typical use of the log in a system call looks like this: 

![](D:\data\ScreenShots\2019-12\chrome_IoRqtTuvyU.png)

**begin_op** waits until the logging system is not currently committing and have enough free log space. The number of system calls in this transaction and the amount of left space should be counted. 

 **log_write** acts as a proxy for bwrite.   It records the block’s sector number in memory, reserving it a slot in the log on disk, and marks the buffer B_DIRTY to prevent the block cache from evicting it. The block must stay in the cache until committed: until then, the cached copy is the only record of the modification; it cannot be written to its place on disk until after commit; and other reads in the same transaction must see the modifications.  log_write notices when a block is written multiple times during a single transaction, and allocates that block the same slot in the log. This optimization is often called absorption. It is common that, for example, the disk block containing inodes of several files is written several times within a transaction. By absorbing several disk writes into one, the file system can save log space and can achieve better performance because only one copy of the disk block must be written to disk. 

 **end_op** first decrease the count of system calls.If it is now zero,it commits this transaction by calling **commit**. There are four stages for **commit** :

1. write_log, which write the log.
2. write_head, which write the head block to disk.
3. install_trans, which writes each blocks into the proper place.
4. set the count in head block zero.

#### 7.3.2.4 Notification

**It is REALLY IMPORTANT that any code dealing with disk after here must be called inside a transaction.(Even it is a read-only syscall)**  

### 7.3.3 Inode Layer(Metadata)

#### 7.3.3.0 Block allocator

 The block allocator provides two functions: **balloc** allocates a new disk block, and **bfree** frees a block.

**Balloc** allocates a new disk block according to the bitmap. (The race that might occur if two processes try to allocate a block at the same time is prevented by the fact that the buffer cache only lets one process use any one bitmap block at a time). **Bfree** clear the right bit in bitmap.

#### 7.3.3.1 Overview

The term inode can have one of two related meanings. It might refer to the ondisk data structure containing a file’s size and list of data block numbers. Or ‘‘inode’’ might refer to an in-memory inode, which contains a copy of the on-disk inode as well as **extra** information needed within the kernel.

The on-disk inode is defined by a struct **dinode**. struct **inode** is the in-memory copy of a struct dinode on disk. The **nlink** field counts the number of directory entries that refer to this inode. The **ref** field of inode counts the number of C pointers referring to the in-memory inode. The **iget** and **iput** functions acquire and release pointers to an inode, modifying the reference count.The **ialloc** is similar to balloc which is used to allocate a new inode. Code must lock the inode using **ilock** before reading or writing its metadata or content. **Ilock** uses a sleep-lock for this purpose (just like sleep-lock in each block cache).

#### 7.3.3.2 ialloc, iget

**Ialloc** is used to allocate a new inode (for example, create a file). Ialloc is similar to **balloc**: it loops over the inode structures on the disk, one block at a time, looking for one that is marked free. When it finds one, it claims it by writing the new type to the disk and then returns an entry from the inode cache with the tail call to **iget**. **Iget**  looks through the inode cache for an active entry (ip->ref > 0) with the desired device and inode number. If it finds one, it returns a new reference to that inode, otherwise it reuse an inode cache, just like **bget**. 

Notice that ialloc and iget **do not** get the lock of returned cache (just get a reference). Instead of that, any time you want to read or write the metadata or content.

> Q: Why should iget return an unlocked inode instead of a locked inode? And why bget return a locked cache instead of an unlocked cache? 

#### 7.3.3.3 ilock, iunlock

Code must lock the inode using **ilock** before reading or writing its metadata or content. **Ilock** uses a sleep-lock for this purpose. Once ilock has exclusive access to the inode, it reads the inode from disk (more likely, the buffer cache) if needed. The function **iunlock**  releases the sleep-lock, which may cause any processes sleeping to be woken up.

#### 7.3.3.4 iput, itrunc

**Iput** releases a C pointer to an inode by decrementing the reference count. If this is the last reference, the inode’s slot in the inode cache is now free and can be re-used for a different inode.

If **iput** sees that there are no C pointer references to an inode and that the inode has no links to it (occurs in no directory), then the inode and its data blocks must be freed. **Iput** calls **itrunc** to truncate the file to zero bytes, freeing the data blocks; sets the inode type to 0 (unallocated); and writes the inode to disk.

The implementation of **itrunc** will be discussed in the next section.

#### 7.3.3.5 Notice for iput

**iput** can write to the disk. This means that any system call that uses the file system may write the disk.

Therefore, even read-only system calls **must** be wrapped in transactions if they use the file system.

#### 7.3.3.6 crash before iput

There is a challenging interaction between **iput** and crashes. **iput** doesn’t truncate a file immediately when the link count for the file drops to zero, because some process might still hold a reference to the inode in memory: a process might still be reading and writing to the file, because it successfully opened it. But, if a crash happens before the last process closes the file descriptor for the file, then the file will be marked allocated on disk but no directory entry points to it.

File systems handle this case in one of two ways. The simple solution is that on recovery, after reboot, the file system scans the whole file system for files that are marked allocated, but have no directory entry pointing to them. If any such file exists, then it can free those files. 

The second solution doesn’t require scanning the file system. In this solution, the file system records on disk (e.g., in the super block) the inode inumber of a file whose link count drops to zero but whose reference count isn’t zero. If the file system removes the file when its reference counts reaches 0, then it updates the on-disk list by removing that inode from the list. On recovery, the file system frees any file in the list.

Choose one of them to implement it.(I prefer the first one which is simple enough to implement.However, Xv6 do neither of them, but simply ignore it.)

### 7.3.4 Inode Layer(Content)

#### 7.3.4.1 Overview

In the previous section, we talked about how to manage Inode. In this section we will discuss how to use Inode to manage content(One Inode corresponds to one file which contain a lot of block in disk).

![](D:\data\ScreenShots\2020-01\chrome_Vv8YZViU5S.png)

The inode data is found in the blocks listed in the dinode’s addrs array. The first NDIRECT blocks of data are listed in the first NDIRECT entries in the array; these blocks are called **direct blocks**. The next NINDIRECT blocks of data are listed not in the inode but in a data block called the **indirect block**. The last entry in the addrs array gives the address of the indirect block.

The function **bmap** manages the representation so that higher-level routines such as **readi** and **writei**.

#### 7.3.4.2 bmap

**Bmap** returns the disk block number of the **bn**’th data block for the inode **ip**. If ip does not have such a block yet, bmap allocates one. An ip->addrs[] or indirect entry of zero indicates that no block is allocated. As bmap encounters zeros, it replaces them with the numbers of fresh blocks, allocated on demand.

**Bmap** makes it easy for **readi** and **writei** to get at an inode’s data.

#### 7.3.4.3 itrunc

**Itrunc** is used to free a file’s blocks, resetting the inode’s size to zero. 

**Itrunc** starts by freeing the direct blocks, then the ones listed in the indirect block, and finally the indirect block itself.

#### 7.3.4.4 readi, writei

**readi** read each block of the file, copying data from the buffer into destination.

writei copy data into the buffer, and update its size if needed.

#### 7.3.4.5 stati 

The function **stati** copies inode metadata into the stat structure, which is exposed to user programs via the **stat** system call.

### 7.3.5 directory layer

A directory is implemented internally much like a file. Its inode has type T_DIR and its data is a sequence of directory entries. Each entry is a struct **dirent**, which contains a name and an inode number. 

The function **dirlookup** searches a directory for an entry with the given name. If it finds one, it returns a pointer to the corresponding inode, unlocked, and sets *poff to the byte offset of the entry within the directory, in case the caller wishes to edit it. If dirlookup finds an entry with the right name, it updates *poff, releases the block, and returns an unlocked inode obtained via iget. 

The function **dirlink**  writes a new directory entry with the given name and inode number into the directory dp. If the name already exists, dirlink should return an error. 

### 7.3.6 path name layer

Path name lookup involves a succession of calls to **dirlookup**, one for each path component.

The function **namei** evaluates path and returns the corresponding inode.

The function **nameiparent** is a variant of **namei**: it stops before the last element, returning the inode of the parent directory and copying the final element into name. 

Both **Namei** and **nameiparent** call the generalized function **namex** to do the real work.

**Namex** starts by deciding where the path evaluation begins. If the path begins with a slash, evaluation begins at the root; otherwise, the current directory. Then it uses **skipelem** to consider each element of the path in turn.

### 7.3.7 file descriptor layer

#### 7.3.7.1 Overview

A cool aspect of the Unix interface is that most resources in Unix are represented as files, including devices such as the console, pipes, and of course, real files. The file descriptor layer is the layer that achieves this uniformity.

each process has its own table of open files, or file descriptors. Each open file is represented by a struct **file**, which is a wrapper around either an **inode** or a **pipe**, plus an i/o **offset**.Each call to **open** creates a new open file (a new struct **file**): if multiple processes open the same file independently, the different instances will have different i/o offsets. A reference count tracks the number of references to a particular open file. A file can be open for reading or writing or both. The readable and writable fields track this.

All the open files in the system are kept in a global file table, the **ftable**. The file table has a function to allocate a file (**filealloc**), create a duplicate reference (**filedup**), release a reference (**fileclose**), and read and write data (**fileread** and **filewrite**).

**Filealloc** scans the file table for an unreferenced file (f->ref == 0) and returns a new reference; **filedup** increments the reference count; and **fileclose** decrements it. When a file’s reference count reaches zero, **fileclose** releases the underlying pipe or inode, according to the type. 

The functions **filestat**, **fileread**, and **filewrite** implement the **stat**, **read**, and **write** operations on files.(**filestat** is only allowed on inodes and calls **stati**.)

### 7.3.8 System call

you can implement some system call of file system if you want.

We recommend 3+2+1 system call: **open**, **read**, **write** and **link**, **unlink** and **stat**.

## 7.4 Summary and Reference

This experiment is modified from xv6. Since there are too many codes to be implemented in the file system part, a reliable choice is to read the xv6 original code and then complete the lab by referring to its implementation. Furthermore, there are more implementation details explained in the xv6 book, it is recommended to read it first or when you encounter unsolvable problems.