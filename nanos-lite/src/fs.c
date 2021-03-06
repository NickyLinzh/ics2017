#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf,size_t len);
enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  file_table[FD_FB].size = _screen.width * _screen.height * 4;//_screen.width is from ioe.c
  // TODO: initialize the size of /dev/fb

}

int fs_open(const char *pathname, int flags, int mode) {
  for(int NR = 0; NR < NR_FILES; NR++) {
    if(!strcmp(file_table[NR].name, pathname))
      return NR;
    }
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  switch(fd) {
   	 case FD_STDIN:
    	 case FD_STDOUT:
   	 case FD_STDERR:
		len = 0;
		break;
   	 case FD_EVENTS:
		len = events_read((void *)buf,len);
		break;
   	 case FD_DISPINFO:
		//TODO();
		if(file_table[fd].open_offset >= file_table[fd].size)
		  return 0;
		if(file_table[fd].open_offset + len > file_table[fd].size)
		  len = file_table[fd].size - file_table[fd].open_offset;
		dispinfo_read(buf,file_table[fd].open_offset,len);
		file_table[fd].open_offset += len;
		break;
    	 default:
		if(file_table[fd].open_offset >= file_table[fd].size)
	          return 0;
		if(file_table[fd].open_offset + len > file_table[fd].size)
		  len = file_table[fd].size - file_table[fd].open_offset;
		  //reset the len
	        ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
		file_table[fd].open_offset += len;
  }
  return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  switch(fd) {
    	 case FD_STDOUT:
    	 case FD_STDERR:
	     for(int count = 0; count < len; count++) {
		     _putc(((char*)buf)[count]);
	     }
	     break;
    	 case FD_FB:
	 //    TODO();
             fb_write(buf,file_table[fd].open_offset,len);
	     file_table[fd].open_offset += len;
	     break;
    	 //case FD_EVENTS:
    	 case FD_DISPINFO:
	   //  TODO();
	     break;
    	 default:
	     if(file_table[fd].open_offset >= file_table[fd].size)
	       return 0;
	     if(file_table[fd].open_offset + len > file_table[fd].size)			 
	       len = file_table[fd].size - file_table[fd].open_offset;
			//reset the len
             ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
	     file_table[fd].open_offset += len;
	}
	return len;
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}

int fs_close(int fd) {
	return 0;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  int ret = -1;
  switch(fd) {
	  case FD_STDIN:
	  case FD_STDOUT:
	  case FD_STDERR:
		  return 0;
	  default:
	      switch(whence) {
		 case SEEK_SET:
			if(offset <= file_table[fd].size && offset >= 0) {					              file_table[fd].open_offset = offset;
		            ret = file_table[fd].open_offset;
			}
			break;
		 case SEEK_CUR:
			if((offset + file_table[fd].open_offset <= file_table[fd].size) && offset + file_table[fd].open_offset >= 0) {
		          file_table[fd].open_offset = offset + file_table[fd].open_offset;
			  ret = file_table[fd].open_offset;
			}
			 break;
		 case SEEK_END:
			if(1) {
			  file_table[fd].open_offset = offset + file_table[fd].size;
	          	  ret = file_table[fd].open_offset;
			}
			break;
			}

	}
	return  (off_t)ret;
}
