#!/bin/bash -e

[ $UID -eq 0 ] || exec sudo $0 "$@"

data_size () {
  DATA_SIZE=`du -cshx -X exclude /data |tail -1 |cut -d\t -f1`
  echo "uncompressed data size: $DATA_SIZE"
  LAST_BACKUP=`stat /home/jminter/.last_backup |sed -ne '/Modify/ { s/.*: //; s/\..*//; p }' || true`
  echo "last backup: $LAST_BACKUP"
}

DATE=`date +%Y%m%d%H%M`
if [[ "$1" = "remote" ]]; then
  data_size
  grep -q /mnt/nas /proc/mounts || mount /mnt/nas
  if [ $? -ne 0 ]; then
    exit 1
  fi
  tar -cvX exclude /etc /data | pigz -9 >/mnt/nas/rh-laptop-3/backup-$DATE.tar.gz.partial
  mv /mnt/nas/rh-laptop-3/backup-$DATE.tar.gz.partial /mnt/nas/rh-laptop-3/backup-$DATE.tar.gz
  tar -cvX exclude-personal /data/home/jminter/personal | pigz -9 >/mnt/nas/rh-laptop-3/backup-$DATE-personal.tar.gz.partial
  mv /mnt/nas/rh-laptop-3/backup-$DATE-personal.tar.gz.partial /mnt/nas/rh-laptop-3/backup-$DATE-personal.tar.gz
  ls -ld /data/home/jminter/downloads/* >/mnt/nas/rh-laptop-3/backup-$DATE.txt
  cp exclude /mnt/nas/rh-laptop-3/backup-$DATE-exclude.txt
  cp exclude-personal /mnt/nas/rh-laptop-3/backup-$DATE-exclude-personal.txt
  >/data/home/jminter/.last_backup

else
  echo "usage: $0 [remote]"
  data_size

  shopt -s dotglob
  (du -x -X exclude /data )|sort -rn |grep -v thunderbird |sed 40q
fi
