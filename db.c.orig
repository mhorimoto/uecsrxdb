#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

MYSQL *db_init(void) {
  MYSQL *conn     = NULL;
  char *sql_serv  = "localhost";
  char *user      = "__USER__";
  char *passwd    = "__PASS__";
  char *db_name   = "__DBNAME__";
  my_bool reconnect = 1;
  
  conn = mysql_init(NULL);
  if (!conn) {
    return NULL;
  }
  // 自動再接続を有効化
  mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
  
  if( !mysql_real_connect(conn,sql_serv,user,passwd,db_name,0,NULL,0) ){
    fprintf(stderr, "DB Connect Error: %s\n", mysql_error(conn));
    mysql_close(conn);
    return NULL;
  }
  return conn;
}

int db_insert(MYSQL *cn,char *str) {
  char sql_str[2048];

  if (cn == NULL) {
    return -1;
  }
  snprintf(sql_str,sizeof(sql_str),"INSERT INTO data (tod,room,region,ord,priority,value,ip,ccmtype) values %s",str);
  // Exec query
  if( mysql_query( cn , sql_str ) ){
    fprintf(stderr, "SQL Error (%u): %s | SQL: %s\n",
	    mysql_errno(cn), mysql_error(cn), sql_str);
    return -1;
  }
  return 0;
}

