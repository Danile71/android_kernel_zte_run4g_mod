#ifndef PREVIEWFEATURE_BUFF_MGR_H
#define PREVIEWFEATURE_BUFF_MGR_H

/******************************************************************************
 *
 ******************************************************************************/
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <mtkcam/common.h>
//
#include <vector>
using namespace std;
using namespace NSCam;
//
/******************************************************************************
 *
 ******************************************************************************/
class PREVIEWFEATUREBuffer : public IImgBuf
{
   
/******************************************************************************
*   Inheritance from IMemBuf.
*******************************************************************************/
public:
    virtual int64_t             getTimestamp() const                    { return mTimestamp; }
    virtual void                setTimestamp(int64_t const timestamp)   { mTimestamp = timestamp; }
    virtual size_t              getBufSize() const                      { return PreviewBuffer->getBufSizeInBytes(0); }
    virtual void*               getVirAddr() const                      { return (void*)PreviewBuffer->getBufVA(0); }
    virtual void*               getPhyAddr() const                      { return (void*)PreviewBuffer->getBufPA(0); }
    virtual const char*         getBufName() const                      { return mName; }              
    virtual int                 getIonFd() const                        { return PreviewBuffer->getFD(0); }
       
/******************************************************************************
*   Inheritance from IImgBuf.
*******************************************************************************/
public:     
    virtual String8 const&      getImgFormat()      const               { return mformat; }
    virtual uint32_t            getImgWidthStride(
                                    uint_t const uPlaneIndex = 0
                                )   const
                                {
                                	  if(uPlaneIndex==0)
                                	  	return mWidth;
                                	  	                                    
                                    return mWidth>>1;
                                }

    virtual uint32_t            getImgWidth()       const               { return mWidth;  }
    virtual uint32_t            getImgHeight()      const               { return mHeight; }
    virtual uint32_t            getBitsPerPixel()   const               { return mBpp;    }


/******************************************************************************
*   Initialization.
*******************************************************************************/
public:
    PREVIEWFEATUREBuffer( uint32_t _w = 0, 
              uint32_t _h = 0, 
              uint32_t _bpp = 0, 
              uint32_t _bufsize = 0, 
              String8  _format = String8(""),
              const char* _name = "")
        : IImgBuf()
        , mWidth(_w)
        , mHeight(_h)
        , mBpp(_bpp)
        , mbufSize(_bufsize)
        , mTimestamp(0)
        , mformat(_format)
        , mName(_name)
        , mpIMemDrv(NULL)
    {
        createBuffer();
    }

    virtual                     ~PREVIEWFEATUREBuffer() {destroyBuffer();};
    bool                        operator!() const   { return PreviewBuffer->getBufVA(0) != 0; }

/******************************************************************************
*   Initialization.
*******************************************************************************/    
private:    
    virtual void                createBuffer();   
    virtual void                destroyBuffer();    
    
private:
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mBpp;
    uint32_t mbufSize;
    int64_t  mTimestamp;
    String8  mformat;
    const char* mName;
    IImageBufferAllocator *mpIMemDrv;
    sp<IImageBuffer> PreviewBuffer;
};

#endif
