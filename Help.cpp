#include <stdio.h>
#include <string.h>

#include <Help.h>

// ヘルプの表示
void Help(){
	printf("パラメータが不正です。\n");
	printf("Usage: DtsEdit <option> <mp4 file>\n");
	printf("\n");
	printf("option list\n");
	printf("	-tc <file name>       : 入力するTimeCodeファイルを指定します。\n");
	printf("	                        -tcが指定された場合は、TimeCodeを指定のMP4ファイルに埋め込みます。\n");
	printf("	                        指定されなかった場合は、MP4ファイルからTimeCodeを抽出します。\n");
	printf("	-tv <1|2>             : 入出力するTimeCodeファイルのバージョンを指定します。デフォルトは2です。\n");
	printf("	-s <time scale>       : timeScaleを指定します。\n");
	printf("	                        未指定の場合は、入力されたtimecode formatによって自動計算されます。\n");
	printf("	-mlt <multiple>       : timeScaleの自動計算に使用します。\n");
	printf("	                        デフォルト値は4.0倍です。timeScaleを直接指定した場合は、内部の計算結果で上書きされます。\n");
	printf("	-r <time rate>        : 最小timeRateを指定します。\n");
	printf("	                        未指定の場合は、入力されたMP4ファイルに従います。\n");
	printf("	-no-dc                : 初期ディレイカットを無効にします。\n");
	printf("	-df <count>           : 初期ディレイを付与する場合の、ディレイフレーム数を指定します。\n");
	printf("	                        未指定の場合は、入力されたMP4ファイルから自動計算します。\n");
	printf("	                        再生時間の短い動画では正確に取得できない場合があります。\n");
	printf("	-o <output file>      : 出力ファイルを指定します。\n");
	printf("\n");
}

int ErrorMessage(int msgId){
	char msg[1024];

	switch(msgId)
	{
		case MSG_NO_4CC:
			strcpy(msg, "未対応のビデオ形式です。\n");
			strcat(msg, "対応するFOURCC：avc1, mp4v, xvid");
			break;
		case MSG_NO_V2:
			strcpy(msg, "指定されたファイルは timecode format v2 ではありません。");
			break;
		case MSG_NO_V1:
			strcpy(msg, "指定されたファイルは timecode format v1 ではありません。");
			break;
		case MSG_MORE_SMALL_DIV_PTS:
			strcpy(msg, "ピクチャの表示間隔が小さすぎます。\n");
			strcat(msg, "timecodeを入力している場合は、内容を確認してください。");
			break;
		case MSG_LOST_FRAME:
			strcpy(msg, "フレームが連続していないため、TimeCode入力を停止しました。");
			break;
		case MSG_NO_ENOUGH_FRAME:
			strcpy(msg, "フレームが足りないため、TimeCode入力を停止しました。");
			break;
		case MSG_NO_SAMPLE:
			strcpy(msg, "サンプルが取得できません。最低でも2フレームの動画を入力してください。");
			break;
		case MSG_MORE_SMALL_TIMERATE:
			strcpy(msg, "TimeScaleの分解能に対してディレイフレームが多すぎるため、初期ディレイカットが出来ません。");
			break;
		default:
			sprintf(msg, "未定義メッセージ：0x%08X", msgId);
			break;
	}
	
	printf("\n%s\n\n", msg);
	return 1;
}
