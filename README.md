## Synopsis

dntree is an example implementation that shows how LDAP DNs can be stored in an [lmdb](http://lmdb.tech/doc/) database, structured as a hierarchical adjecency list.

## Code Example

An example showing how dntree can be used is given in `dntree-example1.c`.

## Motivation

I was curious to learn how to use lmdb. At univention we use the back-mdb backend, which is very reliable and performant.
So lmdb seems like a good choice for LDAP DN related stuff. But lmdb limits key size to [511 bytes](http://lmdb.tech/doc//group__internal.html#gac929399f5d93cef85f874b9e9b1d09e0) by [default](https://bugzilla.redhat.com/show_bug.cgi?id=1086784#c5). So, to use lmdb in some contexts dealing with LDAP DNs (or file paths for that matter) it is useful to map DN values to ID keys and vice versa.

## API Reference

The API, as defined by `dntree.h` is bound to change as this experiment continues.

## Tests

```
make
./dntree-example1 -d 3
```

`libldap` and `liblmdb` need to be installed for this.

## Contributors

Currently this code is maintained by the author, feedback is welcome.

## License

Copyright 2016 Univention GmbH
Copyright 2016 Arvid Requate

The source code of this program is made available
under the terms of the GNU Affero General Public License version 3
(GNU AGPL V3) as published by the Free Software Foundation.

This code is derived from dn2id.c written by Howard Chu.
The code of dn2id.c has been published under [OpenLDAP](http://www.openldap.org/) Public License.  
A copy of that license is available [online](http://www.OpenLDAP.org/license.html).
