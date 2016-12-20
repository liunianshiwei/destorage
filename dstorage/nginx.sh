#!/bin/bash

passwd="axposf1.1"
nginx_master_ID=`ps -u  root| grep nginx | awk {'print $1'}`
nginx_worker_ID=`ps -u  nobody| grep nginx | awk {'print $1'}`
echo nginx_master_ID $nginx_master_ID
echo nginx_worker_ID $nginx_worker_ID
if [ ! -f $"/usr/local/nginx/logs/nginx.pid" ];then
#no nginx.pid need start
	echo $passwd | sudo -S /usr/local/nginx/sbin/nginx
	echo "start nginx sucess"
else
	echo $passwd | sudo -S /usr/local/nginx/sbin/nginx -s reload
	echo "reload nginx sucess"
fi
