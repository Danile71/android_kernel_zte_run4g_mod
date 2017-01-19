#ifndef _FT_MODULE_H_
#define _FT_MODULE_H_
#include "DriverInterface.h"
#include "CmdTarget.h"


class FtModWifi : public CmdTarget
{
public:
	FtModWifi(void);
	virtual ~FtModWifi(void);

public:
	virtual void exec(Frame*);
};

class FtModGPS : public CmdTarget
{
public:
	FtModGPS(void);
	virtual ~FtModGPS(void);

public:
	virtual void exec(Frame*);
};


class FtModNFC : public CmdTarget
{
public:
	FtModNFC(void);
	virtual ~FtModNFC(void);

public:
	virtual void exec(Frame*);

protected:
	 int init(Frame*);
};


class FtModBT : public CmdTarget
{
public:
	FtModBT(void);
	virtual ~FtModBT(void);

public:
	virtual void exec(Frame*);
};

class FtModFM : public CmdTarget
{
public:
	FtModFM(void);
	virtual ~FtModFM(void);

public:
	virtual void exec(Frame*);

protected:
	 int init(Frame*);
};

class FtModAudio : public CmdTarget
{
public:
	FtModAudio(void);
	virtual ~FtModAudio(void);

public:
	virtual void exec(Frame*);

protected:
	 int init(Frame*);
};

class FtModCCAP : public CmdTarget
{
public:
	FtModCCAP(void);
	virtual ~FtModCCAP(void);

public:
	virtual void exec(Frame*);
protected:
    int init(Frame*);
};

class FtModMATV : public CmdTarget
{
public:
	FtModMATV(void);
	virtual ~FtModMATV(void);

public:
	virtual void exec(Frame*);
};

class FtModHDCP : public CmdTarget
{
public:
	FtModHDCP(void);
	virtual ~FtModHDCP(void);

public:
	virtual void exec(Frame*);
protected:
    int init(Frame*);

};

class FtModDRM : public CmdTarget
{
public:
	FtModDRM(void);
	virtual ~FtModDRM(void);

public:
	virtual void exec(Frame*);
};

class FtModNvramBackup : public CmdTarget
{
public:
	FtModNvramBackup(void);
	virtual ~FtModNvramBackup(void);

private:	
	bool SendNVRAMFile(unsigned char file_ID, FT_NVRAM_BACKUP_CNF* pft_cnf);
	
public:
	virtual void exec(Frame*);
};

class FtModNvramRestore : public CmdTarget
{
public:
	FtModNvramRestore(void);
	virtual ~FtModNvramRestore(void);

public:
	virtual void exec(Frame*);

};

class FtModNvramReset : public CmdTarget
{
public:
	FtModNvramReset(void);
	virtual ~FtModNvramReset(void);

public:
	virtual void exec(Frame*);
};

class FtModNvramRead : public CmdTarget
{
public:
	FtModNvramRead(void);
	virtual ~FtModNvramRead(void);

public:
	virtual void exec(Frame*);
};


class FtModNvramWrite : public CmdTarget
{
public:
	FtModNvramWrite(void);
	virtual ~FtModNvramWrite(void);

public:
	virtual void exec(Frame*);
};


class FtModTestAlive : public CmdTarget
{
public:
	FtModTestAlive(void);
	virtual ~FtModTestAlive(void);

public:
	virtual void exec(Frame*);
};

class FtModVersionInfo : public CmdTarget
{
public:
	FtModVersionInfo(void);
	virtual ~FtModVersionInfo(void);

public:
	virtual void exec(Frame*);
};



class FtModVersionInfo2 : public CmdTarget
{
public:
	FtModVersionInfo2(void);
	virtual ~FtModVersionInfo2(void);

public:
	virtual void exec(Frame*);
};


class FtModChipInfo : public CmdTarget
{
public:
	FtModChipInfo(void);
	virtual ~FtModChipInfo(void);

public:
	virtual void exec(Frame*);
};


class FtModPowerOff : public CmdTarget
{
public:
	FtModPowerOff(void);
	virtual ~FtModPowerOff(void);

public:
	virtual void exec(Frame*);
};


class FtModReboot : public CmdTarget
{
public:
	FtModReboot(void);
	virtual ~FtModReboot(void);

public:
	virtual void exec(Frame*);
};


class FtModBuildProp : public CmdTarget
{
public:
	FtModBuildProp(void);
	virtual ~FtModBuildProp(void);

public:
	virtual void exec(Frame*);
};



class FtModGSensor : public CmdTarget
{
public:
	FtModGSensor(void);
	virtual ~FtModGSensor(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

};

class FtModMSensor : public CmdTarget
{
public:
	FtModMSensor(void);
	virtual ~FtModMSensor(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

private:
	FT_MSENSOR_CNF m_ft_cnf;
};

class FtModALSPS : public CmdTarget
{
public:
	FtModALSPS(void);
	virtual ~FtModALSPS(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

private:
	FT_ALSPS_CNF m_ft_cnf;
};

class FtModGyroSensor : public CmdTarget
{
public:
	FtModGyroSensor(void);
	virtual ~FtModGyroSensor(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

};


class FtModModemInfo : public CmdTarget
{
public:
	FtModModemInfo(void);
	virtual ~FtModModemInfo(void);
	int getModemCapability(MODEM_CAPABILITY_LIST_CNF* modem_capa);

public:
	virtual void exec(Frame*);
};


class FtModSIMNum : public CmdTarget
{
public:
	FtModSIMNum(void);
	virtual ~FtModSIMNum(void);

public:
	virtual void exec(Frame*);
};

class FtModSDcard : public CmdTarget
{
public:
	FtModSDcard(void);
	virtual ~FtModSDcard(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

};

class FtModEMMC : public CmdTarget
{
public:
	FtModEMMC(void);
	virtual ~FtModEMMC(void);

public:
	virtual void exec(Frame*);
};

class FtModCRYPTFS : public CmdTarget
{
public:
	FtModCRYPTFS(void);
	virtual ~FtModCRYPTFS(void);

public:
	virtual void exec(Frame*);
};

class FtModADC : public CmdTarget
{
public:
	FtModADC(void);
	virtual ~FtModADC(void);

public:
	virtual void exec(Frame*);
};
#if 0
class FtModAUXADC : public CmdTarget
{
public:
	FtModAUXADC(void);
	virtual ~FtModAUXADC(void);

public:
	virtual void exec(Frame*);

protected:
	virtual void init();
	virtual void deinit();
};



class FtModCPURegR : public CmdTarget
{
public:
	FtModCPURegR(void);
	virtual ~FtModCPURegR(void);

public:
	virtual void exec(Frame*);

protected:
	virtual void init();
	virtual void deinit();
};

class FtModCPURegW : public CmdTarget
{
public:
	FtModCPURegW(void);
	virtual ~FtModCPURegW(void);

public:
	virtual void exec(Frame*);

protected:
	virtual void init();
	virtual void deinit();
};

#endif


class FtModChipID : public CmdTarget
{
public:
	FtModChipID(void);
	virtual ~FtModChipID(void);

public:
	virtual void exec(Frame*);

};

class FtModCTP : public CmdTarget
{
public:
	FtModCTP(void);
	virtual ~FtModCTP(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);

private:
	Touch_CNF m_ft_cnf;
};

class FtModGPIO : public CmdTarget
{
public:
	FtModGPIO(void);
	virtual ~FtModGPIO(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);
};

class FtModDFO : public CmdTarget
{
public:
	FtModDFO(void);
	virtual ~FtModDFO(void);

public:
	virtual void exec(Frame*);

protected:
	int init(Frame*);
};


class FtModSIMDetect : public CmdTarget
{
public:
	FtModSIMDetect(void);
	virtual ~FtModSIMDetect(void);

public:
	virtual void exec(Frame*);

private:
	int GetDevNode();
	unsigned int CheckExtMd();
	unsigned int SetSIMSwitcher(unsigned int &mode);
	unsigned int  GetPrjType(unsigned int &type);
};

class FtModUtility : public CmdTarget
{
public:
	FtModUtility(void);
	virtual ~FtModUtility(void);

public:
	void exec(Frame*);
};


class FtModCustomer : public CmdTarget
{
public:
	FtModCustomer(void);
	virtual ~FtModCustomer(void);

public:
	virtual void exec(Frame*);
};

class FtModSpecialTest : public CmdTarget
{
public:
	FtModSpecialTest(void);
	virtual ~FtModSpecialTest(void);

public:
	virtual void exec(Frame*);
};



#endif	// _FT_MODULE_H_
