/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * fs/dirent/fs_opendir.c
 *
 *   Copyright (C) 2007-2009, 2011, 2013-2014 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <tinyara/kmalloc.h>
#include <tinyara/fs/fs.h>
#include <tinyara/fs/dirent.h>

#include "inode/inode.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: open_mountpoint
 *
 * Description:
 *   Handle the case where the inode to be opened is within a mountpoint.
 *
 * Inputs:
 *   inode -- the inode of the mountpoint to open
 *   relpath -- the relative path within the mountpoint to open
 *   dir -- the dirent structure to be initialized
 *
 * Return:
 *   On success, OK is returned; Otherwise, a positive errno is returned.
 *
 ****************************************************************************/

#ifndef CONFIG_DISABLE_MOUNTPOINT
static inline int open_mountpoint(FAR struct inode *inode, FAR const char *relpath, FAR struct fs_dirent_s *dir)
{
	int ret;

	/* The inode itself as the 'root' of mounted volume.  The actually
	 * directory is at relpath into the* mounted filesystem.
	 *
	 * Verify that the mountpoint inode  supports the opendir() method
	 */

	if (!inode->u.i_mops || !inode->u.i_mops->opendir) {
		return ENOSYS;
	}

	/* Take reference to the mountpoint inode.  Note that we do not use
	 * inode_addref() because we already hold the tree semaphore.
	 */

	inode->i_crefs++;

	/* Perform the opendir() operation */

	ret = inode->u.i_mops->opendir(inode, relpath, dir);
	if (ret < 0) {
		/* We now need to back off our reference to the inode.  We can't
		 * call inode_release() to do that unless we release the tree
		 * semaphore.  The following should be safe because:  (1) after the
		 * reference count was incremented above it should be >=1 so it should
		 * not decrement below zero, and (2) we hold the tree semaphore so no
		 * other thread should be able to change the reference count.
		 */

		inode->i_crefs--;
		DEBUGASSERT(inode->i_crefs >= 0);

		/* Negate the error value so that it can be used to set errno */

		return -ret;
	}

	return OK;
}
#endif

/****************************************************************************
 * Name: open_pseudodir
 *
 * Description:
 *   Handle the case where the inode to be opened is within the top-level
 *   pseudo-file system.
 *
 * Inputs:
 *   inode -- the inode of the mountpoint to open
 *   dir -- the dirent structure to be initialized
 *
 * Return:
 *   None
 *
 ****************************************************************************/

static void open_pseudodir(FAR struct inode *inode, FAR struct fs_dirent_s *dir)
{
	/* We have a valid pseudo-filesystem node.  Take two references on the
	 * inode -- one for the parent (fd_root) and one for the child (fd_next).
	 * Note that we do not call inode_addref because we are holding the tree
	 * semaphore and that would result in deadlock.
	 */

	inode->i_crefs += 2;
	dir->fd_root = inode;		/* Save the inode where we start */
	dir->u.pseudo.fd_next = inode;	/* This is the next node to use for readdir() */

	/* Flag the inode as belonging to the pseudo-filesystem */

#ifndef CONFIG_DISABLE_MOUNTPOINT
	DIRENT_SETPSEUDONODE(dir->fd_flags);
#endif
}

/****************************************************************************
 * Name: open_emptydir
 *
 * Description:
 *   Handle the case where the inode to be opened is an empty, directory node
 *   within the top-level pseudo-file system.  That is, it has no operations
 *   and, therefore, it must be a directory node.  But is has no children
 *   to be enumerated either.
 *
 * Inputs:
 *   dir -- the dirent structure to be initialized
 *
 * Return:
 *   None
 *
 ****************************************************************************/

static inline void open_emptydir(FAR struct fs_dirent_s *dir)
{
	/* We have a valid, but empty pseudo-filesystem node.  fd_next is NULL
	 * meaning that we are already at the end of the list of its children.
	 * fd_root is NULL so that if the directory is rewound, it will still be
	 * at the end of the list.
	 */

#if 0							/* Already nullified by kumm_zalloc */
	dir->fd_root = NULL;		/* Save the inode where we start */
	dir->u.pseudo.fd_next = NULL;	/* We are at the end of the list */
#endif

	/* Flag the inode as belonging to the pseudo-filesystem */

#ifndef CONFIG_DISABLE_MOUNTPOINT
	DIRENT_SETPSEUDONODE(dir->fd_flags);
#endif
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: opendir
 *
 * Description:
 *   The  opendir() function opens a directory stream corresponding to the
 *   directory name, and returns a pointer to the directory stream. The
 *   stream is positioned at the first entry in the directory.
 *
 * Inputs:
 *   path -- the directory to open
 *
 * Return:
 *   The opendir() function returns a pointer to the directory stream.  On
 *   error, NULL is returned, and errno is set appropriately.
 *
 *   EACCES  - Permission denied.
 *   EMFILE  - Too many file descriptors in use by process.
 *   ENFILE  - Too many files are currently open in the
 *             system.
 *   ENOENT  - Directory does not exist, or name is an empty
 *             string.
 *   ENOMEM  - Insufficient memory to complete the operation.
 *   ENOTDIR - 'path' is not a directory.
 *
 ****************************************************************************/

FAR DIR *opendir(FAR const char *path)
{
	FAR struct inode *inode = NULL;
	FAR struct fs_dirent_s *dir;
	FAR const char *relpath;
	bool isroot = false;
	int ret;

	/* If we are given 'nothing' then we will interpret this as
	 * request for the root inode.
	 */

	inode_semtake();
	if (!path || *path == 0 || strcmp(path, "/") == 0) {
		inode = root_inode;
		isroot = true;
		relpath = NULL;
	} else {
		/* We don't know what to do with relative pathes */

		if (*path != '/') {
			ret = -ENOTDIR;
			goto errout_with_semaphore;
		}

		/* Find the node matching the path. */

		inode = inode_search(&path, (FAR struct inode **)NULL, (FAR struct inode **)NULL, &relpath);
	}

	/* Did we get an inode? */

	if (!inode) {
		/* 'path' is not a does not exist. */

		ret = ENOTDIR;
		goto errout_with_semaphore;
	}

	/* Allocate a type DIR -- which is little more than an inode
	 * container.
	 */

	dir = (FAR struct fs_dirent_s *)kumm_zalloc(sizeof(struct fs_dirent_s));
	if (!dir) {
		/* Insufficient memory to complete the operation. */

		ret = ENOMEM;
		goto errout_with_semaphore;
	}

	/* Populate the DIR structure and return it to the caller.  The way that
	 * we do this depends on whenever this is a "normal" pseudo-file-system
	 * inode or a file system mountpoint.
	 */

	dir->fd_position = 0;		/* This is the position in the read stream */

	/* First, handle the special case of the root inode.  This must be
	 * special-cased here because the root inode might ALSO be a mountpoint.
	 */

	if (isroot) {
		/* Whatever payload the root inode carries, the root inode is always
		 * a directory inode in the pseudo-file system
		 */

		open_pseudodir(inode, dir);
	}

	/* Is this a node in the pseudo filesystem? Or a mountpoint?  If the node
	 * is the root (isroot == TRUE), then this is a special case.
	 */

#ifndef CONFIG_DISABLE_MOUNTPOINT
	else if (INODE_IS_MOUNTPT(inode)) {
		/* Yes, the node is a file system mountpoint */

		dir->fd_root = inode;	/* Save the inode where we start */

		/* Open the directory at the relative path */

		ret = open_mountpoint(inode, relpath, dir);
		if (ret != OK) {
			goto errout_with_direntry;
		}
	}
#endif
	else {
		/* The node is part of the root pseudo file system.  Does the inode
		 * have a child? If so that the child would be the 'root' of a list
		 * of nodes under the directory.
		 */

		FAR struct inode *child = inode->i_child;
		if (child) {
			/* It looks we have a valid pseudo-filesystem directory node. */

			open_pseudodir(child, dir);
		} else if (!inode->u.i_ops) {
			/* This is a dangling node with no children and no operations. Set
			 * up to enumerate an empty directory.
			 */

			open_emptydir(dir);
		} else {
			ret = ENOTDIR;
			goto errout_with_direntry;
		}
	}

	inode_semgive();
	return ((DIR *)dir);

	/* Nasty goto's make error handling simpler */

errout_with_direntry:
	kumm_free(dir);

errout_with_semaphore:
	inode_semgive();
	set_errno(ret);
	return NULL;
}
