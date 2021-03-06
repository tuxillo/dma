/*
 * Copyright (c) 2010-2014, Simon Schubert <2@0x2c.org>.
 * Copyright (c) 2008 The DragonFly Project.  All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Simon Schubert <2@0x2c.org>.
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
 * 3. Neither the name of The DragonFly Project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific, prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * This binary is setuid root.  Use extreme caution when touching
 * user-supplied information.  Keep the root window as small as possible.
 */

#include <sys/param.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <paths.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

#include "dma.h"

/*
 * Create a mbox in /var/mail for a given user, or make sure
 * the permissions are correct for dma.
 */

int
main(int argc, char **argv)
{
	const char *user;
	uid_t user_uid;
	gid_t mail_gid;
	int error;
	char fn[PATH_MAX+1];
	int f;

	openlog("dma-mbox-create", 0, LOG_MAIL);

	mail_gid = dma_drop_grpriv();

	/*
	 * We take exactly one argument: the username.
	 */
	if (argc != 2) {
		errno = 0;
		logfail("no arguments");
	}
	user = argv[1];

	syslog(LOG_NOTICE, "creating mbox for `%s'", user);

	user_uid = dma_getuser(user);

	error = snprintf(fn, sizeof(fn), "%s/%s", _PATH_MAILDIR, user);
	if (error < 0 || (size_t)error >= sizeof(fn)) {
		if (error >= 0) {
			errno = 0;
			logfail("mbox path too long");
		}
		logfail("cannot build mbox path for `%s/%s'", _PATH_MAILDIR, user);
	}

	f = open(fn, O_RDONLY|O_CREAT, 0600);
	if (f < 0)
		logfail("cannot open mbox `%s'", fn);

	if (fchown(f, user_uid, mail_gid))
		logfail("cannot change owner of mbox `%s'", fn);

	if (fchmod(f, 0620))
		logfail("cannot change permissions of mbox `%s'", fn);

	/* file should be present with the right owner and permissions */

	syslog(LOG_NOTICE, "successfully created mbox for `%s'", user);

	return (0);
}
