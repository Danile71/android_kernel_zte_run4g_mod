#ifndef _VOW_API_AP_H
#define _VOW_API_AP_H

#ifdef __cplusplus
extern "C" {
#endif

// the HMM parameters
#define STATENUM_K2 (int)20
#define MIXNUM_K2 (int)8
#define DIM_K2 (int)39
#define TOP_N_MOVED_MIX_K2 (int)100

#define STATENUM_SV_K2 (int)16
#define MIXNUM_SV_K2 (int)8
#define DIM_SV_K2 (int)39
#define TOP_N_MOVED_MIX_SV_K2 (int)100

#define TRANP_BYTES_K2 (int)((STATENUM_K2+2)*(STATENUM_K2+2)*4)
#define TRANP_INMD32_BYTES_K2 (int)((STATENUM_K2)*(STATENUM_K2)*4)
#define MIXTURE_WT_BYTES_K2 (int)(STATENUM_K2*MIXNUM_K2*4)
#define GCONST_BYTES_K2 (int)(STATENUM_K2*MIXNUM_K2*4)
#define MEAN_PREC_BYTES_K2 (int)(STATENUM_K2*MIXNUM_K2*DIM_K2*2)
#define MOVED_MIX_BYTES_K2 (int)(STATENUM_K2*MIXNUM_K2)
#define MOVED_MIXMEAN_BYTES_K2 (int)(TOP_N_MOVED_MIX_K2*DIM_K2*2)

#define TRANP_BYTES_SV_K2 (int)((STATENUM_SV_K2+2)*(STATENUM_SV_K2+2)*4)
#define TRANP_INMD32_BYTES_SV_K2 (int)((STATENUM_SV_K2)*(STATENUM_SV_K2)*4)
#define MIXTURE_WT_BYTES_SV_K2 (int)(STATENUM_SV_K2*MIXNUM_SV_K2*4)
#define GCONST_BYTES_SV_K2 (int)(STATENUM_SV_K2*MIXNUM_SV_K2*4)
#define MEAN_PREC_BYTES_SV_K2 (int)(STATENUM_SV_K2*MIXNUM_SV_K2*DIM_SV_K2*2)
#define MOVED_MIX_BYTES_SV_K2 (int)(STATENUM_SV_K2*MIXNUM_SV_K2)
#define MOVED_MIXMEAN_BYTES_SV_K2 (int)(TOP_N_MOVED_MIX_SV_K2*DIM_SV_K2*2)

#define LZERO_FIXED  (int)(-1.0E9)   /* ~log(0) */
#define LSMALL_FIXED (int)(-0.5E9)   /* log values < LSMALL_FIXED are set to LZERO_FIXED */

struct VOW_TestInfo {
    int mode;		// keyword (1) or keyword+SV (2)
    int modelType;		// load SModel (0) or IModel (1)
    int commandID;
    const char	*path;
    int	rtnModelSize;
    char	*rtnModel;
};

int getSizes (struct VOW_TestInfo *vowInfo);
int TestingInitAP (struct VOW_TestInfo *vowInfo);

#ifdef __cplusplus
}
#endif

#endif
