#!/bin/bash

trackerd_PID=`ps -u  root| grep fdfs_trackerd | awk {'print $1'}`
echo $trackerd_PID
if	[ -z $trackerd_PID ]
	sudo kill -9 $trackerd_PID
	sudo fdfs_trackerd /etc/fdfs/tracker.conf
then
	sudo fdfs_trackerd /etc/fdfs/tracker.conf
fi
echo "start fdfs_trackerd sucess"


storaged_PID=`ps -u  root| grep fdfs_storaged | awk {'print $1'}`
echo $storaged_PID
if  [ -z $storaged_PID ]
	sudo kill -9 $storaged_PID
	sudo fdfs_storaged /etc/fdfs/storage.conf
then
	sudo fdfs_storaged /etc/fdfs/storage.conf
fi
echo "start fdfs_storaged sucess"
