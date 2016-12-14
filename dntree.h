/*
 *  header information for dntree.c
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

#ifndef _DNTREE_H_
#define _DNTREE_H_

#include <stdbool.h>
#include <lmdb.h>
#include <ldap.h>

typedef struct mdb_ctx {
	MDB_env		*env;
	MDB_dbi		dbi;
	MDB_txn		*txn;
	MDB_cursor	*cur;
} mdb_ctx;

#define	SUBDN_TYPE_NODE 0
#define	SUBDN_TYPE_LINK 1

typedef unsigned char SUBDNTYPE;
typedef unsigned long DNID;
typedef struct subDN {
	DNID id;
	SUBDNTYPE type;
	char data;
} subDN;

int dnid_init(mdb_ctx *mdb_ctx);
int dntree_get_id4dn(MDB_cursor *cursor, char *dn, DNID *dnid, bool create);
int dntree_lookup_dn4id(MDB_cursor *cur, DNID dnid, char **dn);
int dntree_del_id(MDB_cursor *cursor, DNID dnid);
int dntree_del_ldapdn(MDB_cursor *cursor, LDAPDN dn);

extern void dntree_log(int loglevel, const char *fmt, ...);

#endif /* _DNTREE_H_ */
