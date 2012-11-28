
#pragma once

#define MSG_NO_AVC1					0x00000001
#define MSG_NO_V2					0x00000002
#define MSG_NO_4CC					0x00000003
#define MSG_NO_V1					0x00000004
#define MSG_MORE_SMALL_DIV_PTS		0x00000005
#define MSG_LOST_FRAME				0x00000006
#define MSG_NO_ENOUGH_FRAME			0x00000007
#define MSG_NO_SAMPLE				0x00000008
#define MSG_MORE_SMALL_TIMERATE		0x00000009

void Help();

int ErrorMessage(int msgId);

