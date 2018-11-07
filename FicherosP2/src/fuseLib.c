#include "fuseLib.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>
#include <linux/kdev_t.h>

/**
 * @brief Modifies the data size originally reserved by an inode, reserving or removing space if needed.
 *
 * @param idxNode inode number
 * @param newSize new size for the inode
 * @return int
 **/
int resizeNode(uint64_t idxNode, size_t newSize)
{
    NodeStruct *node = myFileSystem.nodes[idxNode];
    char block[BLOCK_SIZE_BYTES];
    int i, diff = newSize - node->fileSize;

    if(!diff)
        return 0;

    memset(block, 0, sizeof(char)*BLOCK_SIZE_BYTES);

    /// File size increases
    if(diff > 0) {

        /// Delete the extra conent of the last block if it exists and is not full
        if(node->numBlocks && node->fileSize % BLOCK_SIZE_BYTES) {
            int currentBlock = node->blocks[node->numBlocks - 1];

            if( readBlock(&myFileSystem, currentBlock, &block)==-1 ) {
                fprintf(stderr,"Error reading block in resizeNode\n");
                return -EIO;
            }

            int offBlock = node->fileSize % BLOCK_SIZE_BYTES;
            int bytes2Write = (diff > (BLOCK_SIZE_BYTES - offBlock)) ? BLOCK_SIZE_BYTES - offBlock : diff;
            for(i = 0; i < bytes2Write; i++) {
                block[offBlock++] = 0;
            }

            if( writeBlock(&myFileSystem, currentBlock, &block)==-1 ) {
                fprintf(stderr,"Error writing block in resizeNode\n");
                return -EIO;
            }
        }

        /// File size in blocks after the increment
        int newBlocks = (newSize + BLOCK_SIZE_BYTES - 1) / BLOCK_SIZE_BYTES - node->numBlocks;
        if(newBlocks) {
            memset(block, 0, sizeof(char)*BLOCK_SIZE_BYTES);

            // We check that there is enough space
            if(newBlocks > myFileSystem.superBlock.numOfFreeBlocks)
                return -ENOSPC;

            myFileSystem.superBlock.numOfFreeBlocks -= newBlocks;
            int currentBlock = node->numBlocks;
            node->numBlocks += newBlocks;

            for(i = 0; currentBlock != node->numBlocks; i++) {
                if(myFileSystem.bitMap[i] == 0) {
                    myFileSystem.bitMap[i] = 1;
                    node->blocks[currentBlock] = i;
                    currentBlock++;
                    // Clean disk (necessary for truncate)
                    if( writeBlock(&myFileSystem, i, &block)==-1 ) {
                        fprintf(stderr,"Error writing block in resizeNode\n");
                        return -EIO;
                    }
                }
            }
        }
        node->fileSize += diff;

    }
    /// File decreases
    else {
        // File size in blocks after truncation
        int numBlocks = (newSize + BLOCK_SIZE_BYTES - 1) / BLOCK_SIZE_BYTES;
        myFileSystem.superBlock.numOfFreeBlocks += (node->numBlocks - numBlocks);

        for(i = node->numBlocks; i > numBlocks; i--) {
            int nBloque = node->blocks[i - 1];
            myFileSystem.bitMap[nBloque] = 0;
            // Clean disk (it is not really necessary)
            if( writeBlock(&myFileSystem, nBloque, &block)==-1 ) {
                fprintf(stderr,"Error writing block in resizeNode\n");
                return -EIO;
            }
        }
        node->numBlocks = numBlocks;
        node->fileSize += diff;
    }
    node->modificationTime = time(NULL);

    sync();

    /// Update all the information in the backup file
    updateSuperBlock(&myFileSystem);
    updateBitmap(&myFileSystem);
    updateNode(&myFileSystem, idxNode, node);

    return 0;
}

/**
 * @brief It formats the access mode of a file so it is compatible on screen, when it is printed
 *
 * @param mode mode
 * @param str output with the access mode in string format
 * @return void
 **/
void mode_string(mode_t mode, char *str)
{
    str[0] = mode & S_IRUSR ? 'r' : '-';
    str[1] = mode & S_IWUSR ? 'w' : '-';
    str[2] = mode & S_IXUSR ? 'x' : '-';
    str[3] = mode & S_IRGRP ? 'r' : '-';
    str[4] = mode & S_IWGRP ? 'w' : '-';
    str[5] = mode & S_IXGRP ? 'x' : '-';
    str[6] = mode & S_IROTH ? 'r' : '-';
    str[7] = mode & S_IWOTH ? 'w' : '-';
    str[8] = mode & S_IXOTH ? 'x' : '-';
    str[9] = '\0';
}

/**
 * @brief Obtains the file attributes of a file just with the filename
 *
 * Help from FUSE:
 *
 * The 'st_dev' and 'st_blksize' fields are ignored. The 'st_ino' field is ignored except if the 'use_ino' mount option is given.
 *
 *		struct stat {
 *			dev_t     st_dev;     // ID of device containing file
 *			ino_t     st_ino;     // inode number
 *			mode_t    st_mode;    // protection
 *			nlink_t   st_nlink;   // number of hard links
 *			uid_t     st_uid;     // user ID of owner
 *			gid_t     st_gid;     // group ID of owner
 *			dev_t     st_rdev;    // device ID (if special file)
 *			off_t     st_size;    // total size, in bytes
 *			blksize_t st_blksize; // blocksize for file system I/O
 *			blkcnt_t  st_blocks;  // number of 512B blocks allocated
 *			time_t    st_atime;   // time of last access
 *			time_t    st_mtime;   // time of last modification (file's content were changed)
 *			time_t    st_ctime;   // time of last status change (the file's inode was last changed)
 *		};
 *
 * @param path full file path
 * @param stbuf file attributes
 * @return 0 on success and <0 on error
 **/
static int my_getattr(const char *path, struct stat *stbuf)
{
    NodeStruct *node;
    int idxDir;

    fprintf(stderr, "--->>>my_getattr: path %s\n", path);

    memset(stbuf, 0, sizeof(struct stat));

    /// Directory attributes
    if(strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        stbuf->st_uid = getuid();
        stbuf->st_gid = getgid();
        stbuf->st_mtime = stbuf->st_ctime = myFileSystem.superBlock.creationTime;
        return 0;
    }

    /// Rest of the world
    if((idxDir = findFileByName(&myFileSystem, (char *)path + 1)) != -1) {
        node = myFileSystem.nodes[m