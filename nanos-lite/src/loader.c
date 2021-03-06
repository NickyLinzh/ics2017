#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)
size_t get_ramdisk_size();
//void ramdisk_read(void *buf,off_t offset, size_t len);
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_filesz(int fd);
extern void * new_page();
uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();

  //size_t size=get_ramdisk_size();
  //ramdisk_read(DEFAULT_ENTRY,0,size);
  //filename = "/bin/text";
  //int fd = fs_open(filename, 0, 0);
  //fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));
  int fd = fs_open(filename, 0, 0);
  int filesize = fs_filesz(fd);
  void *pa;
  void *va = DEFAULT_ENTRY;
  while(filesize > 0) {
    pa = new_page();//create a new physical page
    _map(as, va, pa);//map virtual to physical
    va += PGSIZE;//add va
    fs_read(fd, pa, PGSIZE);//read file content, and fill in this physical page
    filesize -= PGSIZE;
  }
  
  return (uintptr_t)DEFAULT_ENTRY;
}
