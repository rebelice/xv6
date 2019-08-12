#ifndef KERN_KALLOC_H
#define KERN_KALLOC_H

void kinit1(void *, void *);
void kinit2(void *, void *);
char *kalloc(void);
void kfree(char*);

#endif /* !KERN_KALLOC_H */