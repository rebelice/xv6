// Simple PIO-based (non-DMA) IDE driver code.

// TODO: include any files you need

#define SECTOR_SIZE 512

#define IDE_BSY 0x80
#define IDE_DRDY 0x40
#define IDE_DF 0x20
#define IDE_ERR 0x01

#define IDE_CMD_READ 0x20
#define IDE_CMD_WRITE 0x30
#define IDE_CMD_RDMUL 0xc4
#define IDE_CMD_WRMUL 0xc5

// You must hold idelock while manipulating queue.
static struct spinlock idelock;
static struct buf *idequeue;

static int havedisk1;

/* Busy Wait for IDE disk to become ready.
 * todo:
 * poll the status bits(status register) until the busy bit is clear and the ready bit is set.
 * check if IDE disk is error when checkkerr is set.
 * return value:
 * if checkerr is set: return -1 when IDE disk is error, return 0 otherwise.
 * else return 0 all the time.
 * Hint:
 * 1. you may use inb or insl to get data from I/O Port.
 * 2. Infomation about the status register can be found in ATA_PIO_Mode on https://osdev.org
 * 3. here are some MARCO you may need: IDE_* (IDE_CMD_* is not included)
 */
static int idewait(int checkerr)
{
}

/* Init IDE disk driver, and check if you have disk 1.
 * todo: 
 * wait for IDE disk ready
 * Switch to disk 1 (use Drive/Head Register)
 * Check if disk 1 is present (poll Status Register for 1000 times, see if it responds)
 * Set havedisk1 if disk 1 is present
 * Switch back to disk 0
 * Hint:
 * 1. you may use outb or outsl to send data to I/O Port.
 * 2. Infomation about Drive/Head Register and Status Register can be found in ATA_PIO_Mode on https://osdev.org
 */
void ideinit(void)
{
    //init the lock and pic first.
    initlock(&idelock, "ide");
    ioapicenable(IRQ_IDE, ncpu - 1);

    //todo: your code here.
}

/* Start the request for b. Caller must hold idelock.
 * todo:
 * start a read/write request for buf b on disk device correctly according to buf b
 * Hint:
 * 1. Information can be found in ATA_PIO_Mode and ATA_Command_Matrix on https://osdev.org
 * 2. here are some MARCO you may need: IDE_CMD_* SECTOR_SIZE and BSIZE(define in fs.h) 
 * 3. notice the difference between Block size and Sector size
 * 4. write the data here if you need, but read the data when you get the interrupt(not here).
 * 5. inb insl outb and outsl might be used
 */
static void idestart(struct buf *b)
{
    //todo: your code here.
}

/* Interrupt handler.
 * todo:
 * IMPORTANT: get the idelock at first!
 * read data here if you need
 * update the buf's flag
 * wake up the process waiting for this buf
 * start the right ide request
 * IMPORTANT: release the idelock!
 * Hint:
 * 1. you may use idequeue to manage all the ide request
 * 2. in challenge problem, you should start the right ide request according to your algorithm, while the next ide request in normal problem.
 */
void ideintr(void)
{
}

/* Sync buf b with disk.(in another word:ide request)
 * If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
 * Else if B_VALID is not set, read buf from disk, set B_VALID.
 * todo:
 * IMPORTANT: get the idelock at first!
 * append b to idequeue
 * start this request if idequeue is empty before
 * wait for ide request to finish sleeplock may be suitable.
 * IMPORTANT:Release the lock
 * Hint:
 * 1.busy wait or spin-lock may be not suitable,you may need implement sleeplock
 */
void iderw(struct buf *b)
{
}
