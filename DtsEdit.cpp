// DTSRepair.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "Help.h"
#include "EditMain.h"
#include "windows.h"

#define EXIST(s) (GetFileAttributes(s) != 0xFFFFFFFF)

int main(int argc, char* argv[])
{
	int ret = -1;
	T_EDIT_PARAM prm;

	ZeroMemory(&prm, sizeof(T_EDIT_PARAM));

	// コマンドラインチェック
	if (argc < 2){
		Help();
		return ret;
	}

	// 初期値
	prm.i_mode = MODE_OUT;
	prm.i_tcv = 2;
	prm.i_timescale = 0;
	prm.i_timerate = 0;
	prm.f_scaleFct = 0;
	prm.f_mlt = 4.0;
	prm.i_delayFrame = -1;
	prm.b_dc = TRUE;

	for(int i=1; i<argc; i++){
		char *name = argv[i];
		char *value = NULL;
		char *value2 = NULL;
		if((i+1)<argc)
			value = argv[i+1];

		if(0){}

#define OPT(x) else if (!_stricmp(x, name))

		// タイムコード
		OPT("-tc"){
			if(!value || prm.p_tcfile) goto _ERROR_ARG;
			prm.p_tcfile = new char[strlen(value) + 1];
			strcpy(prm.p_tcfile, value);
			prm.i_mode = MODE_IN;
			i++;		
		}

		// タイムコード
		OPT("-tv"){
			if(!value) goto _ERROR_ARG;
			prm.i_tcv = atoi(value);
			i++;
		}

		// タイムスケール
		OPT("-s"){
			if(!value) goto _ERROR_ARG;
			prm.i_timescale = atol(value);
			i++;
		}

		// 倍数
		OPT("-mlt"){
			if(!value) goto _ERROR_ARG;
			prm.f_mlt = atof(value);
			i++;
		}

		// 最小タイムレート
		OPT("-r"){
			if(!value) goto _ERROR_ARG;
			prm.i_timerate = atol(value);
			i++;
		}

		// 初期ディレイカット
		OPT("-no-dc"){
			prm.b_dc = FALSE;
		}

		// ディレイフレーム数
		OPT("-df"){
			if(!value) goto _ERROR_ARG;
			prm.i_delayFrame = atol(value);
			i++;
		}

		// 出力ファイル名
		OPT("-o"){
			if(!value || prm.p_outfile) goto _ERROR_ARG;
			prm.p_outfile = new char[strlen(value) + 1];
			strcpy(prm.p_outfile, value);
			i++;
		}

		// 入力ファイル
		else if(EXIST(name)){
			if(prm.p_infile) goto _ERROR_ARG;
			prm.p_infile = new char[strlen(name) + 1];
			strcpy(prm.p_infile, name);
		}

		else {
_ERROR_ARG:
			Help();
			goto _ERROR_RET;
		}
	}

	// チェック処理
	if(prm.i_mode == MODE_IN && !EXIST(prm.p_tcfile)){
		Help();
		goto _ERROR_RET;
	}

	if(!prm.p_outfile){
		prm.p_outfile = new char[strlen(prm.p_infile) + 20];
		strcpy(prm.p_outfile, prm.p_infile);

		if(prm.i_mode == MODE_IN)
			strcat(prm.p_outfile, "_tc_input.mp4");
		if(prm.i_mode == MODE_OUT)
			strcat(prm.p_outfile, "_timecode.txt");
	}

	// メイン処理
	EditMain(&prm);
	ret = 0;

_ERROR_RET:
	if(prm.p_tcfile)	delete [] prm.p_tcfile;
	if(prm.p_outfile)	delete [] prm.p_outfile;
	if(prm.p_infile)	delete [] prm.p_infile;
	return ret;
}

