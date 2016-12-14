#!/bin/bash
passwd="232425"
trackerd_PID=`ps -u  root| grep fdfs_trackerd | awk {'print $1'}`
echo $trackerd_PID
if	[ -z $trackerd_PID ]
	echo $passwd | sudo -S kill -9 $trackerd_PID
	echo $passwd | sudo -S fdfs_trackerd /etc/fdfs/tracker.conf
then
	echo $passwd | sudo -S fdfs_trackerd /etc/fdfs/tracker.conf
fi
echo "start fdfs_trackerd sucess"


storaged_PID=`ps -u  root| grep fdfs_storaged | awk {'print $1'}`
echo $storaged_PID
if  [ -z $storaged_PID ]
	echo $passwd | sudo -S kill -9 $storaged_PID
	echo $passwd | sudo -S fdfs_storaged /etc/fdfs/storage.conf
then
	echo $passwd | sudo -S fdfs_storaged /etc/fdfs/storage.conf
fi
echo "start fdfs_storaged sucess"
