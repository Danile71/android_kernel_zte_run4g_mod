#ifndef _ISP_TUNING_IDX_H_
#define _ISP_TUNING_IDX_H_


namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// INDEX_T
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct CUSTOM_NVRAM_REG_INDEX
{
    MUINT16  OBC;
    MUINT16  BPC;
    MUINT16  NR1;
    MUINT16  CFA;
    MUINT16  GGM;
    MUINT16  ANR;
    MUINT16  CCR;
    MUINT16  EE;
    MUINT16  NR3D;
    MUINT16  MFB;    
    MUINT16  LCE;
} CUSTOM_NVRAM_REG_INDEX_T;

typedef CUSTOM_NVRAM_REG_INDEX INDEX_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IndexMgr
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct IndexMgr : protected INDEX_T
{
public:
    IndexMgr()
    {
        ::memset(static_cast<INDEX_T*>(this), 0, sizeof(INDEX_T));
    }

    IndexMgr(INDEX_T const& rIndex)
    {
        (*this) = rIndex;
    }

    IndexMgr& operator=(INDEX_T const& rIndex)
    {
        *static_cast<INDEX_T*>(this) = rIndex;
        return  (*this);
    }

public:
    void dump() const;

public: // Set Index
    MBOOL   setIdx_OBC  (MUINT16 const idx);
    MBOOL   setIdx_BPC  (MUINT16 const idx);
    MBOOL   setIdx_NR1  (MUINT16 const idx);
    MBOOL   setIdx_CFA  (MUINT16 const idx);
    MBOOL   setIdx_GGM  (MUINT16 const idx);
    MBOOL   setIdx_ANR  (MUINT16 const idx);
    MBOOL   setIdx_CCR  (MUINT16 const idx);
    MBOOL   setIdx_EE   (MUINT16 const idx);
    MBOOL   setIdx_NR3D (MUINT16 const idx);
    MBOOL   setIdx_MFB  (MUINT16 const idx);
    MBOOL   setIdx_LCE  (MUINT16 const idx);

public:     ////    Get Index
    inline  MUINT16 getIdx_OBC()  const { return OBC; }
    inline  MUINT16 getIdx_BPC()  const { return BPC; }
    inline  MUINT16 getIdx_NR1()  const { return NR1; }
    inline  MUINT16 getIdx_CFA()  const { return CFA; }
    inline  MUINT16 getIdx_GGM()  const { return GGM; }
    inline  MUINT16 getIdx_ANR()  const { return ANR; }
    inline  MUINT16 getIdx_CCR()  const { return CCR; }
    inline  MUINT16 getIdx_EE()   const { return EE; }
    inline  MUINT16 getIdx_NR3D() const { return NR3D; }
    inline  MUINT16 getIdx_MFB()  const { return MFB; }    
    inline  MUINT16 getIdx_LCE()  const { return LCE; }
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Index Set Template
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <EIspProfile_T ispProfile, MUINT32 sensor = -1, MUINT32 scene = -1, MUINT32 iso = -1>
struct IdxSet
{
    static INDEX_T const idx;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IIdxSetMgrBase
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IdxSetMgrBase
{
public:

    static IdxSetMgrBase& getInstance();

    virtual ~IdxSetMgrBase() {}

public:
    virtual INDEX_T const*
    get(
        MUINT32 const ispProfile, MUINT32 sensor=-1, MUINT32 const scene=-1, MUINT32 const iso=-1
    ) const = 0;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IdxSetMgr
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IdxSetMgr : public IdxSetMgrBase
{
    friend class IdxSetMgrBase;

private:
    INDEX_T const* m_pPreview                   [ESensorMode_NUM][eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pVideo                     [ESensorMode_NUM][eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pCapture                   [ESensorMode_NUM][eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pN3D_Preview               [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pN3D_Video                 [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pN3D_Capture               [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMFB_Blending_All_Off      [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMFB_Blending_All_Off_SWNR [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMFB_PostProc_Mixing       [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMFB_PostProc_Mixing_SWNR  [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMFB_Capture_EE_Off        [ESensorMode_NUM][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pIHDR_Preview              [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pIHDR_Video                [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMulti_Pass_ANR1           [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMulti_Pass_ANR2           [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMFB_Multi_Pass_ANR1       [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pMFB_Multi_Pass_ANR2       [eNUM_OF_ISO_IDX];
    
private:
    MVOID linkIndexSet();

private:    ////    Normal
    inline MBOOL isInvalid(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  ( sensor >= ESensorMode_NUM || scene >= eNUM_OF_SCENE_IDX || iso >= eNUM_OF_ISO_IDX );
    }
    inline INDEX_T const* get_Preview(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pPreview[sensor][scene][iso];
    }
    inline INDEX_T const* get_Video(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pVideo[sensor][scene][iso];
    }
    inline INDEX_T const* get_Capture(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, scene, iso) ? NULL : m_pCapture[sensor][scene][iso];
    }
    inline INDEX_T const* get_N3D_Preview(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pN3D_Preview[iso];
    }
    inline INDEX_T const* get_N3D_Video(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pN3D_Video[iso];
    }
    inline INDEX_T const* get_N3D_Capture(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pN3D_Capture[iso];
    }
    inline INDEX_T const* get_MFB_Blending_All_Off(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMFB_Blending_All_Off[iso];
    }
    inline INDEX_T const* get_MFB_Blending_All_Off_SWNR(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMFB_Blending_All_Off_SWNR[iso];
    }    
    inline INDEX_T const* get_MFB_PostProc_Mixing(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMFB_PostProc_Mixing[iso];
    }
    inline INDEX_T const* get_MFB_PostProc_Mixing_SWNR(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMFB_PostProc_Mixing_SWNR[iso];
    }    
    inline INDEX_T const* get_MFB_Capture_EE_Off(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(sensor, 0, iso) ? NULL : m_pMFB_Capture_EE_Off[sensor][iso];
    }
    inline INDEX_T const* get_IHDR_Preview(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pIHDR_Preview[iso];
    }
    inline INDEX_T const* get_IHDR_Video(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pIHDR_Video[iso];
    }
    inline INDEX_T const* get_Multi_Pass_ANR1(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMulti_Pass_ANR1[iso];
    }
    inline INDEX_T const* get_Multi_Pass_ANR2(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMulti_Pass_ANR2[iso];
    }
    inline INDEX_T const* get_MFB_Multi_Pass_ANR1(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMFB_Multi_Pass_ANR1[iso];
    }
    inline INDEX_T const* get_MFB_Multi_Pass_ANR2(MUINT32 const sensor, MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(0, 0, iso) ? NULL : m_pMFB_Multi_Pass_ANR2[iso];
    }

public:
    virtual
    INDEX_T const*
    get(MUINT32 ispProfile, MUINT32 const sensor/*=-1*/, MUINT32 const scene/*=-1*/, MUINT32 const iso/*=-1*/) const;

};  //  class IdxSetMgr

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IDX_SET
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define IDX_SET(OBC, BPC, NR1, CFA, GGM, ANR, CCR, EE, NR3D, MFB, LCE)\
    {\
        OBC, BPC, NR1, CFA, GGM, ANR, CCR, EE, NR3D, MFB, LCE\
    }

#define IDX_MODE_Preview(sensor, scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Preview, sensor, scene, iso>::idx =

#define IDX_MODE_Video(sensor, scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Video, sensor, scene, iso>::idx =

#define IDX_MODE_Capture(sensor, scene, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Capture, sensor, scene, iso>::idx =

#define IDX_MODE_N3D_Preview(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_N3D_Preview, -1, -1, iso>::idx =

#define IDX_MODE_N3D_Video(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_N3D_Video, -1, -1, iso>::idx =

#define IDX_MODE_N3D_Capture(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_N3D_Capture, -1, -1, iso>::idx =

#define IDX_MODE_MFB_Blending_All_Off(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_MFB_Blending_All_Off, -1, -1, iso>::idx =

#define IDX_MODE_MFB_Blending_All_Off_SWNR(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_MFB_Blending_All_Off_SWNR, -1, -1, iso>::idx =

#define IDX_MODE_MFB_PostProc_Mixing(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_MFB_PostProc_Mixing, -1, -1, iso>::idx =

#define IDX_MODE_MFB_PostProc_Mixing_SWNR(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_MFB_PostProc_Mixing_SWNR, -1, -1, iso>::idx =

#define IDX_MODE_MFB_Capture_EE_Off(sensor, iso)\
    template <> INDEX_T const IdxSet<EIspProfile_MFB_Capture_EE_Off, sensor, -1, iso>::idx =

#define IDX_MODE_IHDR_Preview(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_IHDR_Preview, -1, -1, iso>::idx =

#define IDX_MODE_IHDR_Video(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_IHDR_Video, -1, -1, iso>::idx =

#define IDX_MODE_Multi_Pass_ANR1(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Capture_MultiPass_ANR_1, -1, -1, iso>::idx =

#define IDX_MODE_Multi_Pass_ANR2(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_Capture_MultiPass_ANR_2, -1, -1, iso>::idx =

#define IDX_MODE_MFB_Multi_Pass_ANR1(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_MFB_MultiPass_ANR_1, -1, -1, iso>::idx =

#define IDX_MODE_MFB_Multi_Pass_ANR2(iso)\
    template <> INDEX_T const IdxSet<EIspProfile_MFB_MultiPass_ANR_2, -1, -1, iso>::idx =

};  //  NSIspTuning
#endif //  _ISP_TUNING_IDX_H_

