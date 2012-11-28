#include <stdio.h>
#include "stdafx.h"
#include "Help.h"
#include "EditMain.h"
#include "debug.h"


void EditMain(T_EDIT_PARAM *prm){

	GF_Err		e;
	T_MP4_INFO	mi;
	T_TS_LIST	*ts		= NULL;
	T_TS_LIST	*tsSort	= NULL;

	// 初期化
	gf_sys_init();

	ZeroMemory(&mi, sizeof(T_MP4_INFO));

	// ターゲットファイルのオープン
	mi.fp_in = gf_isom_open(prm->p_infile, GF_ISOM_OPEN_READ, NULL);
	printf("入力ファイル... [%s]\n\n", prm->p_infile);

	mi.i_trackCount = gf_isom_get_track_count(mi.fp_in);
	
	// 各トラックのフォーマットを取得
	u32 lst4CC[] = {GF_ISOM_BRAND_AVC1, GF_ISOM_BRAND_MP4V, GF_ISOM_BRAND_XVID};
	SearchTrackOf4CC(&mi, lst4CC, 3);

	// 目的のトラックが見つからなかった場合。
	if (mi.i_trackNo == NULL){
		ErrorMessage(MSG_NO_4CC);
		goto _ERROR_RET;
	}

	// サンプル数の取得
	mi.ui_sampleCount = gf_isom_get_sample_count(mi.fp_in, mi.i_trackNo);
	if(mi.ui_sampleCount < 3){
		ErrorMessage(MSG_NO_SAMPLE);
		goto _ERROR_RET;
	}

	ts = new T_TS_LIST[mi.ui_sampleCount + 1];
	ZeroMemory(ts, sizeof(T_TS_LIST) * mi.ui_sampleCount);

	// サンプル情報を取得し、ディレイフレーム判定・CTS順に並び替え
	printf("サンプル情報取得中...\n");
	if(readTsFromFile(&mi, ts))
		goto _ERROR_RET;

	// ピクチャ表示順でソート
	tsSort = new T_TS_LIST[mi.ui_sampleCount + 1];
	memcpy(tsSort, ts, sizeof(T_TS_LIST) * mi.ui_sampleCount);
	qsort(tsSort, mi.ui_sampleCount, sizeof(T_TS_LIST), (int(*)(const void*, const void*))CompareTS_PTS);

	// 最終フレームのFPS補完
	{
	u32 i = mi.ui_sampleCount;
	tsSort[i].DTS			= ts[i-1].DTS + (ts[i-1].DTS - ts[i-2].DTS);
	tsSort[i].CTS_Offset	= 0;
	tsSort[i].CTS			= tsSort[i-1].CTS + (tsSort[i-1].CTS - tsSort[i-2].CTS);
	tsSort[i].PTS			= tsSort[i-1].PTS + (tsSort[i-1].PTS - tsSort[i-2].PTS);
	tsSort[i].samples		= i+1;

	// タイムスケールの取得
	mi.i_org_timescale = (int)gf_isom_get_media_timescale(mi.fp_in, mi.i_trackNo);

	// この辺でモード分岐
	if(prm->i_mode == MODE_IN){
		// タイムコード入力

		// 最小フレームレート取得
		mi.i_org_timerate = getMinimumPTSDiff(&mi, tsSort);
		if(prm->i_timerate <= 0) {
			int rate = gcd(mi.i_org_timescale, mi.i_org_timerate);
			prm->i_timerate = mi.i_org_timerate / rate;
		}

		// TimeCode V2 の取り込み
		if (prm->i_tcv == 1){
			if(readTimeCodeFromFileV1(&mi, tsSort, prm))
				goto _ERROR_RET;
		} else if(prm->i_tcv == 2){
			if(readTimeCodeFromFile(&mi, tsSort, prm))
				goto _ERROR_RET;
		} else {
			goto _ERROR_RET;
		}

		// 遅延フレーム数取得
		if(prm->i_delayFrame < 0) {
			prm->i_delayFrame = getDelayFlame(&mi, ts);
		}
		mi.i_delayFrame = prm->i_delayFrame;
		mi.i_initDelay = mi.i_delayFrame * (int)(getAveragePTSDiff(&mi, tsSort) / (double)prm->i_timerate + 0.5) * prm->i_timerate;

		// 結果をコピーしてピクチャ順にソート
		memcpy(ts, tsSort, sizeof(T_TS_LIST) * mi.ui_sampleCount);
		qsort(ts, mi.ui_sampleCount, sizeof(T_TS_LIST), (int(*)(const void*, const void*))CompareTS_Sample);


		// 情報の表示
		printf("\n");
		printf("--- input ---\n");
		printf("TimeScale       ：%d\n", mi.i_org_timescale);
		printf("TimeRate        ：%d\n", mi.i_org_timerate);
		printf("Sample Count    ：%d\n", mi.ui_sampleCount);
		printf("Delay Frame     ：%d\n", mi.i_delayFrame);
		printf("Delay Time      ：%d\n", mi.i_initDelay);
		printf("\n");
		printf("--- output ---\n");
		printf("TimeScale       ：%d\n", prm->i_timescale);
		printf("TimeRate        ：%d\n", prm->i_timerate);
		printf("Multiple        ：%lf\n", prm->f_scaleFct);
		printf("\n");
		printf("ファイル出力中... [%s]\n", prm->p_outfile);

		u32 readDscIdx = 0;
		u32 destTrack = 0;
		mi.fp_out = gf_isom_open(prm->p_outfile, GF_ISOM_OPEN_WRITE, NULL);


		// 出力開始
		for (int trackIdx=1; trackIdx<=mi.i_trackCount; trackIdx++){
			// トラックのコピー
			gf_isom_clone_track(mi.fp_in, trackIdx, mi.fp_out, true, &destTrack);
			// edtsの削除
			gf_isom_remove_edit_segments(mi.fp_out, destTrack);

			if (trackIdx != mi.i_trackNo){
				u32 sampleCount = gf_isom_get_sample_count(mi.fp_in, trackIdx);
				for(u32 i=1; i<=sampleCount; i++){
					printf("... %5.1f%%\r", (double)i/(double)sampleCount * 100.0);
					GF_ISOSample *sample = gf_isom_get_sample(mi.fp_in, trackIdx, i, &readDscIdx);
					e = gf_isom_add_sample(mi.fp_out, destTrack, readDscIdx, sample);
					gf_isom_sample_del(&sample);
				}

			} else {
				// タイムスケール設定
				e = gf_isom_set_media_timescale(mi.fp_out, destTrack, (u32)prm->i_timescale);
				if(e)
					printf("!!! タイムスケール設定失敗 : %d !!!\n", e);

				// DTSを設定し出力
				s64 before_dts = -1;
				int delta_time = int((prm->i_timerate * prm->f_scaleFct) / (mi.i_delayFrame*2));

				if(delta_time < 1 && prm->b_dc){
					ErrorMessage(MSG_MORE_SMALL_TIMERATE);
					break;
				}

				for(u32 i=0; i<mi.ui_sampleCount; i++){
					printf("... %5.1f%%\r", (double)i/(double)mi.ui_sampleCount * 100.0);

					s64 cts = 0;
					s64 dts = 0;
					int offset = 0;
					int dts_delay = 0;
					int cts_delay = 0;
					int ts_diff = 0;

					cts = ts[i].PTS;
					if(!prm->b_dc){
						dts_delay = 0;
						cts_delay = mi.i_initDelay;
					} else {
						for(u32 k=MAX(0, i - mi.i_delayFrame); k<i; k++)
							ts_diff += (int)(tsSort[k + 1].PTS - tsSort[k].PTS);

						dts_delay = MAX(mi.i_initDelay, ts_diff);
						cts_delay = 0;
					}

					if(dts_delay < tsSort[i].PTS)
						dts = tsSort[i].PTS - dts_delay;

					// 前回のDTSよりも値は大きくなければならない。
					if(dts <= before_dts)
						dts = before_dts + delta_time;

					// CTS_Offsetの算出
					offset = (int)(cts - dts + cts_delay);

					// オフセットの計算で負になってしまった場合
					if (offset < 0){
						dts += offset;
						offset = 0;
					}

					ts[i].DTS = before_dts = dts;
					ts[i].CTS_Offset = offset;

					// チェック
					if( (dts + (s64)offset - (s64)ts[0].CTS_Offset) != cts )
						printf("!!! 表示タイミングエラー / Track:%d Frame:%d Target PTS:%I64d (DTS:%I64d, CTS_Offset:%d, Delay:%d) !!!\n", destTrack, i+1, cts, dts, offset, ts[0].CTS_Offset);

					GF_ISOSample *sample = gf_isom_get_sample(mi.fp_in, mi.i_trackNo, i+1, &readDscIdx);
					sample->DTS = ts[i].DTS;
					sample->CTS_Offset = ts[i].CTS_Offset;
					e = gf_isom_add_sample(mi.fp_out, destTrack, readDscIdx, sample);

					if(e)
						printf("!!! サンプル書き込みエラー / Track:%d Frame:%d DTS:%I64d, CTS_Offset:%d Err:%d !!!\n", destTrack, i+1, sample->DTS, sample->CTS_Offset, e);
					gf_isom_sample_del(&sample);
				}

				// edtsの挿入
				//GF_ISOSample *sample = gf_isom_get_sample_info(mi.fp_out, destTrack, 1, NULL, NULL);
				//if(sample->CTS_Offset > 0){
				//	u64 trackDur = gf_isom_get_track_duration(mi.fp_out, destTrack);
				//	gf_isom_remove_edit_segments(mi.fp_out, destTrack);
				//	gf_isom_append_edit_segment(mi.fp_out, destTrack, trackDur, sample->CTS_Offset, GF_ISOM_EDIT_NORMAL);
				//}
				//gf_isom_sample_del(&sample);
			}

			printf("... %dトラック出力終了\n", trackIdx);
		}

		// 出力ファイルを閉じる
		gf_isom_clone_pl_indications(mi.fp_in, mi.fp_out);
		gf_isom_clone_root_od(mi.fp_in, mi.fp_out);

		u64 duration = gf_isom_get_duration(mi.fp_out);
		gf_isom_make_interleave(mi.fp_out, 0.5);
		gf_isom_close(mi.fp_out);

	}
	else if (prm->i_mode == MODE_OUT){
		// TIMECODEの出力
		if(prm->i_tcv == 2){
			FILE *fpout;
			if((fpout = fopen(prm->p_outfile, "wb")) != NULL){
				fprintf(fpout, "# timecode format v2\r\n");
				for(u32 i=0; i<mi.ui_sampleCount; i++){
					fprintf(fpout, "%.6lf\r\n", (double)tsSort[i].PTS / (double)mi.i_org_timescale * 1000.0);
				}
			}
			fclose(fpout);
			printf("... timecode format v2 出力終了\n");
		}
		else if(prm->i_tcv == 1){
			FILE *fpout;
			if((fpout = fopen(prm->p_outfile, "wb")) != NULL){
				fprintf(fpout, "# timecode format v1\r\n");
				fprintf(fpout, "Assume %.6lf\r\n", getMaximumFps(&mi, tsSort));
				u32 stpos = 0;
				double fps = 0.0;
				double beforFps = (double)mi.i_org_timescale/(double)(tsSort[1].PTS - tsSort[0].PTS);
				for(u32 i=0; i<mi.ui_sampleCount; i++){
					fps = (double)mi.i_org_timescale/(double)(tsSort[i+1].PTS - tsSort[i].PTS);
					if (fps != beforFps){
						fprintf(fpout, "%d,%d,%.6lf\r\n",stpos, i-1, beforFps);
						beforFps = fps;
						stpos = i;
					}
				}
				if(stpos <= mi.ui_sampleCount - 1){
					fprintf(fpout, "%d,%d,%.6lf\r\n",stpos, mi.ui_sampleCount - 1, fps);
				}
			}
			fclose(fpout);
			printf("... timecode format v1 出力終了\n");
		} else {
			goto _ERROR_RET;
		}
	}
	}


_ERROR_RET:

	// ファイルを閉じて終了
	gf_isom_close(mi.fp_in);

	if(ts)		delete [] ts;
	if(tsSort)	delete [] tsSort;
	// 最終
	gf_sys_close();
}

