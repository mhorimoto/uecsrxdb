#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <rrd.h>

#define False 0
#define True  1
#define CONFIG_FILE "/usr/local/etc/uecsrxdb.conf" // 設定ファイル名

volatile sig_atomic_t stopflag = 0;
static char version[] = "v2.20";

// パスを格納するグローバル変数（またはmain内で管理）
char logfile_path[256];
char semapd_path[256];
char ramdisk_path[256];

void abrt_handler(int sig);
extern int rep(char *, const char *, const char *, const char *);

// 設定ファイルを読み込む関数
void load_config() {
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        perror("Config file open error");
        exit(1);
    }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue; // コメントや空行を飛ばす
        
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");

        if (key && value) {
            if (strcmp(key, "LOGFILE") == 0) strncpy(logfile_path, value, 255);
            else if (strcmp(key, "SEMAPD") == 0) strncpy(semapd_path, value, 255);
            else if (strcmp(key, "RAMDISK") == 0) strncpy(ramdisk_path, value, 255);
        }
    }
    fclose(fp);
}

int main(int argc, char* argv[]) {

  int sd;
  struct sockaddr_in addr;
  MYSQL *db_init(void);
  MYSQL_RES *db_insert();
  socklen_t sin_size;
  struct sockaddr_in from_addr;
 
  char buf[2048];           // 受信バッファ
  char dbuf[2048];          // 表示バッファ
  char cbuf[256];           // check buffer
  char sqlbuf[256];         // SQL buffer
  char ipa[30];             // IPアドレスのマッチング
  char ccm[30];             // CCMのマッチング
  char ddd[11],tod[9];
  char *strf[7];            // CCM field
  char ccmstatus_name[512]; // CCM データパス名
  char semaphore_name[512]; // セマフォパス名
  int  cnt;                 // 受信カウンタ
  int  rc;
  int  c;
  int  opt_m,opt_s,opt_t,opt_i,opt_c;
  u_int yes = 1;
  time_t now;
  MYSQL *conn;
  MYSQL_RES *resp;
  FILE *fp;                   // for logfile
  FILE *ccmfp;                // for ccmstatus file
  struct tm *tm_now;

  opt_m = False;
  opt_s = True;
  opt_t = False;
  opt_i = False;
  opt_c = False;
  
  signal(SIGPIPE, SIG_IGN);
  if ( signal(SIGINT, abrt_handler) == SIG_ERR ) {
    exit(1);
  }

  // 設定の読み込み
  load_config();

  // IPv4 UDP のソケットを作成
  if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    return -1;
  }
 
  // 待ち受けるIPとポート番号を設定
  addr.sin_family = AF_INET;
  addr.sin_port = htons(16520);
  addr.sin_addr.s_addr = INADDR_ANY; // すべてのアドレス宛のパケットを受信する
  // バインドする
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return -1;
  }
 
  // 受信バッファの初期化
  memset(buf, 0, sizeof(buf));

  // 動的に読み込んだ logfile_path を使用
  fp = fopen(logfile_path, "a");
  if (fp == NULL) {
    printf("log file (%s) can not open.\n", logfile_path);
  }
  
  conn = db_init();
  while(!stopflag) {
    // 受信 パケットが到着するまでブロック
    // from_addr には、送信元アドレスが格納される
    sin_size = sizeof(struct sockaddr_in); // ★ ここで sin_size を初期化 ★
    rc = recvfrom(sd, buf, sizeof(buf), MSG_DONTWAIT,
		  (struct sockaddr *)&from_addr, &sin_size);
    if ( rc > 0 ) {
      buf[rc] = '\0';
      // 日時生成
      now = time(NULL);
      tm_now = localtime(&now);
      sprintf(ddd,"%04d-%02d-%02d",tm_now->tm_year+1900,tm_now->tm_mon+1,tm_now->tm_mday);
      sprintf(tod,"%02d:%02d:%02d",tm_now->tm_hour,tm_now->tm_min,tm_now->tm_sec);

      // Raw output to logfile
      if (fp) {
	    fprintf(fp,"%s %s %s\n",tod,ddd,buf);
        fflush(fp); // バッファを書き出し
      }
      // 不要フィールド削除
      rep(dbuf,buf,"<?xml version=\"1.0\"?><UECS ver=\"1.00-E10\">","");
      rep(buf,dbuf,"</UECS>","");
      rep(dbuf,buf,"<DATA type=\"","");
      rep(buf,dbuf,"\" room=\"",",");
      rep(dbuf,buf,"\" region=\"",",");
      rep(buf,dbuf,"\" order=\"",",");
      rep(dbuf,buf,"\" priority=\"",",");
      rep(buf,dbuf,"</DATA><IP>",",");
      rep(dbuf,buf,"</IP>","");
      rep(buf,dbuf,"\">",",");
      strcpy(dbuf,buf);
      // 受信データの出力
      //      printf("%s\n",dbuf);
      //      WRadiationSum.mMC,1,1,1,29,6.4,192.168.120.65
      strncpy(cbuf,dbuf,255);
      strf[0] = strtok(cbuf,",");
      for(c=1;c<7;c++) {
	strf[c] = strtok(NULL,",");
      }

      // 動的に読み込んだ ramdisk_path を使用
      sprintf(ccmstatus_name,"%s/%s-%s-%s-%s-%s-%s.dat", ramdisk_path,
      	      strf[1],strf[2],strf[3],strf[4],strf[6],strf[0]);
      ccmfp = fopen(ccmstatus_name,"w");
      if(ccmfp) {
        fprintf(ccmfp,"%s\n",strf[5]);
        fclose(ccmfp);
      }

      sprintf(&sqlbuf[0],"('%s %s',%s,%s,%s,%s,%s,inet_aton('%s'),'%s')",
	     ddd,tod,strf[1],strf[2],strf[3],strf[4],strf[5],strf[6],strf[0]);
      db_insert(conn,sqlbuf);

      // 動的に読み込んだ semapd_path を使用
      sprintf(semaphore_name,"%s/%s.semap", semapd_path, strf[6]);
      unlink(semaphore_name);
    } else {
      if ( errno == EAGAIN ) continue;
      perror("recvfrom");
    }
  }
  // ソケットのクローズ
  close(sd);
  if(fp) fclose(fp);
  return 0;
}

void abrt_handler(int sig) {
  stopflag = 1;
}
