/*
 *  example usage for dntree
 *
 * Copyright 2016 Univention GmbH
 * Copyright 2016 Arvid Requate
 *
 * http://www.univention.de/
 *
 * All rights reserved.
 *
 * The source code of this program is made available
 * under the terms of the GNU Affero General Public License version 3
 * (GNU AGPL V3) as published by the Free Software Foundation.
 *
 * Binary versions of this program provided by Univention to you as
 * well as other copyrighted, protected or trademarked materials like
 * Logos, graphics, fonts, specific documentations and configurations,
 * cryptographic keys etc. are subject to a license agreement between
 * you and Univention and not subject to the GNU AGPL V3.
 *
 * In the case you use this program under the terms of the GNU AGPL V3,
 * the program is provided in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License with the Debian GNU/Linux or Univention distribution in file
 * /usr/share/common-licenses/AGPL-3; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include "dntree.h"

unsigned int LOGLEVEL = 1;

static int db_init(mdb_ctx *mdb_ctx)
{
	int		rv;
	MDB_val		key, data;
	struct stat st = {0};

	if (stat("./testdb", &st) == -1) {
		mkdir("./testdb", 0700);
	}

	rv = mdb_env_create(&mdb_ctx->env);
	if (rv != MDB_SUCCESS) {
		dntree_log(0, "%s: failed: %s (%d)",
		           __func__, mdb_strerror(rv), rv);
		return rv;
	};
	rv = mdb_env_set_mapsize(mdb_ctx->env, 10485760);
	if (rv != MDB_SUCCESS) {
		dntree_log(0, "%s: failed: %s (%d)",
		           __func__, mdb_strerror(rv), rv);
		return rv;
	};
	rv = mdb_env_set_maxdbs(mdb_ctx->env, 4);
	if (rv != MDB_SUCCESS) {
		dntree_log(0, "%s: failed: %s (%d)",
		           __func__, mdb_strerror(rv), rv);
		return rv;
	};
	rv = mdb_env_open(mdb_ctx->env, "./testdb", 0, 0664);
	if (rv != MDB_SUCCESS) {
		dntree_log(0, "%s: failed: %s (%d)",
		           __func__, mdb_strerror(rv), rv);
		return rv;
	};
	rv = mdb_txn_begin(mdb_ctx->env, NULL, 0, &mdb_ctx->txn);
	if (rv != MDB_SUCCESS) {
		dntree_log(0, "%s: failed: %s (%d)",
		           __func__, mdb_strerror(rv), rv);
		return rv;
	};

	rv = dnid_init(mdb_ctx);
	if (rv != MDB_SUCCESS) {
		mdb_txn_abort(mdb_ctx->txn);
		return rv;
	};

	rv = mdb_txn_commit(mdb_ctx->txn);
	if (rv != MDB_SUCCESS) {
		dntree_log(0, "%s: failed: %s (%d)",
		           __func__, mdb_strerror(rv), rv);
		return rv;
	};

	return MDB_SUCCESS;
}

static int dnid_txn_start(mdb_ctx *mdb_ctx)
{
	int		rv;
	dntree_log(3, "%s", __func__);

	rv = mdb_txn_begin(mdb_ctx->env, NULL, 0, &mdb_ctx->txn);
	if (rv != MDB_SUCCESS) {
		dntree_log(0, "%s: failed: %s (%d)",
		           __func__, mdb_strerror(rv), rv);
		return rv;
	}
	rv = mdb_cursor_open(mdb_ctx->txn, mdb_ctx->dbi, &mdb_ctx->cur);
	if (rv != MDB_SUCCESS) {
		dntree_log(0, "%s: failed: %s (%d)",
		           __func__, mdb_strerror(rv), rv);
		return rv;
	}

	return MDB_SUCCESS;
}

static int dnid_txn_commit(mdb_ctx *mdb_ctx)
{
	int		rv;
	dntree_log(3, "%s", __func__);

	/* not required, mdb_txn_commit does it for write txn
	if (mdb_ctx->cur) {
		mdb_cursor_close(mdb_ctx->cur);
		mdb_ctx->cur = NULL;
	}
	*/
	rv = mdb_txn_commit(mdb_ctx->txn);
	if (rv != MDB_SUCCESS) {
		dntree_log(0, "%s: failed: %s (%d)",
		           __func__, mdb_strerror(rv), rv);
		return rv;
	}

	return MDB_SUCCESS;
}

static void dnid_close(mdb_ctx *mdb_ctx)
{
	mdb_close(mdb_ctx->env, mdb_ctx->dbi);
	mdb_env_close(mdb_ctx->env);
}

void dntree_log(int loglevel, const char *fmt, ...) {
	va_list args;
	if (loglevel <= LOGLEVEL) {
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		va_end(args);
	}
}

int main(int argc, char *argv[]) {
	int		opt, rv, i, j = 0;
	mdb_ctx		mdb_ctx;
	LDAPDN		ldapdn;
	DNID		dnid;
	char		*dn;
	char		*tmpdn;

	// some example data
	char		*dnlist[]={
		"cn=foo,cn=bar,cn=baz,dc=domain,dc=local",
		"cn=foo\\,bar+cn=baz,dc=domain,dc=local",
		"cn=bar,cn=bar,cn=baz,dc=domain,dc=local",
		NULL
	};

	while ((opt = getopt(argc, argv, "d:")) != -1) {
		switch (opt) {
			case 'd': LOGLEVEL=atoi(optarg); break;
			default:
				  fprintf(stderr, "Usage: %s [-d <loglevel>]\n", argv[0]);
				  exit(EXIT_FAILURE);
		}
	}

	rv = db_init(&mdb_ctx);
	if (rv != MDB_SUCCESS) {
		return rv;
	}

	printf("Test %d: Delete DNs\n", ++j);
	i = 0;
	for (dn=dnlist[i]; dnlist[i]; dn=dnlist[++i]) {
		rv = dnid_txn_start(&mdb_ctx);
		if (rv != MDB_SUCCESS) {
			return rv;
		}

		rv = dntree_del_dn(mdb_ctx.cur, dn);
		if (rv != MDB_SUCCESS && rv != MDB_NOTFOUND) {
			mdb_txn_abort(mdb_ctx.txn);
			return rv;
		}

		rv = dnid_txn_commit(&mdb_ctx);
		if (rv != MDB_SUCCESS) {
			return rv;
		}
	}

	printf("Test %d: Add DNs\n", ++j);
	i = 0;
	for (dn=dnlist[i]; dnlist[i]; dn=dnlist[++i]) {
		rv = dnid_txn_start(&mdb_ctx);
		if (rv != MDB_SUCCESS) {
			return rv;
		}

		rv = dntree_get_id4dn(mdb_ctx.cur, dn, &dnid, true);
		if (rv != MDB_SUCCESS) {
			mdb_txn_abort(mdb_ctx.txn);
			return rv;
		}

		rv = dnid_txn_commit(&mdb_ctx);
		if (rv != MDB_SUCCESS) {
			return rv;
		}
	}

	printf("Test %d: Lookup DNs\n", ++j);
	i = 0;
	for (dn=dnlist[i]; dnlist[i]; dn=dnlist[++i]) {
		rv = dnid_txn_start(&mdb_ctx);
		if (rv != MDB_SUCCESS) {
			return rv;
		}

		rv = dntree_get_id4dn(mdb_ctx.cur, dn, &dnid, true);
		if (rv != MDB_SUCCESS) {
			mdb_txn_abort(mdb_ctx.txn);
			return rv;
		}

		rv = dntree_lookup_dn4id(mdb_ctx.cur, dnid, &tmpdn);
		if (rv != MDB_SUCCESS) {
			mdb_txn_abort(mdb_ctx.txn);
			return rv;
		}
		printf("DN: %s\n", tmpdn);
		free(tmpdn);

		rv = dnid_txn_commit(&mdb_ctx);
		if (rv != MDB_SUCCESS) {
			return rv;
		}
	}

	printf("Test %d: Delete DNs again\n", ++j);
	i = 0;
	for (dn=dnlist[i]; dnlist[i]; dn=dnlist[++i]) {
		rv = dnid_txn_start(&mdb_ctx);
		if (rv != MDB_SUCCESS) {
			return rv;
		}

		rv = dntree_del_dn(mdb_ctx.cur, dn);
		if (rv != MDB_SUCCESS) {
			mdb_txn_abort(mdb_ctx.txn);
			return rv;
		}

		rv = dnid_txn_commit(&mdb_ctx);
		if (rv != MDB_SUCCESS) {
			return rv;
		}
	}

	dnid_close(&mdb_ctx);
	// printf("sizeof(subDN): %zu\n", sizeof(subDN));
	return 0;
}
