C言語のコンパイラ（未完成）です。
## header.h
ヘッダーファイルです。
## main.c
main関数を記述しています。
## reader.c
入力ファイルを読み込みます。
## tokenizer.c
入力をトークンの列に分解します。
## parser.c
トークンの列から構文木を構築します。
## generator.c
構文木上をDFSしてアセンブリを出力します。
## test.sh
inフォルダ内のテキストファイルを1つずつ入力に渡し、outフォルダ内の想定解と比較します。