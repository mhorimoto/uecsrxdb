#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

MYSQL *db_init(void) {
  MYSQL *conn     = NULL;
  MYSQL_ROW row;
  char *sql_serv  = "localhost";
  char *user      = "uecs";
  char *passwd    = "uecsdbp";
  char *db_name   = "uecs_welzo_house";
  int  i;
  
  //  memset( &sql_str[0] , 0x00 , sizeof(sql_str) );

  // mysql接続
  conn = mysql_init(NULL);
  if( !mysql_real_connect(conn,sql_serv,user,passwd,db_name,0,NULL,0) ){
    // error
    exit(-2);
  }
  return(conn);
}

MYSQL_RES *db_insert(MYSQL *cn,char *str) {
  MYSQL_RES *resp = NULL;
  char sql_str[256];

  sprintf(sql_str,"INSERT INTO data (tod,room,region,ord,priority,value,ip,ccmtype) values %s",str);
  //  printf("%s\n",sql_str);
  // Exec query
  if( mysql_query( cn , sql_str ) ){
    // error
    printf("ERROR\n");
    mysql_close(cn);
    exit(-3);
  }
  // Get response
  resp = mysql_use_result(cn);
  return(resp);
}

/*
  while((row = mysql_fetch_row(resp)) != NULL ){
    for(i=0;i<8;i++) {
      printf("%s,",row[i]);
    }
    printf("%s\n",row[i]);
  }

  // 後片づけ
  mysql_free_result(resp);
  mysql_close(conn);
  return 0;
}
*/

