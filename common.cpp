#include "stdafx.h"
#include "Help.h"
#include "debug.h"

// 最大公約数の取得
int gcd( int a, int b) {
    while (1) {
        int c = a % b;
        if( !c )
            return b;
        a = b;
        b = c;
    }
}

int CompareU64(const u64 *a, const u64 *b)
{
	s64 diff = (s64)(*a - *b);
	return (diff > 0 ? 1: diff == 0 ? 0: -1);
}

int CompareTS_PTS(const T_TS_LIST *a, const T_TS_LIST *b)
{
	s64 diff = (s64)(a->PTS - b->PTS);
	return (diff > 0 ? 1: diff == 0 ? 0: -1);
}

int CompareTS_Sample(const T_TS_LIST *a, const T_TS_LIST *b)
{
	int diff = (int)(a->samples - b->samples);
	return (diff > 0 ? 1: diff == 0 ? 0: -1);
}


int SearchTrackOf4CC(T_MP4_INFO *mi, u32 *List4CC, int ListCount){
	u32	tgTrack = NULL;
	u32 tg4CC = NULL;
	for (int i = 1; i <= mi->i_trackCount; i++){
		tg4CC = gf_isom_get_media_subtype(mi->fp_in, i, 1);
		if (tg4CC == GF_ISOM_SUBTYPE_MPEG4)
			tg4CC = gf_isom_get_mpeg4_subtype(mi->fp_in, i, 1);

		DPRINTF("FOUR CC %s \r\n", gf_4cc_to_str(tg4CC));

		for (s32 k=0; k<ListCount; k++){
			if (tg4CC == List4CC[k]){
				tgTrack = i;
				break;
			}
		}

		if (tgTrack){
			mi->i_trackNo = tgTrack;
			mi->ui_4cc = tg4CC;
			break;
		}
	}

	return 0;
}


int readTsFromFile(T_MP4_INFO *mi, T_TS_LIST *ts){
	int ret = 0;

	for(u32 i=0; i<mi->ui_sampleCount; i++){
		GF_ISOSample *sample = gf_isom_get_sample_info(mi->fp_in, mi->i_trackNo, i+1, NULL, NULL);

		if(!sample){
			ret = ErrorMessage(MSG_NO_SAMPLE);
			break;
		}

		ts[i].DTS			= sample->DTS;
		ts[i].CTS_Offset	= sample->CTS_Offset;
		ts[i].CTS			= sample->DTS + sample->CTS_Offset;
		ts[i].PTS			= ts[i].CTS - ts[0].CTS_Offset;
		ts[i].samples		= i+1;
		gf_isom_sample_del(&sample);
	}

	return ret;
}


int getDelayFlame(T_MP4_INFO *mi, T_TS_LIST *ts){
	int delay = 0;
	int maxDelay = 0;
	for(u32 i=1; i<mi->ui_sampleCount; i++){
		if(ts[i-1].CTS > ts[i].CTS){
			delay++;
			maxDelay = MAX(maxDelay, delay);
		} else {
			delay = 0;
		}
	}

	return maxDelay;
}


int getMinimumPTSDiff(T_MP4_INFO *mi, T_TS_LIST *ts){
	int min_diff = INT_MAX;
	for(u32 i=1; i<mi->ui_sampleCount; i++){
		int diff = (int)(ts[i].PTS - ts[i-1].PTS);
		min_diff = MIN(min_diff, diff);
	}
	return min_diff;
}

int getMaximumPTSDiff(T_MP4_INFO *mi, T_TS_LIST *ts){
	int max_diff = 0;
	for(u32 i=1; i<mi->ui_sampleCount; i++){
		int diff = (int)(ts[i].PTS - ts[i-1].PTS);
		max_diff = MAX(max_diff, diff);
	}
	return max_diff;
}

double getAveragePTSDiff(T_MP4_INFO *mi, T_TS_LIST *ts){
	double avr_diff = 0.0;
	for(u32 i=1; i<mi->ui_sampleCount; i++){
		double diff = (double)(ts[i].PTS - ts[i-1].PTS);
		avr_diff += diff / (double)(mi->ui_sampleCount-1);
	}
	return avr_diff;
}

double getAverageFps(T_MP4_INFO *mi, T_TS_LIST *ts){
	double base_fps = (double)mi->i_org_timescale / (double)(ts[1].PTS - ts[0].PTS);
	double sum_fps = 0;
	for(u32 i=1; i<mi->ui_sampleCount; i++){
		sum_fps += ((double)mi->i_org_timescale / (double)(ts[i].PTS - ts[i-1].PTS)) - base_fps;
	}
	return ((sum_fps / (double)mi->ui_sampleCount) + base_fps);
}

double getMaximumFps(T_MP4_INFO *mi, T_TS_LIST *ts){
	double max_fps = 0;
	for(u32 i=1; i<mi->ui_sampleCount; i++){
		double fps = (double)mi->i_org_timescale / (double)(ts[i].PTS - ts[i-1].PTS);
		max_fps = MAX(max_fps, fps);
	}
	return max_fps;
}



int readTimeCodeFromFile(T_MP4_INFO *mi, T_TS_LIST *ts, T_EDIT_PARAM *prm){
	FILE *fpin;
	int err = 0;

	if((fpin = fopen(prm->p_tcfile, "rb")) != NULL){
		printf("timecode取込中...\n");

		char buf1[256], buf2[256], buf3[256], buf4[256];

		/* read header */
		fscanf(fpin, "%s %s %s %s", buf1, buf2, buf3, buf4);
		if(	_stricmp(buf2, "timecode") != 0 ||
			_stricmp(buf3, "format") != 0 ||
			_stricmp(buf4, "v2") != 0
		){
			err = ErrorMessage(MSG_NO_V2);
		} else {
			u32		i			= 0;
			double	timeStamp	= 0;
			bool	enough		= false;
			double	*timeList	= new double[mi->ui_sampleCount];

			// ファイルを読み込み
			while( fscanf(fpin, "%lf", &timeStamp) != EOF ){
				timeList[i] = timeStamp;
				i++;
				if (mi->ui_sampleCount <= i){
					enough = true;
					break;
				}
			}

			if (!enough) {
				err = ErrorMessage(MSG_NO_ENOUGH_FRAME);
			} else {

				double min_timeStamp = INT_MAX;
				// 最小タイミングを取得
				for(i=0; i<mi->ui_sampleCount - 1; i++){
					double tmp = timeList[i+1] - timeList[i];
					if(tmp > 0.0){
						int true_fps = (int)((double)prm->i_timerate / tmp + 0.5);
						if( true_fps > 0)
							tmp = (double)prm->i_timerate / (double)(true_fps);
					}
					min_timeStamp = MIN(min_timeStamp, tmp);
				}
				// エラー回避
				double max_fps = 1000.0 / MAX(min_timeStamp, 1.0);
				int timescale = int(max_fps * (double)prm->i_timerate * prm->f_mlt + 0.5);

				// timescale未指定の場合
				if(prm->i_timescale <= 0){
					prm->i_timescale = timescale;
					prm->f_scaleFct = prm->f_mlt;
				} else {
					prm->f_scaleFct = ((double)prm->i_timescale / (double)timescale) * prm->f_mlt;
				}

				double timerate = prm->i_timerate;
				if (prm->f_scaleFct < 1.0)
					timerate = timerate * prm->f_scaleFct;

				for(i=0; i<mi->ui_sampleCount; i++){
					ts[i].PTS = u64( u64((double)prm->i_timescale * (timeList[i] / 1000.0) / timerate + 0.5) * timerate + 0.5);
					ts[i].CTS = ts[i].PTS;
				}
			}

			delete [] timeList;
		}
	}
	fclose(fpin);
	return err;
}

int readTimeCodeFromFileV1(T_MP4_INFO *mi, T_TS_LIST *ts, T_EDIT_PARAM *prm){
	FILE *fpin;
	int err = 0;

	if((fpin = fopen(prm->p_tcfile, "rb")) != NULL){
		printf("timecode取込中...\n");

		char buf1[256], buf2[256], buf3[256], buf4[256];

		/* read header */
		fscanf(fpin, "%s %s %s %s", buf1, buf2, buf3, buf4);
		if(	_stricmp(buf2, "timecode") != 0 ||
			_stricmp(buf3, "format") != 0 ||
			_stricmp(buf4, "v1") != 0
		){
			err = ErrorMessage(MSG_NO_V1);
		} else {
			double AssumeFps;
			fscanf(fpin, "%s %lf",buf1, &AssumeFps);
			if(	_stricmp(buf1, "Assume") != 0)
				err = ErrorMessage(MSG_NO_V1);
			else {
				double	fps;
				u32		st;
				u32		ed;
				s64		beforeCount = 0;
				bool	enough		= false;
				double	*fpsList	= new double[mi->ui_sampleCount];


				while( fscanf(fpin, "%d,%d,%lf", &st, &ed, &fps) != EOF ){
					
					for(u32 i=st+1; i<=ed+1; i++){
						if ((beforeCount + 1) != i){
							err = ErrorMessage(MSG_LOST_FRAME);
							break;
						}
						beforeCount = i;
						fpsList[i] = fps;

						if (mi->ui_sampleCount <= i+1)
							break;
					}
					if (mi->ui_sampleCount <= ed+1){
						enough = true;
						break;
					}
				}
				if (!enough) {
					err = ErrorMessage(MSG_NO_ENOUGH_FRAME);
				} else {

					double max_fps = 0.1;
					// 最大fpsを取得
					for(u32 i=0; i<mi->ui_sampleCount; i++){
						max_fps = MAX(max_fps, fpsList[i]);
					}
					int timescale = int(max_fps * (double)prm->i_timerate * prm->f_mlt + 0.5);

					// timescale未指定の場合
					if(prm->i_timescale <= 0){
						prm->i_timescale = timescale;
						prm->f_scaleFct = prm->f_mlt;
					} else {
						prm->f_scaleFct = ((double)prm->i_timescale / (double)timescale) * prm->f_mlt;
					}

					double timerate = prm->i_timerate;
					if (prm->f_scaleFct < 1.0)
						timerate = timerate * prm->f_scaleFct;

					// 先頭は0固定
					ts[0].PTS = 0;
					ts[0].CTS = 0;
					for(u32 i=1; i<mi->ui_sampleCount; i++){
						ts[i].PTS = ts[i-1].PTS + u64( u64(((double)prm->i_timescale / (fpsList[i] * timerate)) + 0.5) * timerate + 0.5);
						ts[i].CTS = ts[i].PTS;
					}
				}
			}
		}
	}
	fclose(fpin);
	return err;
}


