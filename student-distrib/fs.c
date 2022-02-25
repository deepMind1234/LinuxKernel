#include "fs.h"

boot_block_t* boot_block = NULL;

/* fs_init
 * description: verify the loaded filesystem and save to global constructs
 * input: fs_start -- pointer to start boot_block
 *        fs_end -- pointer to end of filesystem
 * output: 0/-1 -- search FS_FAIL indicator
 * side-effect: bootblock saved to structures
 */
int32_t fs_init(uint32_t fs_start, uint32_t fs_end) {
    boot_block_t * start = (boot_block_t*) fs_start;
    if (start->dir_count > DENTRY_MAX)
        return FS_FAIL;
    if ((boot_block_t *)fs_end != start + INODE_OFFSET + start->inode_count + start->data_count)
        return FS_FAIL;
    boot_block = (boot_block_t*) fs_start;
    return FS_PASS;
}

/* file_open
 * open a file for reading, and set the inode.
 * inputs - filename -- 32 character buffer with file name
 *          inode -- pointer to inode value to update.
 * outputs - FS_FAIL/FS_PASS
 * side effects - updates inode input with inode of file.
 */
int32_t file_open(char* filename, int32_t* inode) {
    if(!boot_block) return FS_FAIL;
    if(!filename || !inode) return FS_FAIL;

    dentry_t f;
    if (read_dentry_by_name ((char*)filename, &f) == FS_FAIL)
        return FS_FAIL;
    *inode = f.inode_num;
    return FS_PASS;
}

/* file_close
 * close an already open file
 * inputs - filename -- 32 character buffer with file name
 *          inode -- pointer to inode value to update.
 * outputs - FS_FAIL/FS_PASS
 * side effects - clears the inode value
 */
int32_t file_close(int32_t* inode) {
    if(!boot_block) return FS_FAIL;
    if(!inode) return FS_FAIL;

    *inode = NULL;
    return FS_PASS;
}

/* file_write
 * write to file. Not implemented yet, so goes straight to fail.
 * inputs - filename -- 32 character buffer with file name
 * outputs - FS_FAIL
 * side effects - none
 */
int32_t file_write(int32_t *fd, void* buf, int32_t nbytes) {
    /* modification needed*/
    return FS_FAIL;
}

/* file_read
 * read from a file and place data into buffer
 * inputs - fd -- inode number representing the open file
 *          offset -- number of bytes from beginning of file
 *                    to start copying from
 *          buf -- buffer to fill with data
 *          length -- length of buffer.
 * outputs - FS_FAIL/FS_PASS
 * side effects - fills buffer with file data.
 */
int32_t file_read(int32_t fd, int32_t *offset, uint8_t* buf, int32_t length) {
    if(!boot_block) return FS_FAIL;
    if(!offset || !buf) return FS_FAIL;

    int32_t bytes_read = read_data((uint32_t)fd, (uint32_t)*offset, buf, (uint32_t)length);
    if(bytes_read > 0) *offset += bytes_read; // so next read continues.
    return bytes_read;
}

/* dir_open
 * open a directory for reading, and sets the inode value
 * inputs - filename -- 32 character buffer with file name
 *          inode -- pointer to inode value to update.
 * outputs - FS_FAIL/FS_PASS
 * side effects - fills buffer with file data.
 */
int32_t dir_open(char* filename, int32_t* inode) {
    if(!boot_block) return FS_FAIL;
    if(!filename || !inode) return FS_FAIL;

    dentry_t d;
    if (read_dentry_by_name ((char *)filename, &d) == FS_FAIL)
        return FS_FAIL;
    if (d.filetype != DIRECTORY_TYPE)
        return FS_FAIL;
    *inode = d.inode_num;
    return FS_PASS;
}

/* dir_close
 * close a directory after reading by clearing inode value
 * inputs - filename -- 32 character buffer with file name
 *          inode -- pointer to inode value to update.
 * outputs - FS_FAIL/FS_PASS
 * side effects - clears inode input value.
 */
int32_t dir_close(int32_t* inode) {
    if(!boot_block) return FS_FAIL;
    if(!inode) return FS_FAIL;
    
    *inode = FS_FAIL;
    return FS_PASS;
}

/* dir_write
 * write to a dir. Goes straight to FS_FAIL.
 * inputs - filename -- 32 character buffer with file name
 * outputs - FS_FAIL
 * side effects - none.
 */
int32_t dir_write(int32_t *fd, void* buf, int32_t nbytes) {
    return FS_FAIL;
}

/* dir_read
 * read filenames from a directory.
 * inputs - filename -- 32 character buffer with file name
 *          buf -- buffer to fill with 
 * outputs - FS_FAIL
 * side effects - none.
 */
int32_t dir_read(int32_t fd, int32_t *offset, uint8_t* buf, int32_t nbytes) {
    if(!boot_block) return FS_FAIL;
    if(!buf || *offset > DENTRY_MAX) return FS_FAIL;

    dentry_t d;
    if ((read_dentry_by_index((uint32_t)*offset, &d)) == FS_FAIL)
        return FS_PASS;
    uint32_t bytes_to_copy = (nbytes < FILENAME_LEN) ? nbytes : FILENAME_LEN;
    memcpy((char*)buf, (char*)(d.filename), bytes_to_copy);
    *offset = *offset + 1;
    return bytes_to_copy;
}

/* read_dentry_by_name
 * description: find a dentry on given name
 * input: fname -- given name for file
 *        dentry -- pointer to load dentry
 * output: 0/-1 -- search FS_FAIL indicator
 * side-effect: dentry changed
 */
int32_t read_dentry_by_name (const char* fname, dentry_t* dentry) {
    if(!boot_block) return FS_FAIL;
    if(!fname || !dentry) return FS_FAIL;

    int i;
    dentry_t d;
    uint32_t len;
    len = strlen((char*)fname); // storing lenth of fname
    if(len <= 0) return FS_FAIL;
    
    for (i = 0; i < DENTRY_MAX; i++) {
        if (read_dentry_by_index (i,&d) == FS_FAIL)
            return FS_FAIL; // point d to a new dentry
        if (strncmp((char*)fname, d.filename, FILENAME_LEN) == 0) { // check if same name
            *dentry = d; // store found dentry struct
            return FS_PASS; // success
        }
    }
    return FS_FAIL; // no file found
}

/* read_dentry_by_index
 * description: fill a dentry based on index in directory
 * input: index -- file index for file
 *        dentry -- pointer to load dentry
 * output: 0/-1 -- search FS_FAIL indicator
 * side-effect: dentry changed
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
    if(!boot_block) return FS_FAIL;
    if(!dentry || index >= boot_block->dir_count) return FS_FAIL;
    
    int i;
    memcpy((char*)dentry->filename, (char*)(boot_block->direntries[index].filename), FILENAME_LEN);
    dentry->filetype = boot_block->direntries[index].filetype;
    dentry->inode_num = boot_block->direntries[index].inode_num;
    for (i = 0; i < DENTRY_RESERVE; i++) {
        dentry->reserved[i] = boot_block->direntries[index].reserved[i];
    }
    return FS_PASS;
}

/* read_dentry_by_index
 * description: fill a dentry based on index in directory
 * input: index -- file index for file
 *        dentry -- pointer to load dentry
 * output: 0/-1 -- search FS_FAIL indicator
 * side-effect: dentry changed
 */
int32_t read_data (uint32_t fd, uint32_t offset, uint8_t* buf, uint32_t length) {
  if(!boot_block) return FS_FAIL;
  if(fd >= boot_block->inode_count) return FS_FAIL;
  if(!buf) return FS_FAIL;

  inode_t *node = (inode_t *)boot_block + (INODE_OFFSET + fd);
  // nothing to copy
  if(offset > node->length) return FS_FAIL;
  // make sure copy doesn't go past end
  if(node->length < offset + length)
    length = node->length - offset;
  
  int32_t i = (offset)/BLOCK_SIZE, bytes_copied = 0;
  uint32_t data_start = offset;
  uint32_t data_end = offset + length;

  uint32_t curr_ptr = data_start;
  while(curr_ptr < data_end) {
      data_block_t *data = (data_block_t *)boot_block + boot_block->inode_count + INODE_OFFSET + node->data_block_num[i];

      uint32_t block_end = (curr_ptr + BLOCK_SIZE > data_end) ? data_end : curr_ptr + (BLOCK_SIZE - curr_ptr % BLOCK_SIZE);
      
      memcpy((char*)buf+bytes_copied, (char*)data + (curr_ptr % BLOCK_SIZE) , block_end-curr_ptr);
      bytes_copied += block_end-curr_ptr;
      curr_ptr = data_start + bytes_copied;
      i++;
  }
  return bytes_copied;
}

/* dir_num
 * find the number of files in the main directory.
 * inputs - none
 * outputs - number of files in toplevel directory
 * side effects - none
 */
uint32_t dir_num () {
    return boot_block->dir_count;
}

/* print_dentry_details
 * format and print important file details to the screen
 * inputs - fname -- 32 character buffer
 * outputs - FS_FAIL/FS_PASS
 * side effects - prints details of a single file to screen.
 */
int32_t print_dentry_details(uint8_t* fname) {
    if(!boot_block || !fname) return FS_FAIL;

    dentry_t d;
    int32_t node;
    inode_t *inode;
    int32_t i;

    if(read_dentry_by_name((char*)fname, &d) == FS_FAIL) return FS_FAIL;
    if(file_open((char*)fname, &node) == FS_FAIL) return FS_FAIL;

    inode = (inode_t *)boot_block + (INODE_OFFSET + node);

    printf("filename:");
    for(i = 0; i < FILENAME_LEN; i++) {
        if (d.filename[i] == '\0') break;
        putc(d.filename[i]);
    }
    printf(", filetype: %d, filesize: %d\n", d.filetype, inode->length);
    return FS_PASS;
}



