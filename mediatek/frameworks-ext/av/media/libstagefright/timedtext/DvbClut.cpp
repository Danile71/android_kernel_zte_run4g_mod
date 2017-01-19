/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 #ifdef MTK_SUBTITLE_SUPPORT
 /*-----------------------------------------------------------------------------
					 include files
  ----------------------------------------------------------------------------*/
#define LOG_TAG "DVBClut"

#include <stdlib.h>
#include <utils/Log.h>
	 
#include "DvbClut.h"
#include "DvbPage.h"
#include "DvbRegion.h"
#include "DvbObject.h"


#include "DvbClutMgr.h"
#include "DVbPageMgr.h"
#include "DvbRegionMgr.h"
#include "DvbObjectMgr.h"



 
 /*-----------------------------------------------------------------------------
					 macros, defines, typedefs, enums
  ----------------------------------------------------------------------------*/

 /*-----------------------------------------------------------------------------
					 data declarations, extern, static, const
  ----------------------------------------------------------------------------*/
 
 /* Color Transform Table */
 const INT32 ai4_Y_tbl[256] =
 {
		-18624,   -17460,	-16296,   -15132,	-13968,   -12804,	-11640,   -10476,
		 -9312,    -8148,	 -6984,    -5820,	 -4656,    -3492,	 -2328,    -1164,
			 0, 	1164,	  2328, 	3492,	  4656, 	5820,	  6984, 	8148,
		  9312,    10476,	 11640,    12804,	 13968,    15132,	 16296,    17460,
		 18624,    19788,	 20952,    22116,	 23280,    24444,	 25608,    26772,
		 27936,    29100,	 30264,    31428,	 32592,    33756,	 34920,    36084,
		 37248,    38412,	 39576,    40740,	 41904,    43068,	 44232,    45396,
		 46560,    47724,	 48888,    50052,	 51216,    52380,	 53544,    54708,
		 55872,    57036,	 58200,    59364,	 60528,    61692,	 62856,    64020,
		 65184,    66348,	 67512,    68676,	 69840,    71004,	 72168,    73332,
		 74496,    75660,	 76824,    77988,	 79152,    80316,	 81480,    82644,
		 83808,    84972,	 86136,    87300,	 88464,    89628,	 90792,    91956,
		 93120,    94284,	 95448,    96612,	 97776,    98940,	100104,   101268,
		102432,   103596,	104760,   105924,	107088,   108252,	109416,   110580,
		111744,   112908,	114072,   115236,	116400,   117564,	118728,   119892,
		121056,   122220,	123384,   124548,	125712,   126876,	128040,   129204,
		130368,   131532,	132696,   133860,	135024,   136188,	137352,   138516,
		139680,   140844,	142008,   143172,	144336,   145500,	146664,   147828,
		148992,   150156,	151320,   152484,	153648,   154812,	155976,   157140,
		158304,   159468,	160632,   161796,	162960,   164124,	165288,   166452,
		167616,   168780,	169944,   171108,	172272,   173436,	174600,   175764,
		176928,   178092,	179256,   180420,	181584,   182748,	183912,   185076,
		186240,   187404,	188568,   189732,	190896,   192060,	193224,   194388,
		195552,   196716,	197880,   199044,	200208,   201372,	202536,   203700,
		204864,   206028,	207192,   208356,	209520,   210684,	211848,   213012,
		214176,   215340,	216504,   217668,	218832,   219996,	221160,   222324,
		223488,   224652,	225816,   226980,	228144,   229308,	230472,   231636,
		232800,   233964,	235128,   236292,	237456,   238620,	239784,   240948,
		242112,   243276,	244440,   245604,	246768,   247932,	249096,   250260,
		251424,   252588,	253752,   254916,	256080,   257244,	258408,   259572,
		260736,   261900,	263064,   264228,	265392,   266556,	267720,   268884,
		270048,   271212,	272376,   273540,	274704,   275868,	277032,   278196,
 };
 
 const INT32 ai4_Cr_R_tbl[256] =
 {
	   -204288,  -202692,  -201096,  -199500,  -197904,  -196308,  -194712,  -193116,
	   -191520,  -189924,  -188328,  -186732,  -185136,  -183540,  -181944,  -180348,
	   -178752,  -177156,  -175560,  -173964,  -172368,  -170772,  -169176,  -167580,
	   -165984,  -164388,  -162792,  -161196,  -159600,  -158004,  -156408,  -154812,
	   -153216,  -151620,  -150024,  -148428,  -146832,  -145236,  -143640,  -142044,
	   -140448,  -138852,  -137256,  -135660,  -134064,  -132468,  -130872,  -129276,
	   -127680,  -126084,  -124488,  -122892,  -121296,  -119700,  -118104,  -116508,
	   -114912,  -113316,  -111720,  -110124,  -108528,  -106932,  -105336,  -103740,
	   -102144,  -100548,	-98952,   -97356,	-95760,   -94164,	-92568,   -90972,
		-89376,   -87780,	-86184,   -84588,	-82992,   -81396,	-79800,   -78204,
		-76608,   -75012,	-73416,   -71820,	-70224,   -68628,	-67032,   -65436,
		-63840,   -62244,	-60648,   -59052,	-57456,   -55860,	-54264,   -52668,
		-51072,   -49476,	-47880,   -46284,	-44688,   -43092,	-41496,   -39900,
		-38304,   -36708,	-35112,   -33516,	-31920,   -30324,	-28728,   -27132,
		-25536,   -23940,	-22344,   -20748,	-19152,   -17556,	-15960,   -14364,
		-12768,   -11172,	 -9576,    -7980,	 -6384,    -4788,	 -3192,    -1596,
			 0, 	1596,	  3192, 	4788,	  6384, 	7980,	  9576,    11172,
		 12768,    14364,	 15960,    17556,	 19152,    20748,	 22344,    23940,
		 25536,    27132,	 28728,    30324,	 31920,    33516,	 35112,    36708,
		 38304,    39900,	 41496,    43092,	 44688,    46284,	 47880,    49476,
		 51072,    52668,	 54264,    55860,	 57456,    59052,	 60648,    62244,
		 63840,    65436,	 67032,    68628,	 70224,    71820,	 73416,    75012,
		 76608,    78204,	 79800,    81396,	 82992,    84588,	 86184,    87780,
		 89376,    90972,	 92568,    94164,	 95760,    97356,	 98952,   100548,
		102144,   103740,	105336,   106932,	108528,   110124,	111720,   113316,
		114912,   116508,	118104,   119700,	121296,   122892,	124488,   126084,
		127680,   129276,	130872,   132468,	134064,   135660,	137256,   138852,
		140448,   142044,	143640,   145236,	146832,   148428,	150024,   151620,
		153216,   154812,	156408,   158004,	159600,   161196,	162792,   164388,
		165984,   167580,	169176,   170772,	172368,   173964,	175560,   177156,
		178752,   180348,	181944,   183540,	185136,   186732,	188328,   189924,
		191520,   193116,	194712,   196308,	197904,   199500,	201096,   202692,
 };
 
 const INT32 ai4_Cb_B_tbl[256] =
 {
	   -258304,  -256286,  -254268,  -252250,  -250232,  -248214,  -246196,  -244178,
	   -242160,  -240142,  -238124,  -236106,  -234088,  -232070,  -230052,  -228034,
	   -226016,  -223998,  -221980,  -219962,  -217944,  -215926,  -213908,  -211890,
	   -209872,  -207854,  -205836,  -203818,  -201800,  -199782,  -197764,  -195746,
	   -193728,  -191710,  -189692,  -187674,  -185656,  -183638,  -181620,  -179602,
	   -177584,  -175566,  -173548,  -171530,  -169512,  -167494,  -165476,  -163458,
	   -161440,  -159422,  -157404,  -155386,  -153368,  -151350,  -149332,  -147314,
	   -145296,  -143278,  -141260,  -139242,  -137224,  -135206,  -133188,  -131170,
	   -129152,  -127134,  -125116,  -123098,  -121080,  -119062,  -117044,  -115026,
	   -113008,  -110990,  -108972,  -106954,  -104936,  -102918,  -100900,   -98882,
		-96864,   -94846,	-92828,   -90810,	-88792,   -86774,	-84756,   -82738,
		-80720,   -78702,	-76684,   -74666,	-72648,   -70630,	-68612,   -66594,
		-64576,   -62558,	-60540,   -58522,	-56504,   -54486,	-52468,   -50450,
		-48432,   -46414,	-44396,   -42378,	-40360,   -38342,	-36324,   -34306,
		-32288,   -30270,	-28252,   -26234,	-24216,   -22198,	-20180,   -18162,
		-16144,   -14126,	-12108,   -10090,	 -8072,    -6054,	 -4036,    -2018,
			 0, 	2018,	  4036, 	6054,	  8072,    10090,	 12108,    14126,
		 16144,    18162,	 20180,    22198,	 24216,    26234,	 28252,    30270,
		 32288,    34306,	 36324,    38342,	 40360,    42378,	 44396,    46414,
		 48432,    50450,	 52468,    54486,	 56504,    58522,	 60540,    62558,
		 64576,    66594,	 68612,    70630,	 72648,    74666,	 76684,    78702,
		 80720,    82738,	 84756,    86774,	 88792,    90810,	 92828,    94846,
		 96864,    98882,	100900,   102918,	104936,   106954,	108972,   110990,
		113008,   115026,	117044,   119062,	121080,   123098,	125116,   127134,
		129152,   131170,	133188,   135206,	137224,   139242,	141260,   143278,
		145296,   147314,	149332,   151350,	153368,   155386,	157404,   159422,
		161440,   163458,	165476,   167494,	169512,   171530,	173548,   175566,
		177584,   179602,	181620,   183638,	185656,   187674,	189692,   191710,
		193728,   195746,	197764,   199782,	201800,   203818,	205836,   207854,
		209872,   211890,	213908,   215926,	217944,   219962,	221980,   223998,
		226016,   228034,	230052,   232070,	234088,   236106,	238124,   240142,
		242160,   244178,	246196,   248214,	250232,   252250,	254268,   256286,
 };
 
 const INT32 ai4_Cr_G_tbl[256] =
 {
	   -104064,  -103251,  -102438,  -101625,  -100812,   -99999,	-99186,   -98373,
		-97560,   -96747,	-95934,   -95121,	-94308,   -93495,	-92682,   -91869,
		-91056,   -90243,	-89430,   -88617,	-87804,   -86991,	-86178,   -85365,
		-84552,   -83739,	-82926,   -82113,	-81300,   -80487,	-79674,   -78861,
		-78048,   -77235,	-76422,   -75609,	-74796,   -73983,	-73170,   -72357,
		-71544,   -70731,	-69918,   -69105,	-68292,   -67479,	-66666,   -65853,
		-65040,   -64227,	-63414,   -62601,	-61788,   -60975,	-60162,   -59349,
		-58536,   -57723,	-56910,   -56097,	-55284,   -54471,	-53658,   -52845,
		-52032,   -51219,	-50406,   -49593,	-48780,   -47967,	-47154,   -46341,
		-45528,   -44715,	-43902,   -43089,	-42276,   -41463,	-40650,   -39837,
		-39024,   -38211,	-37398,   -36585,	-35772,   -34959,	-34146,   -33333,
		-32520,   -31707,	-30894,   -30081,	-29268,   -28455,	-27642,   -26829,
		-26016,   -25203,	-24390,   -23577,	-22764,   -21951,	-21138,   -20325,
		-19512,   -18699,	-17886,   -17073,	-16260,   -15447,	-14634,   -13821,
		-13008,   -12195,	-11382,   -10569,	 -9756,    -8943,	 -8130,    -7317,
		 -6504,    -5691,	 -4878,    -4065,	 -3252,    -2439,	 -1626, 	-813,
			 0, 	 813,	  1626, 	2439,	  3252, 	4065,	  4878, 	5691,
		  6504, 	7317,	  8130, 	8943,	  9756,    10569,	 11382,    12195,
		 13008,    13821,	 14634,    15447,	 16260,    17073,	 17886,    18699,
		 19512,    20325,	 21138,    21951,	 22764,    23577,	 24390,    25203,
		 26016,    26829,	 27642,    28455,	 29268,    30081,	 30894,    31707,
		 32520,    33333,	 34146,    34959,	 35772,    36585,	 37398,    38211,
		 39024,    39837,	 40650,    41463,	 42276,    43089,	 43902,    44715,
		 45528,    46341,	 47154,    47967,	 48780,    49593,	 50406,    51219,
		 52032,    52845,	 53658,    54471,	 55284,    56097,	 56910,    57723,
		 58536,    59349,	 60162,    60975,	 61788,    62601,	 63414,    64227,
		 65040,    65853,	 66666,    67479,	 68292,    69105,	 69918,    70731,
		 71544,    72357,	 73170,    73983,	 74796,    75609,	 76422,    77235,
		 78048,    78861,	 79674,    80487,	 81300,    82113,	 82926,    83739,
		 84552,    85365,	 86178,    86991,	 87804,    88617,	 89430,    90243,
		 91056,    91869,	 92682,    93495,	 94308,    95121,	 95934,    96747,
		 97560,    98373,	 99186,    99999,	100812,   101625,	102438,   103251,
 };
 
 const INT32 ai4_Cb_G_tbl[256] =
 {
		-50048,   -49657,	-49266,   -48875,	-48484,   -48093,	-47702,   -47311,
		-46920,   -46529,	-46138,   -45747,	-45356,   -44965,	-44574,   -44183,
		-43792,   -43401,	-43010,   -42619,	-42228,   -41837,	-41446,   -41055,
		-40664,   -40273,	-39882,   -39491,	-39100,   -38709,	-38318,   -37927,
		-37536,   -37145,	-36754,   -36363,	-35972,   -35581,	-35190,   -34799,
		-34408,   -34017,	-33626,   -33235,	-32844,   -32453,	-32062,   -31671,
		-31280,   -30889,	-30498,   -30107,	-29716,   -29325,	-28934,   -28543,
		-28152,   -27761,	-27370,   -26979,	-26588,   -26197,	-25806,   -25415,
		-25024,   -24633,	-24242,   -23851,	-23460,   -23069,	-22678,   -22287,
		-21896,   -21505,	-21114,   -20723,	-20332,   -19941,	-19550,   -19159,
		-18768,   -18377,	-17986,   -17595,	-17204,   -16813,	-16422,   -16031,
		-15640,   -15249,	-14858,   -14467,	-14076,   -13685,	-13294,   -12903,
		-12512,   -12121,	-11730,   -11339,	-10948,   -10557,	-10166,    -9775,
		 -9384,    -8993,	 -8602,    -8211,	 -7820,    -7429,	 -7038,    -6647,
		 -6256,    -5865,	 -5474,    -5083,	 -4692,    -4301,	 -3910,    -3519,
		 -3128,    -2737,	 -2346,    -1955,	 -1564,    -1173,	  -782, 	-391,
			 0, 	 391,	   782, 	1173,	  1564, 	1955,	  2346, 	2737,
		  3128, 	3519,	  3910, 	4301,	  4692, 	5083,	  5474, 	5865,
		  6256, 	6647,	  7038, 	7429,	  7820, 	8211,	  8602, 	8993,
		  9384, 	9775,	 10166,    10557,	 10948,    11339,	 11730,    12121,
		 12512,    12903,	 13294,    13685,	 14076,    14467,	 14858,    15249,
		 15640,    16031,	 16422,    16813,	 17204,    17595,	 17986,    18377,
		 18768,    19159,	 19550,    19941,	 20332,    20723,	 21114,    21505,
		 21896,    22287,	 22678,    23069,	 23460,    23851,	 24242,    24633,
		 25024,    25415,	 25806,    26197,	 26588,    26979,	 27370,    27761,
		 28152,    28543,	 28934,    29325,	 29716,    30107,	 30498,    30889,
		 31280,    31671,	 32062,    32453,	 32844,    33235,	 33626,    34017,
		 34408,    34799,	 35190,    35581,	 35972,    36363,	 36754,    37145,
		 37536,    37927,	 38318,    38709,	 39100,    39491,	 39882,    40273,
		 40664,    41055,	 41446,    41837,	 42228,    42619,	 43010,    43401,
		 43792,    44183,	 44574,    44965,	 45356,    45747,	 46138,    46529,
		 46920,    47311,	 47702,    48093,	 48484,    48875,	 49266,    49657,
 };
 
 
 /* Color LookUp Default Table */
 const GL_COLOR_T at_2bit_color_default_CLUT[DVB_SBTL_2BIT_CLUT_SIZE] = 
 {
	 { _PERCENT_000, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_050}, },
 };
 
 const GL_COLOR_T at_4bit_color_default_CLUT[DVB_SBTL_4BIT_CLUT_SIZE] = 
 {
	 { _PERCENT_000, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_050}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_050}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_000}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_050}, },
 };
 
 const GL_COLOR_T at_8bit_color_default_CLUT[DVB_SBTL_8BIT_CLUT_SIZE] = 
 {
	 { _PERCENT_000, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_066}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_083}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_083}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_050}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_000}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_050}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_083}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_083}, {_PERCENT_083}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_066}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_000}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_025, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_066}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_083}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_083}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_050}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_050}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_083}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_083}, {_PERCENT_100}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_000}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_066}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_050}, },
	 { _PERCENT_025, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_066}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_066}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_100}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_100}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_066}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_033}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_066}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_100}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_100}, {_PERCENT_083}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_016}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_016}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_050}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_050}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_016}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_033}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_016}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_050}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_050}, {_PERCENT_033}, },
	 { _PERCENT_025, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_033}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_083}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_016}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_033}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_016}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_050}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_000}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_000}, {_PERCENT_050}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_016}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_033}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_016}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_050}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_050}, {_PERCENT_050}, },
	 { _PERCENT_025, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_050}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_050}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_066}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_083}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_083}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_050}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_050}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_083}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_083}, {_PERCENT_083}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_066}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_000}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_025, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_050}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_050}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_066}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_083}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_083}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_050}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_050}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_083}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_083}, {_PERCENT_100}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_000}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_000}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_066}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_033}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_033}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_000}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_000}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_000}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_033}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_033}, {_PERCENT_050}, },
	 { _PERCENT_025, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_033}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_083}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_050}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_083}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_016}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_016}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_050}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_050}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_033}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_016}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_033}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_016}, {_PERCENT_033}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_000}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_000}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_033}, },
	 { _PERCENT_025, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_100}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_033}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_066}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_033}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_066}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_066}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_016}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_033}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_016}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_100}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_050}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_033}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_016}, {_PERCENT_050}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_033}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_016}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_033}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_016}, {_PERCENT_050}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_033}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_016}, },
	 { _PERCENT_050, {_PERCENT_100}, {_PERCENT_100}, {_PERCENT_100}, },
	 { _PERCENT_100, {_PERCENT_050}, {_PERCENT_050}, {_PERCENT_050}, },
 };
 
 const DVB_NBIT_CLUT_T t_2bit_default_CLUT =
 {
	 (GL_COLOR_T*) at_2bit_color_default_CLUT, 0
 };
 
 const DVB_NBIT_CLUT_T t_4bit_default_CLUT =
 {
	 (GL_COLOR_T*) at_4bit_color_default_CLUT, 0
 };
 
 const DVB_NBIT_CLUT_T t_8bit_default_CLUT =
 {
	 (GL_COLOR_T*) at_8bit_color_default_CLUT, 0
 };
 
 /*-----------------------------------------------------------------------------
					 functions declarations
  ----------------------------------------------------------------------------*/

 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 const DVB_NBIT_CLUT_T* DvbClut::getDefaultClut(UINT8 ui1_color_depth)
 {
	 switch (ui1_color_depth)
	 {
		 case DVB_SBTL_RGN_DEPTH_2BIT:
			 return (&t_2bit_default_CLUT);
 
		 case DVB_SBTL_RGN_DEPTH_4BIT:
			 return (&t_4bit_default_CLUT);
 
		 case DVB_SBTL_RGN_DEPTH_8BIT:
			 return (&t_8bit_default_CLUT);
 
		 default:
			 /* Should Abort here */
			 return (NULL);
	 }
 }
 
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
DvbClut::DvbClut()
{ 
	 this->pt_2bit_CLUT = (DVB_NBIT_CLUT_T*) &t_2bit_default_CLUT;
	 this->pt_4bit_CLUT = (DVB_NBIT_CLUT_T*) &t_4bit_default_CLUT;
	 this->pt_8bit_CLUT = (DVB_NBIT_CLUT_T*) &t_8bit_default_CLUT;
 	
 }
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 DvbClut::~DvbClut()
 {
	
 
	 if (NULL != this->pt_2bit_CLUT)
	 {
		 release(this->pt_2bit_CLUT,  DVB_SBTL_RGN_DEPTH_2BIT);
	 }
 
	 if (NULL != this->pt_4bit_CLUT)
	 {
		 release(this->pt_4bit_CLUT,  DVB_SBTL_RGN_DEPTH_4BIT);
	 }
 
	 if (NULL != this->pt_8bit_CLUT)
	 {
		 release(this->pt_8bit_CLUT, DVB_SBTL_RGN_DEPTH_8BIT);
	 }
	 

 }

 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 INT32 DvbClut::create(
	 UINT8* 			 pui1_data, 
	 UINT32 			 ui4_data_len, 
	 DvbClut**	 ppt_this)
 {
	 INT32					 i4_ret;
	 DvbClut*		 pt_this;
 
	 *ppt_this = NULL;
	 pt_this = new DvbClut();
	 
	 if (NULL == pt_this)
	 {
		 return (DVBR_INSUFFICIENT_MEMORY);
	 }
 
	 pt_this->pt_2bit_CLUT = (DVB_NBIT_CLUT_T*) &t_2bit_default_CLUT;
	 pt_this->pt_4bit_CLUT = (DVB_NBIT_CLUT_T*) &t_4bit_default_CLUT;
	 pt_this->pt_8bit_CLUT = (DVB_NBIT_CLUT_T*) &t_8bit_default_CLUT;
 
	 i4_ret = pt_this->parseSegment_impl(pui1_data, 
										  ui4_data_len,
										  TRUE);
 
	 if (DVBR_OK > i4_ret)
	 {
		 /* segment data error */
		 //DVB_sbtl_clut_delete(pt_this, pt_service);
		 delete pt_this;
		 return (i4_ret);
	 }
	 else
	 {
		 *ppt_this = pt_this;
		 return (DVBR_OK);
	 }
 }

 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 INT32 DvbClut::parseSegment(
	 UINT8* 			 pui1_data, 
	 UINT32 			 ui4_data_len)
 {
	 return parseSegment_impl(pui1_data,
							 ui4_data_len, 
							 FALSE);
 }
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 DVB_NBIT_CLUT_T* DvbClut::linkSub(
	 UINT8				 ui1_color_depth)
 {
	 DVB_NBIT_CLUT_T*	 pt_nbit_clut;
 
	 switch (ui1_color_depth)
	 {
		 case DVB_SBTL_RGN_DEPTH_2BIT:
			 pt_nbit_clut = this->pt_2bit_CLUT;
			 break;
 
		 case DVB_SBTL_RGN_DEPTH_4BIT:
			 pt_nbit_clut = this->pt_4bit_CLUT;
			 break;
 
		 case DVB_SBTL_RGN_DEPTH_8BIT:
			 pt_nbit_clut = this->pt_8bit_CLUT;
			 break;
 
		 default:
			 return (NULL);
	 }
 
	 if (0 != pt_nbit_clut->ui2_link_cnt)
	 {
		 pt_nbit_clut->ui2_link_cnt++;
	 }
 
	 return (pt_nbit_clut);
 }
 
 
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 INT32 DvbClut::getClutEntry(
	 UINT8* 			 pui1_data, 
	 UINT32 			 ui4_data_len,
	 UINT8				 ui1_n_bits_entry_flag,
	 UINT8				 ui1_full_range_flag,
	 UINT8				 ui1_entry_id,
	 UINT32*			 pui4_inc_size )
 {
	 UINT8					 ui1_Y, ui1_CR, ui1_CB, ui1_T;
	 INT32					 i4_R, i4_G, i4_B;
	 GL_COLOR_T*			 pt_gl_color;
	 INT32					 i4_ret = DVBR_OK;
	 DVB_NBIT_CLUT_T*	 pt_nbit_CLUT;
 
	 switch (ui1_n_bits_entry_flag)
	 {
		 case DVB_SBTL_RGN_DEPTH_2BIT:
			 if (ui1_entry_id > (UINT8)DVB_SBTL_2BIT_CLUT_SIZE)
			 {
				 return (DVBR_WRONG_SEGMENT_DATA);
			 }
 
			 /* is reference by decoded region or default table */
			 if (1 != this->pt_2bit_CLUT->ui2_link_cnt)
			 {
				 i4_ret = dup(DVB_SBTL_RGN_DEPTH_2BIT, 
											  this->pt_2bit_CLUT, 
											  &pt_nbit_CLUT);
 
				 if (DVBR_OK > i4_ret)
				 {
					 /* Insufficient memory */
					 ALOGV("[SBTL]Insufficient memory to create 2bit clut\n");
					 return (DVBR_INSUFFICIENT_MEMORY);
				 }
				 else
				 {
					 this->pt_2bit_CLUT = pt_nbit_CLUT;
				 }
			 }
 
			 pt_gl_color = &(this->pt_2bit_CLUT->pat_color_entry[ui1_entry_id]);
			 break;
 
		 case DVB_SBTL_RGN_DEPTH_4BIT:
			 if (ui1_entry_id > (UINT8)DVB_SBTL_4BIT_CLUT_SIZE)
			 {
				 return (DVBR_WRONG_SEGMENT_DATA);
			 }
			 
			 /* is reference by decoded region or default table */
			 if (1 != this->pt_4bit_CLUT->ui2_link_cnt)
			 {
				 i4_ret = dup(DVB_SBTL_RGN_DEPTH_4BIT, 
											  this->pt_4bit_CLUT, 
											  &pt_nbit_CLUT);
 
				 if (DVBR_OK > i4_ret)
				 {
					 /* Insufficient memory */
					 ALOGV("[SBTL]Insufficient memory to create 4bit clut\n");
					 return (DVBR_INSUFFICIENT_MEMORY);
				 }
				 else
				 {
					 this->pt_4bit_CLUT = pt_nbit_CLUT;
				 }
			 }
 
			 pt_gl_color = &(this->pt_4bit_CLUT->pat_color_entry[ui1_entry_id]);
			 break;
 
		 case DVB_SBTL_RGN_DEPTH_8BIT:
 
			 /* is reference by decoded region or default table */
			 if (1 != this->pt_8bit_CLUT->ui2_link_cnt)
			 {
				 i4_ret = dup(DVB_SBTL_RGN_DEPTH_8BIT, 
											  this->pt_8bit_CLUT, 
											  &pt_nbit_CLUT);
 
				 if (DVBR_OK > i4_ret)
				 {
					 /* Insufficient memory */
					 ALOGV("[SBTL]Insufficient memory to create 8bit clut\n");
					 return (DVBR_INSUFFICIENT_MEMORY);
				 }
				 else
				 {
					 this->pt_8bit_CLUT = pt_nbit_CLUT;
				 }
			 }
			 
			 pt_gl_color = &(this->pt_8bit_CLUT->pat_color_entry[ui1_entry_id]);
			 break;
 
		 default:
			 return (DVBR_WRONG_SEGMENT_DATA);
	 }
 
	 if (1 == ui1_full_range_flag)
	 {
		 if (DVB_SBTL_CLUTDS_FULL_RANGE_ENTRY_SIZE > ui4_data_len)
		 {
			 return (DVBR_WRONG_SEGMENT_DATA);
		 }
 
		 DVB_SBTL_CLUTDS_GET_FULL_RANGE_Y(pui1_data, ui1_Y);
		 DVB_SBTL_CLUTDS_GET_FULL_RANGE_CR(pui1_data, ui1_CR);
		 DVB_SBTL_CLUTDS_GET_FULL_RANGE_CB(pui1_data, ui1_CB);
		 DVB_SBTL_CLUTDS_GET_FULL_RANGE_T(pui1_data, ui1_T);
         
		 if (0 == ui1_Y)
		 {
			 ui1_T = 255;
		 }
		 pt_gl_color->a = (UINT8)(255 - ui1_T);

         if (pt_gl_color->a)
         {
    		 DVB_SBTL_CLUT_ADJ_YCBCR(ui1_Y, ui1_CB, ui1_CR);
     
    		 i4_R = ai4_Y_tbl[ui1_Y] + ai4_Cr_R_tbl[ui1_CR];
    		 i4_G = ai4_Y_tbl[ui1_Y] - ai4_Cr_G_tbl[ui1_CR] - ai4_Cb_G_tbl[ui1_CB];
    		 i4_B = ai4_Y_tbl[ui1_Y] + ai4_Cb_B_tbl[ui1_CB];
    		 
    		 i4_R = (i4_R / 1000);
    		 if (i4_R < 0)
    		 {
    			 i4_R = 0;
    		 }
    		 else if (i4_R > 255)
    		 {
    			 i4_R = 255;
    		 }
     
    		 i4_G = (i4_G / 1000);
    		 if (i4_G < 0)
    		 {
    			 i4_G = 0;
    		 }
    		 else if (i4_G > 255)
    		 {
    			 i4_G = 255;
    		 }
    		 
    		 i4_B = (i4_B / 1000);
    		 if (i4_B < 0)
    		 {
    			 i4_B = 0;
    		 }
    		 else if (i4_B > 255)
    		 {
    			 i4_B = 255;
    		 }
         }
         else
         {
            i4_R = i4_G = i4_B = 0;
         }
 
		 pt_gl_color->u1.r = (UINT8) i4_R;
		 pt_gl_color->u2.g = (UINT8) i4_G;
		 pt_gl_color->u3.b = (UINT8) i4_B;
		 
		 *pui4_inc_size = DVB_SBTL_CLUTDS_FULL_RANGE_ENTRY_SIZE;
	 }
	 else
	 {
        DVB_SBTL_CLUTDS_GET_HALF_RANGE_Y(pui1_data, ui1_Y);
        DVB_SBTL_CLUTDS_GET_HALF_RANGE_CR(pui1_data, ui1_CR);
        DVB_SBTL_CLUTDS_GET_HALF_RANGE_CB(pui1_data, ui1_CB);
        DVB_SBTL_CLUTDS_GET_HALF_RANGE_T(pui1_data, ui1_T);

        ui1_Y = ui1_Y << 2;
        ui1_CR = ui1_CR << 4;
        ui1_CB = ui1_CB << 4;
        ui1_T = ui1_T << 6;

        if (0 == ui1_Y)
        {
            ui1_T = 255;
        }
        pt_gl_color->a = (UINT8)(255 - ui1_T);

        if (pt_gl_color->a)
        {
            DVB_SBTL_CLUT_ADJ_YCBCR(ui1_Y, ui1_CB, ui1_CR);

            i4_R = ai4_Y_tbl[ui1_Y] + ai4_Cr_R_tbl[ui1_CR];
            i4_G = ai4_Y_tbl[ui1_Y] - ai4_Cr_G_tbl[ui1_CR] - ai4_Cb_G_tbl[ui1_CB];
            i4_B = ai4_Y_tbl[ui1_Y] + ai4_Cb_B_tbl[ui1_CB];

            i4_R = (i4_R / 1000);
            if (i4_R < 0)
            {
                i4_R = 0;
            }
            else if (i4_R > 255)
            {
                i4_R = 255;
            }

            i4_G = (i4_G / 1000);
            if (i4_G < 0)
            {
                i4_G = 0;
            }
            else if (i4_G > 255)
            {
                i4_G = 255;
            }

            i4_B = (i4_B / 1000);
            if (i4_B < 0)
            {
                i4_B = 0;
            }
            else if (i4_B > 255)
            {
                i4_B = 255;
            }
        }
        else
        {
            i4_R = i4_G = i4_B = 0;
        }
        pt_gl_color->u1.r = (UINT8) i4_R;
        pt_gl_color->u2.g = (UINT8) i4_G;
        pt_gl_color->u3.b = (UINT8) i4_B;

        *pui4_inc_size = DVB_SBTL_CLUTDS_HALF_RANGE_ENTRY_SIZE;
	 }

	 ALOGI( "getClutEntry [ui1_n_bits_entry_flag:%d][ui1_entry_id:%d]\n", ui1_n_bits_entry_flag, ui1_entry_id);
     ALOGI( "getClutEntry [T:%3d][Y:%3d][CR:%3d][CB:%3d]\n", ui1_T, ui1_Y, ui1_CR, ui1_CB);
	 ALOGI( "getClutEntry [A:%3d][R:%3d][G:%3d][B:%3d]\n", pt_gl_color->a, pt_gl_color->u1.r, pt_gl_color->u2.g, pt_gl_color->u3.b );
 
	 return i4_ret;
 }
 
 
 INT32 DvbClut::parseSegment_impl(
	 UINT8* 			 pui1_data, 
	 UINT32 			 ui4_data_len,
	 bool				 is_new_CLUT)
 {
	 UINT8					 ui1_CLUT_id = 0;
	 UINT8					 ui1_version = 0;
	 UINT8					 ui1_entry_id = 0;
	 UINT8					 ui1_full_range_flag = 0;
	 UINT8					 ui1_n_bits_entry_flag = 0;
	 UINT32 				 ui4_inc_size = 0;
	 INT32					 i4_ret;
	 bool					 is_update;
 
 
	 is_update = FALSE;
	 if (ui4_data_len < DVB_SBTL_CLUTDS_MIN_LEN)
	 {
		 return (DVBR_WRONG_SEGMENT_DATA);
	 }
 
	 DVB_SBTL_CLUTDS_GET_VERSION(pui1_data, ui1_version);
 
	 if (FALSE == is_new_CLUT)
	 {
		 if (this->ui1_version_number == ui1_version)
		 {
			 return (DVBR_OK);
		 }
	 }
	 else
	 {
		 is_update = TRUE;
		 DVB_SBTL_CLUTDS_GET_CLUT_ID(pui1_data, ui1_CLUT_id);
		 this->ui1_id = ui1_CLUT_id;
		 DVB_LOG("DvbClutMgr::parseSegment_impl() new  CLUT id = %d",ui1_CLUT_id);
	 }
 
    #if 0
	 {
		 UINT32 		 i;
 
		 for (i = 0; i < ui4_data_len; i++)
		 {
			 if (0 != i && 0 == i%10)
			 {
				 x_dbg_stmt( "\n  " );
			 }
			 
			 if (pui1_data[i] < 0x10)
			 {
				 x_dbg_stmt( " 0%x", pui1_data[i] );
			 }
			 else
			 {
				 x_dbg_stmt( " %x", pui1_data[i] );
			 }
		 }
 
		 x_dbg_stmt( "\n" );
	 }
    #endif
 
	 this->ui1_version_number = ui1_version;
	 pui1_data += DVB_SBTL_CLUTDS_MIN_LEN;
	 ui4_data_len -= DVB_SBTL_CLUTDS_MIN_LEN;
 
	 while (DVB_SBTL_CLUTDS_HALF_RANGE_ENTRY_SIZE <= ui4_data_len)
	 {
 
		 DVB_SBTL_CLUTDS_GET_ENTRY_ID(pui1_data, ui1_entry_id);
		 DVB_SBTL_CLUTDS_GET_N_BITS_ENTRY_FLAG(pui1_data, ui1_n_bits_entry_flag);
		 DVB_SBTL_CLUTDS_GET_FULL_RANGE_FLAG(pui1_data, ui1_full_range_flag);
 
		 if (DVB_SBTL_RGN_DEPTH_2BIT&ui1_n_bits_entry_flag)
		 {
			 if (ui1_entry_id <= (UINT8)DVB_SBTL_2BIT_CLUT_SIZE)
			 {
				 i4_ret = getClutEntry( 
					 pui1_data,
					 ui4_data_len,
					 DVB_SBTL_RGN_DEPTH_2BIT, 
					 ui1_full_range_flag,
					 ui1_entry_id,
					 &ui4_inc_size );
				 if (i4_ret < 0)
				 {
					 return (DVBR_WRONG_SEGMENT_DATA);
				 }
			 }
		 }
 
		 if (DVB_SBTL_RGN_DEPTH_4BIT&ui1_n_bits_entry_flag)
		 {
			 if (ui1_entry_id <= (UINT8)DVB_SBTL_4BIT_CLUT_SIZE)
			 {
				 i4_ret = getClutEntry( 
					 pui1_data,
					 ui4_data_len,
					 DVB_SBTL_RGN_DEPTH_4BIT, 
					 ui1_full_range_flag,
					 ui1_entry_id,
					 &ui4_inc_size );
				 if (i4_ret < 0)
				 {
					 return (DVBR_WRONG_SEGMENT_DATA);
				 }
			 }
		 }
 
		 if (DVB_SBTL_RGN_DEPTH_8BIT&ui1_n_bits_entry_flag)
		 {
			 i4_ret = getClutEntry( 
				 pui1_data,
				 ui4_data_len,
				 DVB_SBTL_RGN_DEPTH_8BIT, 
				 ui1_full_range_flag,
				 ui1_entry_id,
				 &ui4_inc_size );
			 if (i4_ret < 0)
			 {
				 return (DVBR_WRONG_SEGMENT_DATA);
			 }
		 }
 
		 if (0 == ui4_inc_size)
		 {
			 return (DVBR_WRONG_SEGMENT_DATA);
		 }
 
		 is_update = TRUE;
		 ui4_data_len -= ui4_inc_size;
		 pui1_data += ui4_inc_size;
	 }
 
	 if (TRUE == is_update)
	 {
		 return (DVBR_CONTENT_UPDATE);
	 }
	 else
	 {
		 return (DVBR_OK);
	 }
 }
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 INT32 DvbClut::dup(
	 UINT8				 ui1_color_depth,
	 DVB_NBIT_CLUT_T*	 pt_dup_nbit_clut,
	 DVB_NBIT_CLUT_T**	 ppt_this)
 {
	 DVB_NBIT_CLUT_T*	 pt_this;
	 GL_COLOR_T*			 pt_gl_color;
 
	 *ppt_this = NULL;
 
	 pt_this = (DVB_NBIT_CLUT_T*) malloc(sizeof(DVB_NBIT_CLUT_T));
 
	 if (NULL == pt_this)
	 {
		 return (DVBR_INSUFFICIENT_MEMORY);
	 }
 
	 pt_this->ui2_link_cnt = 1;
	 switch (ui1_color_depth)
	 {
		 case DVB_SBTL_RGN_DEPTH_2BIT:
			 pt_gl_color = (GL_COLOR_T*) malloc(DVB_SBTL_2BIT_CLUT_SIZE*sizeof(GL_COLOR_T));
 
			 if (NULL == pt_gl_color)
			 {
				 free(pt_this);
				 return (DVBR_INSUFFICIENT_MEMORY);
			 }
 
			 memcpy(pt_gl_color,
					  pt_dup_nbit_clut->pat_color_entry,
					  sizeof(GL_COLOR_T) * DVB_SBTL_2BIT_CLUT_SIZE);
 
			 pt_this->pat_color_entry = pt_gl_color;
			 break;
 
		 case DVB_SBTL_RGN_DEPTH_4BIT:
			 pt_gl_color = (GL_COLOR_T*) malloc(DVB_SBTL_4BIT_CLUT_SIZE*sizeof(GL_COLOR_T));
 
			 if (NULL == pt_gl_color)
			 {
				 free( pt_this);
				 return (DVBR_INSUFFICIENT_MEMORY);
			 }
 
			 memcpy(pt_gl_color,
					  pt_dup_nbit_clut->pat_color_entry,
					  sizeof(GL_COLOR_T) * DVB_SBTL_4BIT_CLUT_SIZE);
 
			 pt_this->pat_color_entry = pt_gl_color;
			 break;
 
		 case DVB_SBTL_RGN_DEPTH_8BIT:
			 pt_gl_color = (GL_COLOR_T*) malloc(DVB_SBTL_8BIT_CLUT_SIZE*sizeof(GL_COLOR_T));
 
			 if (NULL == pt_gl_color)
			 {
				 free(pt_this);
				 return (DVBR_INSUFFICIENT_MEMORY);
			 }
 
			 memcpy(pt_gl_color,
					  pt_dup_nbit_clut->pat_color_entry,
					  sizeof(GL_COLOR_T) * DVB_SBTL_8BIT_CLUT_SIZE);
 
			 pt_this->pat_color_entry = pt_gl_color;
			 break;
 
		 default:
			 free(pt_this);
			 return (DVBR_INV_ARG);
	 }
 
	 /* Not default table */
	 if (0 != pt_dup_nbit_clut->ui2_link_cnt)
	 {
	    pt_dup_nbit_clut->ui2_link_cnt--;
	 }
 
	 *ppt_this = pt_this;
	 return (DVBR_OK);
 }
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
  VOID DvbClut::clutDelete(DVB_NBIT_CLUT_T*	pt_dvb_nbit_clut,
	 UINT8					 ui1_color_depth)
 {
 	//DVB_sbtl_service_mem_free(pt_service, pt_this->pat_color_entry);
    //DVB_sbtl_service_mem_free(pt_service, pt_this);
	 free(pt_dvb_nbit_clut->pat_color_entry);
	 free(pt_dvb_nbit_clut);
 }
 
 /*-----------------------------------------------------------------------------
  * Name: 
  *
  * Description: 
  *
  * Inputs:  pt_this	 Pointer points to the 
  *
  * Outputs: -
  *
  * Returns: -
  ----------------------------------------------------------------------------*/
 VOID DvbClut::release(DVB_NBIT_CLUT_T*	pt_dvb_nbit_clut,UINT8		 ui1_color_depth)
 {
	 if (0 == pt_dvb_nbit_clut->ui2_link_cnt) /* default table */
	 {
		 return;
	 }
 
	 pt_dvb_nbit_clut->ui2_link_cnt--;
 
	 if (0 == pt_dvb_nbit_clut->ui2_link_cnt)
	 {
		 DvbClut::clutDelete(pt_dvb_nbit_clut,ui1_color_depth);
	 }
}
#endif

