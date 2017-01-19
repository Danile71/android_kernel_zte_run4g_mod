#ifndef _CAMERA_CUSTOM_FLICKER_PARA_H_
#define _CAMERA_CUSTOM_FLICKER_PARA_H_

enum
{
  e_sensorModePreview=0,
  e_sensorModeVideoPreview,
  e_sensorModeVideo=e_sensorModeVideoPreview,
  e_sensorModeCapture,
  e_sensorModeZsd=e_sensorModeCapture,
  e_sensorModeVideo1,
  e_sensorModeVideo2,
  e_sensorModeCustom1,
  e_sensorModeCustom2,
  e_sensorModeCustom3,
  e_sensorModeCustom4,
  e_sensorModeCustom5,

};

typedef struct {
	MINT32	m;
	MINT32	b_l;
	MINT32	b_r;
	MINT32	offset;
} FLICKER_CUST_STATISTICS;

typedef struct
{
MINT32 flickerFreq[9];
MINT32 flickerGradThreshold;
MINT32 flickerSearchRange;
MINT32 minPastFrames;
MINT32 maxPastFrames;
FLICKER_CUST_STATISTICS EV50_L50;
FLICKER_CUST_STATISTICS EV50_L60;
FLICKER_CUST_STATISTICS EV60_L50;
FLICKER_CUST_STATISTICS EV60_L60;
MINT32 EV50_thresholds[2];
MINT32 EV60_thresholds[2];
MINT32 freq_feature_index[2];

}FLICKER_CUST_PARA;

#endif // #ifndef _CAMERA_CUSTOM_FLICKER_PARA_H_


