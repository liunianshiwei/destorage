#include "fcgi_config.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#include "fcgi_stdio.h"
#include "util_cgi.h"
#include "redis_op.h"
#include "make_log.h"
#include "redis_keys.h"
#define UPLOAD_LOG_MODULE "cgi"
#define UPLOAD_LOG_PROC   "upload"


/* -------------------------------------------*/
/**
 * @brief  得到上传文件所属用户的 用户名
 * @return 0 -succ 
 *         -1 fail
 */
/* -------------------------------------------*/
int get_username(char *user)
{
	char *query_string = getenv("QUERY_STRING");
	query_parse_key_value(query_string,"user",user,NULL);
	if(strlen(user) == 0)
	{
		LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "get user has no value!!", user);
		return -1;
	}
	else
	{
		LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "get user = [%s]", user);
	}
	return 0;
}


/* -------------------------------------------*/
/**
 * @brief  解析上传的post数据 保存到本地临时路径
 *         同时得到文件名称
 *
 * @param len       (in)    post数据的长度
 * @param file_name (out)   文件的文件名
 *
 * @returns   
 *          0 succ, -1 fail
 */
/* -------------------------------------------*/
int recv_save_file(int len, char *filename)
{
	
	int retn = 0;
	int i, ch;
	
	char *file_buf = NULL;
	char *begin = NULL;
	char *end = NULL;
	char *p, *q, *k;
	
	char boundary[256] = {0};
    char content_text[256] = {0};

	//==========> 开辟存放文件的 内存 <===========

	file_buf = malloc(len);//存放文件数据内容
	if (file_buf == NULL) 
	{
		printf("malloc error! file size is to big!!!!\n");
		retn = -1;
		return retn;
	}

	begin = file_buf;
	p = begin;
	
	for (i = 0; i < len; i++) 
	{
		if ((ch = getchar()) < 0) 
		{
			printf("Error: Not enough bytes received on standard input<p>\n");
			goto END;
		}
		//putchar(ch);
		*p = ch;
		p++;
	}

	//===========> 开始处理前端发送过来的post数据格式 <============
	//begin deal
	end = p;

	p = begin;

	//get boundary
	p = strstr(begin, "\r\n");
	if (p == NULL) 
	{
		printf("wrong no boundary!\n");
		goto END;
	}

	strncpy(boundary, begin, p-begin);
	boundary[p-begin] = '\0';
	//printf("boundary: [%s]\n", boundary);

	p+=2;//\r\n
	//已经处理了p-begin的长度
	len -= (p-begin);

	//get content text head
	begin = p;

	p = strstr(begin, "\r\n");
	if(p == NULL)
	{
		printf("ERROR: get context text error, no filename?\n");
		goto END;
	}
	strncpy(content_text, begin, p-begin);
	content_text[p-begin] = '\0';
	//printf("content_text: [%s]\n", content_text);

	p+=2;//\r\n
	len -= (p-begin);

	//get filename
	// filename="123123.png"
	//           ↑
	q = begin;
	q = strstr(begin, "filename=");
	
	q+=strlen("filename=");
	q++;

	k = strchr(q, '"');
	strncpy(filename, q, k-q);
	filename[k-q] = '\0';

	trim_space(filename);
	//printf("filename: [%s]\n", filename);

	//get file
	begin = p;     
	p = strstr(begin, "\r\n");
	p+=4;//\r\n\r\n
	len -= (p-begin);

	begin = p;
	// now begin -->file's begin
	//find file's end
	p = memstr(begin, len, boundary);
	if (p == NULL) {
		p = end-2;    //\r\n
	}
	else {
		p = p -2;//\r\n
	}
	
	//begin---> file_len = (p-begin)
	
	//=====> 此时begin-->p两个指针的区间就是post的文件二进制数据
	//======>将数据写入文件中,其中文件名也是从post数据解析得来  <===========
	int fd = 0;
	fd = open(filename, O_CREAT|O_WRONLY, 0644);
	if (fd < 0) {
		printf("open %s error\n", filename);
		retn = -1;
		goto END;
	}
	ftruncate(fd, (p-begin));
	write(fd, begin, (p-begin));
	close(fd); 
	
END:
	free(file_buf);
	return retn;
}


/* -------------------------------------------*/
/**
 * @brief  将一个本地文件上传到 后台分布式文件系统中
 *
 * @param filename  (in) 本地文件的路径
 * @param fileid    (out)得到上传之后的文件ID路径
 *
 * @returns   
 *      0 succ, -1 fail
 */
/* -------------------------------------------*/
int upload_to_dstorage(char *filename, char *fileid)
{
    int retn = 0;

    pid_t pid;
    int pfd[2];

    if (pipe(pfd) < 0) 
	{
        printf("pip error\n");
    }

    pid = fork();
    if (pid < 0)
	{
        printf("fork error\n");
    }

    if (pid == 0) 
	{
        //child
        //关闭读管道
        close(pfd[0]);

        //将标准输出重定向到写管道
        dup2(pfd[1], STDOUT_FILENO);

        execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf", filename, NULL);
        close(pfd[1]);
    }
	
	if(pid > 0)
	{
		//关闭写管道
		close(pfd[1]);

		//read from 读管道
		read(pfd[0],fileid, 256);
		trim_space(fileid);  
		if (strlen(fileid) == 0) 
		{  
			retn = -1;
			goto END;
		}
		wait(NULL);
		close(pfd[0]);
	}
END:
    return retn;
}


/* -------------------------------------------*/
/**
 * @brief  封装文件存储在分布式系统中的 完整 url
 *
 * @param fileid        (in)    文件分布式id路径
 * @param fdfs_file_url (out)   文件的完整url地址
 *
 * @returns   
 *      0 succ, -1 fail
 */
/* -------------------------------------------*/
int make_file_url(char *fileid, char *fdfs_file_url)
{
    int retn = 0;

    char *p = NULL;
    char *q = NULL;
    char *k = NULL;

	char fdfs_file_stat_buf[256] = {0};
    char fdfs_file_host_name[30] = {0};

    int pfd[2];

    pid_t pid;

    pipe(pfd);

    pid = fork();

    if (pid == 0) 
	{
        //child
        //关闭读管道
        close(pfd[0]);

        //将标准输出重定向到写管道
        dup2(pfd[1], STDOUT_FILENO);
        execlp("fdfs_file_info", "fdfs_file_info", "/etc/fdfs/client.conf", fileid, NULL);
    }

    if(pid > 0)
	{
		//关闭写管道
		close(pfd[1]);
		//read from 读管道
		read(pfd[0], fdfs_file_stat_buf, 256);
		
		wait(NULL);
		close(pfd[0]);

		//拼接上传文件的完整url地址--->http://host_name/group1/M00/00/00/D12313123232312.png
		p = strstr(fdfs_file_stat_buf, "source ip address: ");

		//192.168.123.123
		//q              k
		q = p + strlen("source ip address: ");
		k = strstr(q, "\n");
		strncpy(fdfs_file_host_name, q, k-q);

		fdfs_file_host_name[k-q] = '\0';

		//printf("host_name:[%s]\n", fdfs_file_host_name);

		strcat(fdfs_file_url, "http://");
		strcat(fdfs_file_url, fdfs_file_host_name);
		strcat(fdfs_file_url, ":");
		strcat(fdfs_file_url, "80");
		strcat(fdfs_file_url, "/");
		strcat(fdfs_file_url, fileid);

		printf("[%s]\n", fdfs_file_url);
		memset(fdfs_file_stat_buf, 0, 256);
        memset(fdfs_file_host_name, 0, 30);
	}
	
    return retn;
}

/**
 * @brief  将文件信息存储到redis数据库 多个hash表形式
 *
 * @param fileid
 * @param fdfs_file_url
 * @param filename
 * @param user
 *
 * @returns   
 *          0 succ, -1 fail
 */
/* -------------------------------------------*/
int store_file_to_redis(char *fileid, char *fdfs_file_url, char *filename, char *user)
{
	int retn = 0;
	redisContext * redis_conn = NULL;
	
	time_t now;
	char create_time[TIME_STRING_LEN];
	char suffix[SUFFIX_LEN];
	char user_id[10] = {0};
	char file_user_list[KEY_NAME_SIZ] = {0};
	
	redis_conn = rop_connectdb_nopwd(REDIS_SERVER_IP, REDIS_SERVER_PORT);
	if(redis_conn == NULL)
	{
		retn = -1;
		return retn;
	}
	
	//FILEID_URL_HASH
	rop_hash_set(redis_conn, FILEID_URL_HASH, fileid, fdfs_file_url);
	
	//FILEID_NAME_HASH
	rop_hash_set(redis_conn, FILEID_NAME_HASH, fileid, filename);
	
	//FILEID_TIME_HASH
    //create time
	now = time(NULL);
	strftime(create_time, TIME_STRING_LEN - 1, "%Y-%m-%d %H:%M:%S", localtime(&now)); //格式化一个时间字符串
	rop_hash_set(redis_conn, FILEID_TIME_HASH, fileid, create_time);
	
	//FILEID_USER_HASH
	rop_hash_set(redis_conn, FILEID_USER_HASH, fileid, user);
	
	//FILEID_TYPE_HASH
	get_file_suffix(filename, suffix); //得到文件后缀字符串
	rop_hash_set(redis_conn, FILEID_TYPE_HASH, fileid, suffix);
	
	//FILEID_SHARED_STATUS_HASH(文件的共享状态)
	rop_hash_set(redis_conn, FILEID_SHARED_STATUS_HASH, fileid, "0");
	
	//将指定的zset表，对应的成员，值自增1
	//将文件插入到FILE_HOT_ZSET中,默认的全值是1
	rop_zset_increment(redis_conn, FILE_HOT_ZSET, fileid);
	
	//将FILEID插入到用户私有列表中FILE_USER_LIST
	rop_hash_get(redis_conn, USER_USERID_HASH,  user, user_id);
    sprintf(file_user_list, "%s%s", FILE_USER_LIST, user_id);
    rop_list_push(redis_conn, file_user_list, fileid);
	
	//给指定的hash表 指定的field对应的value自增num
	//文件引用计数+1
    rop_hincrement_one_field(redis_conn,FILE_REFERENCE_COUNT_HASH , fileid, 1);
	
	//关闭连接
	rop_disconnect(redis_conn);
	return retn;
}

int main ()
{
    char user[256] = {0};
    char filename[256] = {0};
	char fileid[256] = {0};
    char fdfs_file_path[256] = {0};

    char fdfs_file_url[512] = {0};
    


    while (FCGI_Accept() >= 0) 
	{
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        printf("Content-type: text/html\r\n"
                "\r\n");
		
		if (get_username(user) < 0) {
            break;
        }
		
        if (contentLength != NULL) 
		{
            len = strtol(contentLength, NULL, 10);
        }
        else 
		{
            len = 0;
        }

        if (len <= 0) 
		{
            printf("No data from standard input\n");
        }
        else 
		{
                //===============> 得到上传文件  <============
			if(recv_save_file(len, filename) < 0)
			{
				goto END;
			}				

            //===============> 将该文件存入fastDFS中,并得到文件的file_id <============
			if(upload_to_dstorage(filename, fileid) < 0)
			{
				goto END;
			}

            //================ > 得到文件所存放storage的host_name <=================
			if(make_file_url(fileid, fdfs_file_url) < 0)
			{
				goto END;
			}
			
			//================> 删除本地临时存放的上传文件 <===============
            unlink(filename);
			
			//===============> 将该文件的fastDFS路径和名称和上传者 存入redis中 <======
            if (store_file_to_redis(fileid, fdfs_file_url, filename, user) < 0) 
			{
                goto END;
            }
		END:		  
            memset(filename, 0, 256);
            memset(fdfs_file_path, 0, 256);
            memset(fdfs_file_url, 0, 512);
			memset(user, 0, 256);
      
            //printf("date: %s\r\n", getenv("QUERY_STRING"));
        }
    } /* while */

    return 0;
}
