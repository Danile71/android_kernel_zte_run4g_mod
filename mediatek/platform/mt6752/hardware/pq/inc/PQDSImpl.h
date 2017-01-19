#ifndef __PQDSIMPL_H__
#define __PQDSIMPL_H__

/*
* header files
*/

#define DSHP_ANDROID_PLATFORM

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed int int32_t;

// Initial register values to DS HW
struct DSInitReg {
    uint32_t useless; // To pass compilation
};

// Fields collected from DS HW
struct DSInput {
    uint16_t SrcWidth;
    uint16_t SrcHeight;
    uint16_t DstWidth;
    uint16_t DstHeight;
    uint16_t orgGainHigh;
    uint16_t orgGainMid;
};

// Fields which will be set to HW registers
struct DSOutput {
    uint16_t outGainHigh;
    uint16_t outGainMid;
};

// DS FW registers
struct DSReg {
    int32_t iUpSlope;        // Range from -64 to 63
	int32_t iUpThreshold;    // Range from 0 to 4095
	int32_t iDownSlope;      // Range from -64 to 63
	int32_t iDownThreshold;  // Range from 0 to 4095
};


///////////////////////////////////////////////////////////////////////////////
// DS FW Processing class
///////////////////////////////////////////////////////////////////////////////
class CPQDSFW
{

    /* ........Dynamic Sharpness Process, functions......... */
public:

#ifdef DSHP_ANDROID_PLATFORM
    void onCalculate(const DSInput *input, DSOutput *output);
    void onInitPlatform(void);
#else
    void vDrvDSProc_int(const DSInput * input, DSOutput * output);
#endif


    void DSInitialize(void);

    CPQDSFW()
    {
        memset( this, 0, sizeof( * this ) );
        pDSReg = new DSReg;
        DSInitialize();
    };

    ~CPQDSFW()
    {
        delete pDSReg;
    };

private:

    /* ........Dynamic Sharpness Process, variables......... */
public:
    DSReg * pDSReg;

private:
    DSInput oldInput;
    DSOutput oldOutput;
};

#endif //__PQDSIMPL_H__
