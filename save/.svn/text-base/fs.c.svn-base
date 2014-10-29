#include "fs.h"

#define BLOCK 4096
#define BOOT_ENTRY_SIZE 64
#define BLOCK_SIZE 4

pcb_t *pcb;		// current pcb

/*
*	fs_open
*	Inputs: filename, curr_pcb
*	Return: 0 on success
*	Doesn't have to do anything because open already
*	occurred in the system_open function.  However, it
*	will grab a pointer to the current pcb for reference.
*/
int32_t fs_open(const uint8_t* filename, pcb_t* curr_pcb)
{	
	pcb = curr_pcb;
	return 0;
}

/*
*	fs_write
*	Inputs: inode, offset, buffer to write from, length
*	Return: -1 on failure
*	Read-only filesystem, so this will return failure.
*/
int32_t fs_write(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	return -1;
}

/*
*	fs_close
*	Inputs: file descriptor to close, current pcb
*	Return: 0 on success
*	Will clear the given file descriptor from the current pcb.
*/
int32_t fs_close(int32_t fd, pcb_t* curr_pcb)
{
	curr_pcb->file_array[fd].fops_ptr = NULL;
	curr_pcb->file_array[fd].inode_ptr = NULL;
	curr_pcb->file_array[fd].file_position = 0;
	curr_pcb->file_array[fd].flags = 0;

	return 0;
}

/*
*	fs_read_name
*	Inputs: filename, buffer to read to, length to read
*	Return: 0 on success, -1 on failure
*	Wrapper function that gets the data for the file of a given name.
*	Will use read_dentry_by_name to get the dentry, and then copy the
*	data into the given buffer.
*/
int32_t fs_read_name(uint8_t* fname, uint8_t* buf, int32_t nbytes) {
	dentry_t curr_dentry;

	// find dentry based on file name
	if (read_dentry_by_name(fname, &curr_dentry) == -1) {
		return -1;
	}

	// read data based on file type
	if (curr_dentry.type == 1) {
		return read_dir(NULL, buf);
	} else if (curr_dentry.type == 2) {
		return read_data(curr_dentry.inode, 0, buf, nbytes);
	}

	return 0;
}

/*
*	fs_read
*	Inputs: file descriptor, buffer to read to, length to read
*	Return: number of bytes read, or -1 on failure
*	The main system_read function, it will use the info in the current
*	file descriptor the read data into the given buffer, returning the
*	number of bytes that were copied.
*/
int32_t fs_read(uint32_t fd, uint8_t* buf, int32_t nbytes) {
	uint32_t inode_num;
	uint32_t check;

	// retrieve file descriptor pointer for current pcb
	fd_t* fd_ptr = (fd_t*) &(pcb->file_array[fd]);

	if (fd_ptr->inode_ptr == NULL) {
		return read_dir(fd_ptr, buf);
	}

	if (fd_ptr->file_position >= fd_ptr->inode_ptr->length) {return 0;}

	inode_num = ((uint32_t) (fd_ptr->inode_ptr) - (uint32_t) boot) / BLOCK - 1;

	check = read_data(inode_num, fd_ptr->file_position, buf, nbytes);
	if (check == -1)
	{
	 	return 0;
	} 

	fd_ptr->file_position += check;
	/*
	if (nbytes != 0 && check == 0) {fd_ptr->file_position = 0;} 
	else {fd_ptr->file_position += check;} */

	return check;
}

/*
*	read_dir
*	Inputs: file descriptor, buffer to read to
*	Return: 0 on success
*	Will read info of the given directory in the file descriptor
*	into the given buffer.
*/
int32_t read_dir(fd_t* fd, uint8_t* buf) {
	if (fd->file_position >= boot->num_directory_entry) {
		int i;

		for (i = 0; i < NAME_SIZE; i++) {
			buf[i] = 0;
		}

		return 0;
	}

	dentry_t curr_dentry;

	if (read_dentry_by_index(fd->file_position, &curr_dentry) == 0) {
		strncpy((int8_t*) buf, (int8_t*)curr_dentry.name, NAME_SIZE);
		int name_length = strlen((int8_t*)curr_dentry.name);
		fd->file_position++;
		return name_length;
	} else {
		return 0;
	}
}

/*
*	read_data
*	Inputs: inode to read, offset from start of file, buffer, length to read
*	Return: bytes read, or -1 on failure
*	Function to access the data corresponding to the given inode, and copy it into
*	the given buffer, based on an offset from the start of the file, and how many 
*	bytes to read.
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	int i;
	int block_count;
	int byte_count;
	uint8_t* buf_start = buf;
	uint32_t file_length;
	inode_t* inode_ptr;
	inode_t* inodes;
	uint32_t length_left;
	uint8_t* off_ptr;
	uint32_t bytes_to_read;
	uint32_t bytes_read;

	inodes = (inode_t*) (boot);
	inode_ptr = &(inodes[inode + 1]);

	length_left = inode_ptr->length - offset;

	if (length_left <= 0)
		return 0;

	if (inode_ptr->length > KB4 * INODE_BLOCKS)
	{
		return -1;
	}

	file_length = inode_ptr->length;

	if ((offset + length) > file_length) {
		length = file_length;
	}

	block_count = (offset + length) / KB4;
	byte_count = (offset + length) % KB4;
	bytes_to_read = length;
	i = 0;

	if (offset + length > file_length)
	{
		bytes_to_read = file_length - offset;
		block_count = bytes_to_read / BLOCK;
	}

	bytes_read = bytes_to_read;

	while (offset > BLOCK)
	{
		i++;
		block_count--;
		offset = offset - BLOCK;
	}


	while (bytes_to_read > 0)
	{
		if (offset > BLOCK)
		{
			i++;
			offset = offset % BLOCK;
		}

		if (block_count > 0)
		{

			off_ptr = (uint8_t*) &data[inode_ptr->blocks[i]] + offset;
			memcpy(buf_start, off_ptr, KB4 - offset);
			block_count--;
			bytes_to_read -= KB4 - offset;
			buf_start += KB4 - offset;
			offset = 0;
			i++;
		}

		else
		{
			off_ptr = (uint8_t*) &data[inode_ptr->blocks[i]] + offset;
			memcpy(buf_start, off_ptr, bytes_to_read);
			bytes_to_read = 0;
		}

	}

	return bytes_read;
}

/*
*	read_dentry_by_name
*	Inputs: file name, dentry to fill out.
*	Return: 0 on success, -1 on failure
*	Function to find the corresponding info for a given filename, and fill
*	out the dentry pointed to with the correct info.
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
	int i;

	for (i = 0; i < DIRECTORY_ENTRIES; i++) {
		if (strncmp((int8_t*) (boot->directory_entry[i].name), (int8_t*) (fname), NAME_SIZE - 1) == 0) {
			*dentry = boot->directory_entry[i];

			strcpy((int8_t*)(dentry->name), (int8_t*) (boot->directory_entry[i].name));
			dentry->type = boot->directory_entry[i].type;
			dentry->inode = boot->directory_entry[i].inode;
			strcpy((int8_t*)(dentry->reserved), (int8_t*) (boot->directory_entry[i].reserved));
			
			return 0;
		}
	}

	return -1;
}

/*
*	read_dentry_by_index
*	Inputs: file index, dentry to fill out.
*	Return: 0 on success, -1 on failure.
*	Function to find corresponding info for a give file index, and fill
*	out the dentry pointed to with correct info.
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
{
	int inodes;
	inodes = boot->num_inodes;

	if (index >= inodes)
	{
		return -1;
	}

	*dentry = boot->directory_entry[index];

	strcpy((int8_t*)(dentry->name), (int8_t*) (boot->directory_entry[index].name));
	dentry->type = boot->directory_entry[index].type;
	dentry->inode = boot->directory_entry[index].inode;
	strcpy((int8_t*)(dentry->reserved), (int8_t*) (boot->directory_entry[index].reserved));

	return 0;
}

/*
*	setup_fs
*	Inputs: pointer to the boot block
*	Return: void
*	Function to set up a pointer to the boot block, and to set up a pointer to
*	the start of the data blocks.
*/
void setup_fs(uint32_t boot_ptr)
{
	int inodes;
	boot = (boot_block_t*)boot_ptr;

	inodes = boot->num_inodes;

	data = (data_block_t*) (boot_ptr + BLOCK + inodes * BLOCK); // start, plus boot block, plus however many inodes, to get to data
}
