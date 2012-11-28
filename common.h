
#ifndef _COMMON_H_

// common.h

#define _COMMON_H_

#define GF_ISOM_BRAND_MP4V GF_4CC('m','p','4','v')
#define GF_ISOM_BRAND_XVID GF_4CC('x','v','i','d')

typedef struct {
	char		*p_infile;
	char		*p_outfile;
	char		*p_tcfile;
	int			i_mode;
	int			i_tcv;
	BOOL		b_dc;
	int			i_timescale;
	int			i_timerate;
	int			i_delayFrame;
	double		f_scaleFct;
	double		f_mlt;
} T_EDIT_PARAM;

typedef struct {
	GF_ISOFile	*fp_in;
	GF_ISOFile	*fp_out;
	int			i_trackCount;
	int			i_trackNo;
	u32			ui_4cc;
	u32			ui_sampleCount;
	int			i_delayFrame;
	int			i_initDelay;
	int			i_org_timescale;
	int			i_org_timerate;
} T_MP4_INFO;

typedef struct {
	u64			DTS;
	u32			CTS_Offset;
	u64			CTS;
	u64			PTS;	// delayを含まないピクチャ表示タイミング
	u32			samples;
} T_TS_LIST;

int gcd( int a, int b);
int CompareU64(const u64 *a, const u64 *b);
int CompareTS_PTS(const T_TS_LIST *a, const T_TS_LIST *b);
int CompareTS_Sample(const T_TS_LIST *a, const T_TS_LIST *b);

int SearchTrackOf4CC(T_MP4_INFO *mi, u32 *List4CC, int ListCount);
int readTsFromFile(T_MP4_INFO *mi, T_TS_LIST *ts);
int getDelayFlame(T_MP4_INFO *mi, T_TS_LIST *ts);
int getMinimumPTSDiff(T_MP4_INFO *mi, T_TS_LIST *ts);
int getMaximumPTSDiff(T_MP4_INFO *mi, T_TS_LIST *ts);
double getAveragePTSDiff(T_MP4_INFO *mi, T_TS_LIST *ts);
double getAverageFps(T_MP4_INFO *mi, T_TS_LIST *ts);
double getMaximumFps(T_MP4_INFO *mi, T_TS_LIST *ts);

int readTimeCodeFromFile(T_MP4_INFO *mi, T_TS_LIST *tc, T_EDIT_PARAM *prm);
int readTimeCodeFromFileV1(T_MP4_INFO *mi, T_TS_LIST *tc, T_EDIT_PARAM *prm);



#endif //_COMMON_H_


