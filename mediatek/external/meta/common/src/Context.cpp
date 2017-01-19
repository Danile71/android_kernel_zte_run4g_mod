
#include <DfoDefines.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mlist.h"
#include "Modem.h"
#include "MdRxWatcher.h"

#include "FtModule.h"
#include "Context.h"
#include "LogDefine.h"
#include "hardware/ccci_intf.h"

#define CCCI_ONE_PATH	"/dev/ttyC1"
#define CCCI_TWO_PATH	"/dev/ccci2_tty1"
#define CCCI_FIVE_PATH	"/dev/eemcs_md_log"
#define USB_EXTERNAL_PATH "/dev/ttyACM0"


class Context
{
private:
	Context(void);
public:
	~Context(void);

public:
	static Context *instance();

	Modem * createModem(const char *ccci, unsigned short id);
	Modem * getModem(unsigned short id);

	CmdTarget *getModule(unsigned short id);
	
	unsigned int dumpData(const unsigned char* con, int length);
	unsigned int dumpDataInHexString(const unsigned char* con, int length, unsigned int bytesPerRow);
	
	unsigned int getFileSize(int fd);
	const char* makepath(unsigned char file_ID);

	unsigned getMdmInfo();

	unsigned int getMdmType();
	unsigned int getActiveMdmId();	
	unsigned int getMdmNumber();
	signed int getModemHandle(unsigned short id);
	void createModemThread(unsigned short modemIndex,int usbUsb);
	void setLogLevel(unsigned int level);
	unsigned int getLogLevel();
	
	void destroy();

private:
	void initModuleList();

private:
	mlist<Modem*>		m_mdmList;
	mlist<CmdTarget*>	m_modList;
	SerPort *			m_serPort;

	static Context *	m_myInst;

	unsigned int m_mdmNumber;
	unsigned int m_activeMdmId;
	unsigned int m_mdmType;
	unsigned int m_logLevel;
};

Context *Context::m_myInst = NULL;


Context::Context(void)
	:m_serPort(NULL)
{
	initModuleList();
	META_LOG("[Meta] initModuleList");
	getMdmInfo();
}

Context::~Context(void)
{
	mlist<Modem*>::iterator it0 = m_mdmList.begin();

	while (it0 != m_mdmList.end())
	{
		delete (*it0);
		++ it0;
	}

	mlist<CmdTarget*>::iterator it1 = m_modList.begin();

	while (it1 != m_modList.end())
	{
		delete (*it1);
		++ it1;
	}

	if (m_serPort != NULL)
	{
		delete m_serPort;
	}
}

Context *Context::instance()
{
	return (m_myInst==NULL) ? ((m_myInst=new Context)) : m_myInst;
}

void Context::destroy()
{
	delete m_myInst;
	m_myInst = NULL;
}

Modem * Context::createModem(const char *ccci, unsigned short id)
{
	Modem *p = new Modem(ccci, id);

	if(p!=NULL)
	{
		m_mdmList.push_back(p);
		META_LOG("[Meta] Creat modem%d success.",id+1);
	}
	else
	{
		META_LOG("[Meta] Creat modem%d fail.",id+1);
	}
	return p;
}

CmdTarget * Context::getModule(unsigned short id)
{
	mlist<CmdTarget*>::iterator it = m_modList.begin();

	while (it != m_modList.end())
	{
		META_LOG("[Meta] it->id = %d",(*it)->getId());
		if ((*it)->getId() == id)
		{
			return (*it);
		}
		++ it;
	}
	return NULL;
}

Modem * Context::getModem(unsigned short id)
{
	mlist<Modem*>::iterator it = m_mdmList.begin();

	while (it != m_mdmList.end())
	{
		META_LOG("[Meta] modem it->id = %d",(*it)->getId());
		if ((*it)->getId() == id)
		{
			return (*it);
		}
		++ it;
	}
	return NULL;
}

void Context::initModuleList()
{
		META_LOG("[Meta] Enter initModuleList");

#ifdef FT_WIFI_FEATURE
	m_modList.push_back(new FtModWifi);
#endif

#ifdef FT_GPS_FEATURE
	m_modList.push_back(new FtModGPS);
#endif

#ifdef FT_NFC_FEATURE
	m_modList.push_back(new FtModNFC);
#endif

#ifdef FT_BT_FEATURE
	m_modList.push_back(new FtModBT);
#endif

#ifdef FT_FM_FEATURE
	m_modList.push_back(new FtModFM);
#endif

#ifdef FT_AUDIO_FEATURE
	m_modList.push_back(new FtModAudio);
#endif

#ifdef FT_CCAP_FEATURE
	m_modList.push_back(new FtModCCAP);
#endif

#ifdef FT_MATV_FEATURE
	m_modList.push_back(new FtModMATV);
#endif

#ifdef FT_HDCP_FEATURE
	m_modList.push_back(new FtModHDCP);
#endif

#ifdef FT_DRM_KEY_MNG_FEATURE
	m_modList.push_back(new FtModDRM);
#endif

#ifdef FT_NVRAM_FEATURE
	m_modList.push_back(new FtModNvramBackup);
	m_modList.push_back(new FtModNvramRestore);
	m_modList.push_back(new FtModNvramReset);
	m_modList.push_back(new FtModNvramRead);
	m_modList.push_back(new FtModNvramWrite);
#endif

#ifdef FT_GSENSOR_FEATURE  
	m_modList.push_back(new FtModGSensor);
#endif

#ifdef FT_MSENSOR_FEATURE
	m_modList.push_back(new FtModMSensor);
#endif

#ifdef FT_ALSPS_FEATURE 
	m_modList.push_back(new FtModALSPS);
#endif

#ifdef FT_GYROSCOPE_FEATURE   
	m_modList.push_back(new FtModGyroSensor);
#endif

#ifdef FT_SDCARD_FEATURE	
	m_modList.push_back(new FtModSDcard);
#endif

#ifdef FT_EMMC_FEATURE
	m_modList.push_back(new FtModEMMC);
#endif

#ifdef FT_CRYPTFS_FEATURE
	m_modList.push_back(new FtModCRYPTFS);
#endif

#ifdef FT_ADC_FEATURE
	m_modList.push_back(new FtModADC);
#endif

#ifdef FT_TOUCH_FEATURE
	m_modList.push_back(new FtModCTP);
#endif

#ifdef FT_DFO_FEATURE
	m_modList.push_back(new FtModDFO);
#endif

#ifdef FT_GPIO_FEATURE
	m_modList.push_back(new FtModGPIO);
#endif

	m_modList.push_back(new FtModCustomer);
	m_modList.push_back(new FtModChipID);
	m_modList.push_back(new FtModTestAlive);
	m_modList.push_back(new FtModVersionInfo);
	m_modList.push_back(new FtModVersionInfo2);
	m_modList.push_back(new FtModPowerOff);
	m_modList.push_back(new FtModReboot);
	m_modList.push_back(new FtModBuildProp);
	m_modList.push_back(new FtModModemInfo);
	m_modList.push_back(new FtModSIMNum);
	m_modList.push_back(new FtModUtility);
	m_modList.push_back(new FtModSpecialTest);
	m_modList.push_back(new FtModChipInfo);
	m_modList.push_back(new FtModSIMDetect);
	
}

unsigned int Context::getMdmInfo()
{
	unsigned int modem_number =0;
	unsigned int active_modem_id = 0;
	unsigned int modem_type = 0;
	bool isactive = false;

	if(MTK_ENABLE_MD1)
	{
		modem_type |= 0x01;	
		modem_number++;
		if(!isactive)
		{
			active_modem_id = 1;
			isactive = true;
		}
		META_LOG("[Meta] MTK_ENABLE_MD1 is true");
	}

	if(MTK_ENABLE_MD2)
	{
		modem_type |= 0x02;
		modem_number++;
		if(!isactive)
		{
			active_modem_id = 2;
			isactive = true;
		}
		META_LOG("[Meta] MTK_ENABLE_MD2 is true");
	}

	if(MTK_ENABLE_MD5)
	{
		modem_type |= 0x10;	
		modem_number++;
		if(!isactive)
		{
			active_modem_id = 5;
			isactive = true;
		}
		META_LOG("[Meta] MTK_ENABLE_MD5 is true");
	}

	META_LOG("[Meta] modem_type = %d, modem_number = %d, active_modem_id = %d", modem_type, modem_number, active_modem_id);

	m_mdmType = modem_type;
	m_mdmNumber = modem_number;
	m_activeMdmId = active_modem_id;
	
	return modem_number;		
}


void Context::setLogLevel(unsigned int level)
{
	m_logLevel = level;
}
unsigned int Context::getLogLevel()
{
	return m_logLevel;
}

unsigned int Context::getMdmType()
{
	return m_mdmType;
}


unsigned int Context::getActiveMdmId()
{
	return m_activeMdmId;
}


unsigned int Context::getMdmNumber()
{
	return m_mdmNumber;		
}

signed int Context::getModemHandle(unsigned short id) 
{
	Modem *md = getModem(id);
	if(md != NULL)
		return md->getDevHandle();

	return -1;
}


unsigned int Context::dumpData(const unsigned char* con, int length)
{
	META_LOG("[Meta] Dump data is:  ");
	int i = 0;
	for(i = 0; i < length; i++)
		printf(" (%02x) ",con[i]);
	META_LOG("[Meta] Dump finished!");
	return 0;


}

unsigned int Context::dumpDataInHexString(const unsigned char* con, int length, unsigned int bytesPerRow)
{

	if(getLogLevel())
	{
		int i = 0;
		unsigned int j = 0;
		unsigned int rowLength = 3 * bytesPerRow + 1;
		unsigned char hex[rowLength];
		unsigned char high;
		unsigned char low;
		META_LOG("[Meta] Dump begin!");
		for(i = 0; i < length; i++)
		{
			high = (con[i] >> 4);
			low = (con[i] & 0x0f);
		
			if(high < 0x0a)
         	   high += 0x30;
        	else
         	   high += 0x37;
        
        	if(low < 0x0a)
        	    low += 0x30;
        	else
         	   low += 0x37;
        
        	hex[j++] = high;
        	hex[j++] = low;
        	hex[j++] = ' ';

			if (j == rowLength - 1 || i == length - 1)
			{
				hex[j] = '\0';
				j = 0;
				META_LOG("%s", hex);
			}
		}

		META_LOG("[Meta] Dump finished!");	
	}
	
	return 0;
}


unsigned int Context::getFileSize(int fd)
{
	struct stat file_stat;
	if(fstat(fd, &file_stat) < 0)
	{
		return 0;
	}
	else
	{
		return (unsigned int)file_stat.st_size;
	}
}


const char* Context::makepath(unsigned char file_ID)
{
	if(file_ID == 0)
		return "/data/nvram/AllMap";
	else
	{
		if(file_ID == 1)
			return "/data/nvram/AllFile";
		else
		{
			META_LOG("[Meta] makepath error: invalid file_ID %d! ", file_ID);
			return "";
		}
	}
}

void Context::createModemThread(unsigned short modemIndex, int usbUsb)
{
	Modem *pMdHandle = NULL;	
	MdRxWatcher *pRxWatcher = NULL;
	char dev_node[32] = {0};
	pRxWatcher = new MdRxWatcher(modemIndex);
	if(usbUsb == 1)
	{
		snprintf(dev_node, 32, "%s",USB_EXTERNAL_PATH);
	}
	else 
	{
		snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_META_DATA,(CCCI_MD)modemIndex));
	}
	
	pMdHandle = createModem(dev_node, modemIndex);
	pMdHandle->pumpAsync(pRxWatcher);
}



//////////////////////////////////////////////////////////////////////////


Modem * createModem(const char *ccci, unsigned short id)
{
	return Context::instance()->createModem(ccci, id);
}

CmdTarget * getModule(unsigned short id)
{
	return Context::instance()->getModule(id);
}

Modem * getModem(unsigned short id)
{
	return Context::instance()->getModem(id);
}

unsigned int dumpData(const unsigned char* con, int length)
{
	return Context::instance()->dumpData(con,length);
}

unsigned int dumpDataInHexString(const unsigned char* con, int length, unsigned int bytesPerRow)
{
	return Context::instance()->dumpDataInHexString(con,length,bytesPerRow);
}

unsigned int getFileSize(int fd)
{
	return Context::instance()->getFileSize(fd);
}

const char* makepath(unsigned char file_ID)
{
	return Context::instance()->makepath(file_ID);
}

void destroyContext()
{
	return Context::instance()->destroy();
}

unsigned int getMdmType()
{
	return Context::instance()->getMdmType();
}

unsigned int getActiveMdmId()
{
	return Context::instance()->getActiveMdmId();
}

unsigned int getMdmNumber()
{
	return Context::instance()->getMdmNumber();
}

signed int getModemHandle(unsigned short id)
{
	return Context::instance()->getModemHandle(id);
}

void setLogLevel(unsigned int level)
{
	return Context::instance()->setLogLevel(level);	
}
unsigned int getLogLevel()
{
	return Context::instance()->getLogLevel();	
}

void createModemThread(unsigned short modemIndex,int usbUsb)
{
	return Context::instance()->createModemThread(modemIndex,usbUsb);	
}


