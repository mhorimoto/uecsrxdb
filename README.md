# uecsrxdb
Server program for UECS data into MariaDB and High Speed RRD

## このプログラムの目的

受信できた全てのUECSパケットをデータベース(MariaDB)に保存する事を目的としています。

## 必要条件

* OSとしてLinux
  systemctlで起動することを前提としています。
* プログラムをコンパイルすためにGCC
* DBとしてMariaDB

## プログラム作成方法

 1. db.cのDatabaseのユーザ名やDB名を設定します。
    db.cの中の\_\_USER\_\_や\_\_PASS\_\_、\_\_DBNAME\_\_ の部分を既存のDBに合わせて編集します。  
     
        MYSQL *db_init(void) {
        	MYSQL *conn     = NULL;
            MYSQL_ROW row;
            char *sql_serv  = "localhost";
            char *user      = "__USER__";
            char *passwd    = "__PASS__";
            char *db_name   = "__DBNAME__";
            int  i;
    
2. コンパイルは、make コマンド一発で出来るはずです。 make install
    でプログラムは、/usr/local/bin/uecsrxdb としてinstallされます。
3. uecsrxdb.serviceファイルはOSに環境に合わせてコピーして自動実行する設定などを施してください。例えば、/etc/systemd/system の下にコピーするなど。
4. DBにテーブルを生成するためには mariadb-sql.txt に従ってください。
5. v1.10以降では、死活監視機能のためのファイルを保存するディレクトリを作成します。
　　　　　`mkdir /var/log/uecs/semaphore`

## 実行方法
試験的に動かす場合には、/usr/local/bin/uecsrxdb を直接実行してください。  
エラーがあれば、コンソールに表示されます。エラーが出ないことを確認した後、systemctlで起動してください。
    

## セマフォファイル

v1.10から採用された、UECSデバイスの死活監視機能のためのファイルです。
受電すると、SEMAPD/[IP Address].semap ファイルをunlink()します。
監視プログラム(chkuecs)は当該するファイルが消えていることを確認します。
消えていなければ、受電出来ていないわけなので、異常と判断します。
消えていれば、touch SEMAPD/[IP Address].semap してファイルを生成します。

## 履歴

 * v3.00: RRDCACHE対応のrrdcached
 * v2.00: サーバ上にramdiskを作って即値ファイルを生成するようにした
 * v1.10: 死活監視機能のためのセマフォファイル機能を付加
         受電されたIPアドレスのセマフォファイルを削除します。
         削除されていなければ受電が無いことを意味します。
 *  v1.00: 1st release 2023/10/25

## 作者

堀本　正文 https://www.ys-lab.tech/ 

## LICENSE
 MITライセンスです。
