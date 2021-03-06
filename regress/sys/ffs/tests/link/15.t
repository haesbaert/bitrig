# $FreeBSD: src/tools/regression/fstest/tests/link/15.t,v 1.1 2007/01/17 01:42:09 pjd Exp $

desc="link returns ENOSPC if the directory in which the entry for the new link is being placed cannot be extended because there is no space left on the file system containing the directory"

n0=`namegen`
n1=`namegen`
n2=`namegen`

expect 0 mkdir ${n0} 0755
COUNT=256
NEWFSARGS=
# Leave space for the log.
[ "${WAPBL}" ] && { COUNT=2048; NEWFSARGS="-s -1792k"; }
dd if=/dev/zero of=tmpdisk bs=1k count=${COUNT} 2>/dev/null
vnconfig vnd1 tmpdisk
newfs ${NEWFSARGS} /dev/rvnd1c >/dev/null
mountfs /dev/vnd1c ${n0}
expect 0 create ${n0}/${n1} 0644
i=0
while :; do
	if ! ln ${n0}/${n1} ${n0}/${i} >/dev/null 2>&1 ; then
		break
	fi
	i=`expr $i + 1`
done
expect ENOSPC link ${n0}/${n1} ${n0}/${n2}
umount /dev/vnd1c
vnconfig -u vnd1
rm tmpdisk
expect 0 rmdir ${n0}
