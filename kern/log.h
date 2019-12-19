#ifndef KERN_LOG_H
#define KERN_LOG_H

void            initlog(int dev);
void            log_write(struct buf*);
void            begin_op();
void            end_op();

#endif /* !KERN_LOG_H */