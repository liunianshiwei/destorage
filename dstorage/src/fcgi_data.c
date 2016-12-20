#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fdfs_client.h"
#include "fcgi_stdio.h"
#include "cJSON.h"
#include "util_cgi.h"
#include "redis_op.h"
#include "make_log.h"

#define DATA_LOG_MODULE          "cgi"
#define DATA_LOG_PROC            "data"

extern char g_host_name[HOST_NAME_LEN];
extern char g_storage_web_port[PORT_LEN];
extern char g_web_server_port[PORT_LEN];

void increase_file_pv(char *file_id)
{
	redisContext *redis_conn = NULL;
	
	redis_conn = rop_connectdb_nopwd(REDIS_SERVER_IP, REDIS_SERVER_PORT);
    if (redis_conn == NULL) {
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "redis connected error");
        return;
    }


    rop_zset_increment(redis_conn, FILE_HOT_ZSET, file_id);


    rop_disconnect(redis_conn);
}

void print_file_list_json(int fromId, int count, char *cmd, char *username)
{
	int i = 0;
	int retn = 0;
	int score = 0;
	
	cJSON *root = NULL;
	cJSON *array = NULL;
	char *out;
	
	char filename[FILE_NAME_LEN] = {0};
	char create_time[FIELD_ID_SIZE] = {0};
	char picurl[PIC_URL_LEN] = {0};
	char suffix[8] = {0};
	char pic_name[PIC_NAME_LEN] = {0};
	char file_url[FILE_NAME_LEN] = {0};
	char fileid_list[VALUES_ID_SIZE] = {0};
	char user[USER_NAME_LEN] = {0};
	char shared_status[2] = {0};
	
	int endId = fromId + count -1;
	
	RVALUES fileid_list_values = NULL;
	int value_num;
	redisContext *redis_conn = NULL;
	
	redis_conn = rop_connectdb_nopwd(REDIS_SERVER_IP, REDIS_SERVER_PORT);
    if (redis_conn == NULL) 
	{
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "redis connected error");
        return;
    }
	
	LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "fromId:%d, count:%d",fromId, count);
	fileid_list_values = malloc(count * VALUES_ID_SIZE);
	
	if(strcmp(cmd,"newFile") == 0)
	{
		char user_id[10] = {0};
		rop_hash_get(redis_conn, USER_USERID_HASH, username, user_id);
		sprintf(fileid_list, "%s%s", FILE_USER_LIST, user_id);
	}
	retn = rop_range_list(redis_conn, fileid_list, fromId, endId, fileid_list_values, &value_num);
	if(retn < 0)
	{
		LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "redis range %s error", FILE_PUBLIC_LIST);
        rop_disconnect(redis_conn);
        return;
	}
	
	LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "value_num=%d\n", value_num);
	
	root = cJSON_CreateObject();
	array = cJSON_CreateArray();
	for(i = 0; i < value_num; i++)
	{
		//array数组
		cJSON* item = cJSON_CreateObject();
		
		//id
		cJSON_AddStringToObject(item, "id", fileid_list_values[i]);
		
		//kind
		cJSON_AddNumberToObject(item, "kind", 2);
		
		//title_m filename
		rop_hash_get(redis_conn, FILEID_NAME_HASH, fileid_list_values[i], filename);
		LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "filename=%s\n", filename);
		cJSON_AddStringToObject(item, "title_m", filename);
		
		//title_s(username)
		rop_hash_get(redis_conn, FILEID_USER_HASH, fileid_list_values[i],user);
		LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "user=%s\n", user);
		cJSON_AddStringToObject(item, "title_s", user);
		
		//time 
		rop_hash_get(redis_conn, FILEID_TIME_HASH, fileid_list_values[i], create_time);
		cJSON_AddStringToObject(item, "descrip", create_time);
		LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "create_time=%s\n", create_time);
		
		//picurl_m
		memset(picurl, 0, PIC_NAME_LEN);
		strcat(picurl, g_host_name);//http://192.168.23.222
		strcat(picurl, ":");//http://192.168.23.222:
		strcat(picurl, g_web_server_port);//http://192.168.23.222:80
		strcat(picurl, "/static/file_png/");//http://192.168.23.222:80/static/file_png/
		
		//得到文件后缀字符串
		get_file_suffix(filename, suffix);
		sprintf(pic_name,"%s.png",suffix);
		strcat(picurl, pic_name);////http://192.168.23.222:80/static/file_png/group1/M00/00/00/wKgCbFepUHGAUTOhAP_fjuN0kbA670.png
		cJSON_AddStringToObject(item, "picurl_m", picurl);
		
		//url
        rop_hash_get(redis_conn, FILEID_URL_HASH, fileid_list_values[i], file_url);

        cJSON_AddStringToObject(item, "url", file_url);
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "file_url=%s\n", file_url);
		
		 //pv
        score = rop_zset_get_score(redis_conn, FILE_HOT_ZSET, fileid_list_values[i]);
        cJSON_AddNumberToObject(item, "pv", score-1);

        //hot (文件共享状态)
        rop_hash_get(redis_conn, FILEID_SHARED_STATUS_HASH, fileid_list_values[i], shared_status);
        cJSON_AddNumberToObject(item, "hot", atoi(shared_status));


        cJSON_AddItemToArray(array, item);
	}
	
	cJSON_AddItemToObject(root, "games", array);
	out = cJSON_Print(root);
	
	LOG(DATA_LOG_MODULE, DATA_LOG_PROC,"%s", out);
    printf("%s\n", out);

    free(fileid_list_values);
    free(out);

    rop_disconnect(redis_conn);
}

int main ()
{
	char fromId[5];
    char count[5];
    char cmd[20];
    char user[USER_NAME_LEN];
    char fileId[FILE_NAME_LEN];

    while (FCGI_Accept() >= 0) 
	{

        //char buf[4096] = {0};

        //1 获取 前段的参数
		char *query = getenv("QUERY_STRING");
		LOG(DATA_LOG_MODULE, DATA_LOG_PROC,  "url: %s", getenv("QUERY_STRING"));
		
		memset(fromId, 0, 5);
		memset(count, 0 ,5);
		memset(cmd, 0, 20);
		memset(user, 0,USER_NAME_LEN);
		memset(fileId, 0, FILE_NAME_LEN);
		
		//解析url query 类似 abc=123&bbb=456 字符串
		//  cmd=newFile&fromId=0&count=8&user=
		query_parse_key_value(query, "cmd", cmd, NULL);
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC,  "cmd: %s", cmd);
		
		
		if(strcmp(cmd, "newFile") == 0)
		{
			//分别从cmd中获取fromId count user 值
			query_parse_key_value(query, "fromId", fromId, NULL);
			query_parse_key_value(query, "count",count, NULL);
			query_parse_key_value(query, "user", user, NULL);
			LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "=== fromId:%s, count:%s, cmd:%s, user:%s", fromId, count, cmd, user);
			
			printf("Content-type: text/html\r\n"
                "\r\n");
				
			print_file_list_json(atoi(fromId), atoi(count), cmd, user);
			
		}
		else if(strcmp(cmd, "increase") == 0)
		{	//文件被点击下载
			//得到点击的fileId
			query_parse_key_value(query, "fileId", fileId, NULL);
			LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "=== fileId:%s,cmd:%s", fileId,  cmd);
			
			str_replace(fileId, "%2F", "/");
			
			increase_file_pv(fileId);
			
			printf("Content-type: text/html\r\n"
                "\r\n");
		}
        
        //进程lrange 查询FILE_ID_LIST 得到fileid 

        //全部文件已经租床称了一个json字符串

        
        /* //test
        FILE *fp = NULL;
        fp = fopen("json_test_data.json", "r");
        fread(buf, 4096, 1, fp);
        //LOG(DATA_LOG_MODULE, DATA_LOG_PROC,  "%s\n", buf);
        fclose(fp);

        printf("%s\n", buf); */

    } /* while */

    return 0;
}
