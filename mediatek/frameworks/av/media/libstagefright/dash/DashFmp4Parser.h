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

#ifndef PARSER_H_

#define PARSER_H_

#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/DataSource.h>
#include <utils/Vector.h>

namespace android {

struct ABuffer;
struct MetaData;

struct DashFmp4Parser : public RefBase {

    DashFmp4Parser();

	void setUserInfo(const sp<AMessage> &notify, int64_t baseMediaUs,const char* obServerName);
	status_t flush(sp<AMessage> &flushMsg);
	status_t stop();
	bool hasAvaliBuffer();
	status_t  proceedBuffer(sp<ABuffer> &buffer);

	enum {
		kWhatSidx = 'sidx',
		kWhatFileMeta = 'fmta',	
		kWhatError = 'errn',
	};
	struct SsixEntry {
		uint8_t  mLevel;
		uint32_t mRange_Size;
	};
	struct SidxEntry {	
		uint32_t mReference_ID;//for which track
		uint64_t mEarliest_PTS;
		uint64_t mFirst_Offset;
		int64_t mOffset;
		size_t mSize;
		uint64_t mSegmentStartUs;
		uint32_t mDurationUs;
		uint32_t mTimeScale;
		uint8_t mReference_Type;//0 -- media 1--sub-sidx
		bool mStarts_with_SAP;
		uint8_t mSAP_Type;
		Vector<SsixEntry> mSsix;	 
	};


	enum{
		Track_Invalid_Min =0,
		Track_Video = 1,
		Track_Audio =2 ,
		Track_SubTitles = 4,
		Track_Invalid_Max =5,
	};
		

	sp<AMessage> getFormat(uint32_t trackType );
	status_t dequeueAccessUnit(uint32_t trackType, sp<ABuffer> *accessUnit );    

	status_t seekTo(bool audio, int64_t timeUs);
	bool isSeekable() const;


protected:
    virtual ~DashFmp4Parser();

private:

    struct TrackFragment;
    struct DynamicTrackFragment;
    struct StaticTrackFragment;

    struct DispatchEntry {
        uint32_t mType;
        uint32_t mParentType;
        status_t (DashFmp4Parser::*mHandler)(uint32_t, size_t, uint64_t);
    };

    struct Container {
        uint64_t mOffset;
        uint64_t mBytesRemaining;
        uint32_t mType;
        bool mExtendsToEOF;
    };

    struct SampleDescription {
        uint32_t mType;
        uint16_t mDataRefIndex;
        sp<AMessage> mFormat;
    };
    struct SampleInfo {
        off64_t mOffset;
        size_t mSize;
        uint32_t mPresentationTime;
        size_t mSampleDescIndex;
        uint32_t mFlags;
		
		sp<ABuffer> mIV; // IV_size* bytes
		uint16_t mSubsample_count;
		Vector<uint32_t> mBytesOfClearData;
		Vector<uint32_t> mBytesOfEncryptedData;		 

    };

    struct MediaDataInfo {
        sp<ABuffer> mBuffer;
        off64_t mOffset;
    };


	
	struct MfraEntry{
	    uint32_t mTrackID;
	    uint64_t mTime;
	    uint64_t mMoofOffset;
	    uint32_t mTrafNumber;
	    uint32_t mTrunNumber;
	    uint32_t mSampleNumber;
	};
	struct TencInfo{
		uint32_t mIsEncrypted;
		uint8_t mIV_Size;
		uint8_t mKID[16];  //contains 16bytes
	};
	struct PsshInfo{
		uint8_t mSystemId[16];;  //contains 16bytes UUID
		uint32_t mDataSize;
	 	uint8_t* mData;  //contains mDataSize  bytes,content specific data
	};


    struct TrackInfo {
        enum Flags {
            kTrackEnabled     = 0x01,
            kTrackInMovie     = 0x02,
            kTrackInPreview   = 0x04,
        };

        uint32_t mTrackID;
        uint32_t mFlags;
        uint32_t mDuration;  // This is the duration in terms of movie timescale!
        uint64_t mSidxDuration; // usec, from sidx box, which can use a different timescale

        uint32_t mMediaTimeScale;

        uint32_t mMediaHandlerType;
        Vector<SampleDescription> mSampleDescs;

        // from track extends:
        uint32_t mDefaultSampleDescriptionIndex;
        uint32_t mDefaultSampleDuration;
        uint32_t mDefaultSampleSize;
        uint32_t mDefaultSampleFlags;

        uint32_t mDecodingTime;

        Vector<SidxEntry> mSidx;

		
		bool mIsEncrpyted;
		TencInfo mTenc;		
		uint32_t mStartDecodeSegmentIndex;
		int64_t  mBaseMediaTimeUs;

        sp<StaticTrackFragment> mStaticFragment;
        List<sp<TrackFragment> > mFragments;
    };

    struct TrackFragmentHeaderInfo {
        enum Flags {
            kBaseDataOffsetPresent         = 0x01,
            kSampleDescriptionIndexPresent = 0x02,
            kDefaultSampleDurationPresent  = 0x08,
            kDefaultSampleSizePresent      = 0x10,
            kDefaultSampleFlagsPresent     = 0x20,
            kDurationIsEmpty               = 0x10000,
        };

        uint32_t mTrackID;
        uint32_t mFlags;
        uint64_t mBaseDataOffset;
        uint32_t mSampleDescriptionIndex;
        uint32_t mDefaultSampleDuration;
        uint32_t mDefaultSampleSize;
        uint32_t mDefaultSampleFlags;

        uint64_t mDataOffset;
    };

    static const DispatchEntry kDispatchTable[];

    off_t mBufferPos;
    bool mSuspended;
    bool mDoneWithMoov;
    off_t mFirstMoofOffset; // used as the starting point for offsets calculated from the sidx box
    sp<ABuffer> mBuffer;
    Vector<Container> mStack;
    KeyedVector<uint32_t, TrackInfo> mTracks;  // TrackInfo by trackID
    Vector<MediaDataInfo> mMediaData;

    uint32_t mCurrentTrackID;

    status_t mFinalResult;

    TrackFragmentHeaderInfo mTrackFragmentHeaderInfo;
	

     bool mIsDrm;
     KeyedVector<uint32_t, uint32_t> mTrackIndexAndTrackIDMaps;  
     KeyedVector<uint32_t, MfraEntry> mMfraEntries;  
	 Vector<int64_t>mMoofOffsets;

	uint32_t  mState;
	int32_t mHaveTrack;
	bool mIsFlushing;
	Mutex mDataLock;
	sp<AMessage> mNotify;//notify the meta  
	sp<MetaData> mFileMeta;
	const char* mObServerName;
	int64_t mBaseMediaTimeUs;
	
	void notifyObserver(int32_t whatInfo);
	status_t checkTrackValid(uint32_t trackType);
	status_t setTrackStartDecodeTime(uint32_t trackType,uint32_t startDecoderSegmentIndex);

	status_t onProceed();
    void enter(off64_t offset, uint32_t type, uint64_t size);

    uint16_t readU16(size_t offset);
    uint32_t readU32(size_t offset);
    uint64_t readU64(size_t offset);

    uint8_t readU8(size_t offset);
    uint32_t readU24(size_t offset);
    uint32_t  readUInt(size_t offset, int size) ;

    void skip(off_t distance);
    status_t need(size_t size);
    bool fitsContainer(uint64_t size) const;

    status_t parseTrackHeader(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseMediaHeader(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseMediaHandler(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseTrackExtends(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseTrackFragmentHeader(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseTrackFragmentRun(
            uint32_t type, size_t offset, uint64_t size);



	//mfra
	status_t parseMovieFragmentRandomAccess(
	            uint32_t type, size_t offset, uint64_t size);
	status_t parseTrackFragmentRandomAccess(
	            uint32_t type, size_t offset, uint64_t size);
	status_t parseMovieFragmentRandomAccessOffset (
	            uint32_t type, size_t offset, uint64_t size);
	//subtitle
	status_t parseSubtitleSampleEntry(
	        uint32_t type, size_t offset, uint64_t size);
	//ssix
	status_t parseSubSegmentIndex(
	        uint32_t type, size_t offset, uint64_t size);
	
	//drm : pssh/tenc/senc
	status_t parseProtectionSystemSpecificHeader(
	        uint32_t type, size_t offset, uint64_t size);
	status_t parseOriginalFormat(
	        uint32_t type, size_t offset, uint64_t size);
	status_t parseSchemeType(
	        uint32_t type, size_t offset, uint64_t size);
	status_t  parseSchemeInformation(
	        uint32_t type, size_t offset, uint64_t size);

	status_t parseTrackEncryption(
	        uint32_t type, size_t offset, uint64_t size);   
	status_t parseSampleEncryption(
	        uint32_t type, size_t offset, uint64_t size);
	
	//aux:saiz,saio, not parsed now
	status_t parseSampleAuxiliaryInformationOffsets(
	        uint32_t type, size_t offset, uint64_t size);
	status_t parseSampleAuxiliaryInformationSize(
	        uint32_t type, size_t offset, uint64_t size);


    status_t parseVisualSampleEntry(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseAudioSampleEntry(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseSampleSizes(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseCompactSampleSizes(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseSampleToChunk(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseChunkOffsets(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseChunkOffsets64(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseAVCCodecSpecificData(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseESDSCodecSpecificData(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseMediaData(
            uint32_t type, size_t offset, uint64_t size);

    status_t parseSegmentIndex(
            uint32_t type, size_t offset, uint64_t size);

    TrackInfo *editTrack(uint32_t trackID, bool createIfNecessary = false);

    ssize_t  findTrack(uint32_t trackType);

    status_t makeAccessUnit(
            TrackInfo *info,
            const SampleInfo &sample,
            const MediaDataInfo &mdatInfo,
            sp<ABuffer> *accessUnit);

    status_t getSample(
            TrackInfo *info,
            sp<TrackFragment> *fragment,
            SampleInfo *sampleInfo);

    static int CompareSampleLocation(
        const SampleInfo &sample, const MediaDataInfo &mdatInfo);


    void copyBuffer(
            sp<ABuffer> *dst,
            size_t offset, uint64_t size) const;

    DISALLOW_EVIL_CONSTRUCTORS(DashFmp4Parser);
};


struct DashFmp4Parser::TrackFragment : public RefBase {
    TrackFragment() {}

    virtual status_t getSample(SampleInfo *info) = 0;
    virtual void advance() = 0;

    virtual status_t signalCompletion() = 0;
    virtual bool complete() const = 0;

protected:
    virtual ~TrackFragment() {}

private:
    DISALLOW_EVIL_CONSTRUCTORS(TrackFragment);
};

struct DashFmp4Parser::DynamicTrackFragment : public DashFmp4Parser::TrackFragment {
    DynamicTrackFragment();

    virtual status_t getSample(SampleInfo *info);
    virtual void advance();

    void addSample(
            off64_t dataOffset, size_t sampleSize,
            uint32_t presentationTime,
            size_t sampleDescIndex,
            uint32_t flags);

    // No more samples will be added to this fragment.
    virtual status_t signalCompletion();

    virtual bool complete() const;

	status_t getSampleByIndex(uint32_t sampleIndex,SampleInfo *info);


protected:
    virtual ~DynamicTrackFragment();

private:
    bool mComplete;
    size_t mSampleIndex;
    Vector<SampleInfo> mSamples;

    size_t mSampleCount;


    DISALLOW_EVIL_CONSTRUCTORS(DynamicTrackFragment);
};

struct DashFmp4Parser::StaticTrackFragment : public DashFmp4Parser::TrackFragment {
    StaticTrackFragment();

    virtual status_t getSample(SampleInfo *info);
    virtual void advance();

    virtual status_t signalCompletion();
    virtual bool complete() const;

    status_t parseSampleSizes(
            DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size);

    status_t parseCompactSampleSizes(
            DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size);

    status_t parseSampleToChunk(
            DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size);

    status_t parseChunkOffsets(
            DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size);

    status_t parseChunkOffsets64(
            DashFmp4Parser *parser, uint32_t type, size_t offset, uint64_t size);

protected:
    virtual ~StaticTrackFragment();

private:
    size_t mSampleIndex;
    size_t mSampleCount;
    uint32_t mChunkIndex;

    SampleInfo mSampleInfo;

    sp<ABuffer> mSampleSizes;
    sp<ABuffer> mCompactSampleSizes;

    sp<ABuffer> mSampleToChunk;
    ssize_t mSampleToChunkIndex;
    size_t mSampleToChunkRemaining;

    sp<ABuffer> mChunkOffsets;
    sp<ABuffer> mChunkOffsets64;
    uint32_t mPrevChunkIndex;
    uint64_t mNextSampleOffset;

    void updateSampleInfo();

    DISALLOW_EVIL_CONSTRUCTORS(StaticTrackFragment);
};


}  // namespace android

#endif  // PARSER_H_

