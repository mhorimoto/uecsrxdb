# uecsrxdb
Server program for UECS data into MariaDB

* v1.10: 死活監視機能のためのセマフォファイル機能を付加
  	 受電されたIPアドレスのセマフォファイルを削除する
	 削除されていなければ受電が無いことを意味します。
* v1.00: 1st release 2023/10/25

## セマフォファイル

受電すると、SEMAPD/[IP Address].semap ファイルをunlink()する。
監視プログラムは当該するファイルが消えていることを確認する。
消えていなければ、受電出来ていないわけなので、異常と判断する。
消えていれば、touch SEMAPD/[IP Address].semap する。