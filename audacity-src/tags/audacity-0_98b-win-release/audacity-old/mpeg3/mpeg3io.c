#include "mpeg3private.h"
#include "mpeg3protos.h"

#include <mntent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

mpeg3_fs_t* mpeg3_new_fs(char *path)
{
	mpeg3_fs_t *fs = calloc(1, sizeof(mpeg3_fs_t));
	fs->css = mpeg3_new_css();
	strcpy(fs->path, path);
	return fs;
}

int mpeg3_delete_fs(mpeg3_fs_t *fs)
{
	mpeg3_delete_css(fs->css);
	free(fs);
	return 0;
}

int mpeg3_copy_fs(mpeg3_fs_t *dst, mpeg3_fs_t *src)
{
	strcpy(dst->path, src->path);
	dst->current_byte = 0;
	return 0;
}

long mpeg3io_get_total_bytes(mpeg3_fs_t *fs)
{
/*
 * 	struct stat st;
 * 	if(stat(fs->path, &st) < 0) return 0;
 * 	return (long)st.st_size;
 */

	fseek(fs->fd, 0, SEEK_END);
	fs->total_bytes = ftell(fs->fd);
	fseek(fs->fd, 0, SEEK_SET);
	return fs->total_bytes;
}

long mpeg3io_path_total_bytes(char *path)
{
	struct stat st;
	if(stat(path, &st) < 0) return 0;
	return (long)st.st_size;
}

int mpeg3io_open_file(mpeg3_fs_t *fs)
{
/* Need to perform authentication before reading a single byte. */
	mpeg3_get_keys(fs->css, fs->path);

	if(!(fs->fd = fopen(fs->path, "rb")))
	{
		perror("mpeg3io_open_file");
		return 1;
	}

	fs->total_bytes = mpeg3io_get_total_bytes(fs);
	
	if(!fs->total_bytes)
	{
		fclose(fs->fd);
		return 1;
	}
	fs->current_byte = 0;
	return 0;
}

int mpeg3io_close_file(mpeg3_fs_t *fs)
{
	if(fs->fd) fclose(fs->fd);
	fs->fd = 0;
	return 0;
}

int mpeg3io_read_data(unsigned char *buffer, long bytes, mpeg3_fs_t *fs)
{
	int result = 0;
	result = !fread(buffer, 1, bytes, fs->fd);
	fs->current_byte += bytes;
	return (result && bytes);
}

int mpeg3io_device(char *path, char *device)
{
	struct stat file_st, device_st;
    struct mntent *mnt;
	FILE *fp;

	if(stat(path, &file_st) < 0)
	{
		perror("mpeg3io_device");
		return 1;
	}

	fp = setmntent(MOUNTED, "r");
    while(fp && (mnt = getmntent(fp)))
	{
		if(stat(mnt->mnt_fsname, &device_st) < 0) continue;
		if(device_st.st_rdev == file_st.st_dev)
		{
			strncpy(device, mnt->mnt_fsname, MPEG3_STRLEN);
			break;
		}
	}
	endmntent(fp);

	return 0;
}

int mpeg3io_seek(mpeg3_fs_t *fs, long byte)
{
	fs->current_byte = byte;
	return fseek(fs->fd, byte, SEEK_SET);
}

int mpeg3io_seek_relative(mpeg3_fs_t *fs, long bytes)
{
	fs->current_byte += bytes;
	return fseek(fs->fd, fs->current_byte, SEEK_SET);
}

void mpeg3io_complete_path(char *complete_path, char *path)
{
	if(path[0] != '/')
	{
		char current_dir[MPEG3_STRLEN];
		getcwd(current_dir, MPEG3_STRLEN);
		sprintf(complete_path, "%s/%s", current_dir, path);
	}
	else
		strcpy(complete_path, path);
}

void mpeg3io_get_directory(char *directory, char *path)
{
	char *ptr = strrchr(path, '/');
	if(ptr)
	{
		int i;
		for(i = 0; i < ptr - path; i++)
		{
			directory[i] = path[i];
		}
		directory[i] = 0;
	}
}

void mpeg3io_get_filename(char *filename, char *path)
{
	char *ptr = strrchr(path, '/');
	if(!ptr) 
		ptr = path;
	else
		ptr++;

	strcpy(filename, ptr);
}

void mpeg3io_joinpath(char *title_path, char *directory, char *new_filename)
{
	sprintf(title_path, "%s/%s", directory, new_filename);
}
