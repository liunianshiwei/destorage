#!/bin/bash
fcgi_upload_ID=`ps aux | grep "fcgi_upload" | grep -v grep | awk '{print $2}'`
fcgi_data_ID=`ps aux | grep "fcgi_data" | grep -v grep | awk '{print $2}'`
fcgi_reg_ID=`ps aux | grep "fcgi_reg" | grep -v grep | awk '{print $2}'`
fcgi_login_ID=`ps aux | grep "fcgi_login" | grep -v grep | awk '{print $2}'`

echo $fcgi_upload_ID
echo $fcgi_data_ID
echo $fcgi_reg_ID
echo $fcgi_login_ID
kill -9 $fcgi_upload_ID
kill -9 $fcgi_data_ID
kill -9 $fcgi_reg_ID
kill -9 $fcgi_login_ID
spawn-fcgi -a 127.0.0.1 -p 8013 -f ./fcgi_upload
spawn-fcgi -a 127.0.0.1 -p 8014 -f ./fcgi_data
spawn-fcgi -a 127.0.0.1 -p 8015 -f ./fcgi_reg
spawn-fcgi -a 127.0.0.1 -p 8016 -f ./fcgi_login
