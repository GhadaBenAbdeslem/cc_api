#include "ccimp/ccimp_filesystem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#ifdef UNIT_TEST
#define ccimp_fs_error_desc                 ccimp_fs_error_desc_real
#define ccimp_fs_session_error              ccimp_fs_session_error_real
#define ccimp_fs_file_open                  ccimp_fs_file_open_real
#define ccimp_fs_file_read                  ccimp_fs_file_read_real
#define ccimp_fs_file_write                 ccimp_fs_file_write_real
#define ccimp_fs_file_close                 ccimp_fs_file_close_real
#define ccimp_fs_file_seek                  ccimp_fs_file_seek_real
#define ccimp_fs_file_truncate              ccimp_fs_file_truncate_real
#define ccimp_fs_file_remove                ccimp_fs_file_remove_real
#define ccimp_fs_dir_open                   ccimp_fs_dir_open_real
#define ccimp_fs_dir_read_entry             ccimp_fs_dir_read_entry_real
#define ccimp_fs_dir_entry_status           ccimp_fs_dir_entry_status_real
#define ccimp_fs_dir_close                  ccimp_fs_dir_close_real
#define ccimp_fs_hash_status                ccimp_fs_hash_status_real
#define ccimp_fs_hash_file                  ccimp_fs_hash_file_real
#endif

#define MIN_VALUE(a, b)    ((a) < (b) ? (a) : (b))

typedef struct
{
   DIR         * dirp;
   struct dirent dir_entry;
} dir_data_t;

static int app_convert_file_open_mode(int const oflag)
{
#if (CCIMP_FILE_O_RDONLY == O_RDONLY) && (CCIMP_FILE_O_WRONLY == O_WRONLY) && (CCIMP_FILE_O_RDWR == O_RDWR) && \
    (CCIMP_FILE_O_CREAT == O_CREAT)   && (CCIMP_FILE_O_APPEND == O_APPEND) && (CCIMP_FILE_O_TRUNC == O_TRUNC)

    return oflag;
#else
    int result = 0;

    if (oflag & CCIMP_FILE_O_WRONLY) result |= O_WRONLY;
    if (oflag & CCIMP_FILE_O_RDWR)   result |= O_RDWR;
    if (oflag & CCIMP_FILE_O_APPEND) result |= O_APPEND;
    if (oflag & CCIMP_FILE_O_CREAT)  result |= O_CREAT;
    if (oflag & CCIMP_FILE_O_TRUNC)  result |= O_TRUNC;

    if ((oflag & (CCIMP_FILE_O_WRONLY | CCIMP_FILE_O_RDWR)) == 0)
        result |= O_RDONLY;

    return result;
#endif
}

ccimp_status_t ccimp_fs_file_open(ccimp_fs_file_open_t * const file_open_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    int const oflag = app_convert_file_open_mode(file_open_data->flags);
    int const fd = open(file_open_data->path, oflag, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH); /* 0664 = Owner RW + Group RW + Others R */

    if (fd < 0)
    {
        file_open_data->errnum.value = errno;
        status = CCIMP_STATUS_ERROR;
        goto done;
    }

    file_open_data->handle.value = fd;
done:
    return status;
}

ccimp_status_t ccimp_fs_file_read(ccimp_fs_file_read_t * const file_read_data)
{
   ccimp_status_t status = CCIMP_STATUS_OK;
   int result = read(file_read_data->handle.value, file_read_data->buffer, file_read_data->bytes_available);
   if (result >= 0)
   {
       file_read_data->bytes_used = result;
   }
   else
   {
       if (errno == EAGAIN)
       {
             status = CCIMP_STATUS_BUSY;
       }
       else
       {
             file_read_data->errnum.value = errno;
             status = CCIMP_STATUS_ERROR;
       }
   }
  return status;
}

ccimp_status_t ccimp_fs_file_write(ccimp_fs_file_write_t * const file_write_data)
{
   ccimp_status_t status = CCIMP_STATUS_OK;
   int result = write(file_write_data->handle.value, file_write_data->buffer, file_write_data->bytes_available);
   if (result >= 0)
   {
       file_write_data->bytes_used = result;
   }
   else
   {
       if (errno == EAGAIN)
       {
             status = CCIMP_STATUS_BUSY;
       }
       else
       {
             file_write_data->errnum.value = errno;
             status = CCIMP_STATUS_ERROR;
       }
   }

   return status;
}

ccimp_status_t ccimp_fs_file_close(ccimp_fs_file_close_t * const file_close_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    int result = close(file_close_data->handle.value);

    if (result < 0)
    {
        file_close_data->errnum.value = errno;
        status = CCIMP_STATUS_ERROR;
    }

    return status;
}

ccimp_status_t ccimp_fs_file_seek(ccimp_fs_file_seek_t * const file_seek_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    int origin;
    off_t offset;
    switch (file_seek_data->origin)
    {
       case CCIMP_SEEK_SET:
          origin = SEEK_SET;
          break;
       case CCIMP_SEEK_END:
          origin = SEEK_END;
          break;
       case CCIMP_SEEK_CUR:
          origin = SEEK_CUR;
          break;
   }
   offset = lseek(file_seek_data->handle.value, file_seek_data->requested_offset, origin);
   file_seek_data->resulting_offset = (ccimp_file_offset_t) offset;
   if (offset < 0)
   {
       if (errno == EAGAIN)
       {
           status = CCIMP_STATUS_BUSY;
       }
       else
       {
           file_seek_data->errnum.value = errno;
           status = CCIMP_STATUS_ERROR;
       }
   }

    return status;
}

ccimp_status_t ccimp_fs_file_truncate(ccimp_fs_file_truncate_t * const file_truncate_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    int result = ftruncate(file_truncate_data->handle.value, file_truncate_data->length_in_bytes);
    if (result < 0)
    {
        if (errno == EAGAIN)
        {
            status = CCIMP_STATUS_BUSY;
        }
        else
        {
            file_truncate_data->errnum.value = errno;
            status = CCIMP_STATUS_ERROR;
        }
    }
    return status;
}

ccimp_status_t ccimp_fs_file_remove(ccimp_fs_file_remove_t * const file_remove_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    int result = unlink(file_remove_data->path);
    if (result < 0)
    {
        if (errno == EAGAIN)
        {
            status = CCIMP_STATUS_BUSY;
        }
        else
        {
            file_remove_data->errnum.value = errno;
            status = CCIMP_STATUS_ERROR;
        }
    }
    return status;
}

ccimp_status_t ccimp_fs_dir_open(ccimp_fs_dir_open_t * const dir_open_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    DIR * dirp;

    dirp = opendir(dir_open_data->path);
    if (dirp != NULL)
    {
        dir_data_t * dir_data = malloc(sizeof *dir_data);
        if (dir_data != NULL)
        {
            dir_open_data->handle.pointer = dir_data;
            dir_data->dirp = dirp;
        }
        else
        {
            closedir(dirp);
            dir_open_data->errnum.value = ENOMEM;
            status = CCIMP_STATUS_ERROR;
        }
    }
    else
    {
       if (errno == EAGAIN)
       {
             status = CCIMP_STATUS_BUSY;
       }
       else
       {
             dir_open_data->errnum.value = errno;
             status = CCIMP_STATUS_ERROR;
       }
    }
    return status;
}

ccimp_status_t ccimp_fs_dir_read_entry(ccimp_fs_dir_read_entry_t * const dir_read_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    dir_data_t * dir_data = dir_read_data->handle.pointer;
    struct dirent * p_dirent;
    int error;
    /* This sample does not skip "." and ".." */
    error = readdir_r(dir_data->dirp, &dir_data->dir_entry, &p_dirent);
    if (error != 0)
    {
       if (error == EAGAIN)
       {
             status = CCIMP_STATUS_BUSY;
       }
       else
       {
             dir_read_data->errnum.value = error;
             status = CCIMP_STATUS_ERROR;
       }
    }
    else if (p_dirent == NULL)
    {
        /* Finished with the directory */
        dir_read_data->entry_name[0] = '\0';
    }
    else
    {
        /* Valid entry, copy the name */
        size_t name_len = strlen(p_dirent->d_name);
        if(name_len < dir_read_data->bytes_available)
        {
            memcpy(dir_read_data->entry_name, p_dirent->d_name, name_len + 1);
        }
        else
        {
            dir_read_data->errnum.value = ENAMETOOLONG;
            status = CCIMP_STATUS_ERROR;
        }
    }
    return status;
}

ccimp_status_t ccimp_fs_dir_entry_status(ccimp_fs_dir_entry_status_t * const dir_entry_status_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    struct stat statbuf;

    int result = stat(dir_entry_status_data->path, &statbuf);
    if (result == 0)
    {
        dir_entry_status_data->status.type  = CCIMP_FS_DIR_ENTRY_UNKNOWN;
        dir_entry_status_data->status.last_modified = (uint32_t) statbuf.st_mtim.tv_sec;
        if (S_ISDIR(statbuf.st_mode))
        {
           dir_entry_status_data->status.type = CCIMP_FS_DIR_ENTRY_DIR;
        }
        else if (S_ISREG(statbuf.st_mode))
        {
           dir_entry_status_data->status.type = CCIMP_FS_DIR_ENTRY_FILE;
           dir_entry_status_data->status.file_size = (ccimp_file_offset_t) statbuf.st_size;
        }
    }
    else
    {
       if (errno == EAGAIN)
       {
             status = CCIMP_STATUS_BUSY;
       }
       else
       {
             dir_entry_status_data->errnum.value = errno;
             status = CCIMP_STATUS_ERROR;
       }
    }
    return status;
}

ccimp_status_t ccimp_fs_dir_close(ccimp_fs_dir_close_t * const dir_close_data)
{
    dir_data_t const * const dir_data = dir_close_data->handle.pointer;
    closedir(dir_data->dirp);
    free((void *)dir_data);

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_fs_hash_status(ccimp_fs_hash_status_t * const hash_status_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    struct stat statbuf;
    int result = stat(hash_status_data->path, &statbuf);

    if (result == 0)
    {
        hash_status_data->status.type  = CCIMP_FS_DIR_ENTRY_UNKNOWN;
        hash_status_data->status.last_modified = (uint32_t) statbuf.st_mtim.tv_sec;
        if (S_ISDIR(statbuf.st_mode))
        {
           hash_status_data->status.type = CCIMP_FS_DIR_ENTRY_DIR;
        }
        else if (S_ISREG(statbuf.st_mode))
        {
           hash_status_data->status.type = CCIMP_FS_DIR_ENTRY_FILE;
           hash_status_data->status.file_size = (ccimp_file_offset_t) statbuf.st_size;
        }

        switch (hash_status_data->hash_alg.requested)
        {
            case CCIMP_FS_HASH_NONE:
            case CCIMP_FS_HASH_MD5:
            case CCIMP_FS_HASH_BEST:
                if (hash_status_data->status.type != CCIMP_FS_DIR_ENTRY_UNKNOWN)
                {
                    hash_status_data->hash_alg.actual = CCIMP_FS_HASH_MD5;
                }
                else
                {
                    hash_status_data->hash_alg.actual = CCIMP_FS_HASH_NONE;
                }
                break;

            case CCIMP_FS_HASH_CRC32:
                hash_status_data->hash_alg.actual = CCIMP_FS_HASH_NONE;
                break;
        }
    }
    else
    {
       if (errno == EAGAIN)
       {
             status = CCIMP_STATUS_BUSY;
       }
       else
       {
             hash_status_data->errnum.value = errno;
             status = CCIMP_STATUS_ERROR;
       }
    }
    return status;
}

ccimp_status_t ccimp_fs_hash_file(ccimp_fs_hash_file_t * const file_hash_data)
{
    ccimp_status_t status = CCIMP_STATUS_OK;
    switch (file_hash_data->hash_algorithm)
    {
         case CCIMP_FS_HASH_CRC32:
             /*status = app_calc_crc32(file_hash_data->path, file_hash_data->hash_value);*/
             break;
         case CCIMP_FS_HASH_MD5:
             /*status = app_calc_md5(file_hash_data->path, file_hash_data->hash_value);*/
             break;
         default:
            /* This would never happen. */
            break;
    }
    return status;
}

ccimp_status_t ccimp_fs_error_desc(ccimp_fs_error_desc_t * const error_desc_data)
{
    int errnum = error_desc_data->errnum.value;

    error_desc_data->bytes_used = 0;

    if (errnum != 0)
    {
        char * err_str = strerror(errnum);

        error_desc_data->bytes_used = MIN_VALUE(strlen(err_str), error_desc_data->bytes_available);
        memcpy(error_desc_data->error_string, err_str, error_desc_data->bytes_used);
    }

    switch(errnum)
    {
        case EACCES:
        case EPERM:
        case EROFS:
            error_desc_data->error_status = CCIMP_FS_ERROR_PERMISSION_DENIED;
            break;

        case ENOMEM:
        case ENAMETOOLONG:
            error_desc_data->error_status = CCIMP_FS_ERROR_INSUFFICIENT_MEMORY;
            break;

        case ENOENT:
        case ENODEV:
        case EBADF:
            error_desc_data->error_status = CCIMP_FS_ERROR_PATH_NOT_FOUND;
            break;

        case EINVAL:
        case ENOSYS:
        case ENOTDIR:
        case EISDIR:
            error_desc_data->error_status = CCIMP_FS_ERROR_INVALID_PARAMETER;
            break;

        case ENOSPC:
            error_desc_data->error_status = CCIMP_FS_ERROR_INSUFFICIENT_SPACE;
            break;

        default:
            error_desc_data->error_status = CCIMP_FS_ERROR_UNKNOWN;
            break;
    }

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_fs_session_error(ccimp_fs_session_error_t * const session_error_data)
{
    printf("Session error %d\n", session_error_data->session_error);
    if (session_error_data->imp_context != NULL)
    {
        free(session_error_data->imp_context);
    }

    return CCIMP_STATUS_OK;
}
