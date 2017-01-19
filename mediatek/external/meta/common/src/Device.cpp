#include <assert.h>
#include <unistd.h>




#include "Context.h"
#include "Device.h"
#include "LogDefine.h"


IDevWatcher::~IDevWatcher(void){}

Device::Device(void)
	: m_fd(NULL_FILE_DESCRIPTOR), m_pWatcher(NULL)
{
	m_wMutex = PTHREAD_MUTEX_INITIALIZER;
}

Device::~Device(void)
{
	if (m_fd != NULL_FILE_DESCRIPTOR)
	{
		close(m_fd);
	}
}

signed int Device::read(unsigned char *buf, unsigned int len)
{
	 int tmpLen = ::read(m_fd, buf, len);	

	 //META_LOG("[Meta] read data from device: len =%d , m_fd = %d", tmpLen, m_fd);
	 return tmpLen;
}

signed int Device::write(const unsigned char *p, unsigned int len)
{
    int bytes_written = -1;
	int remain_size = len;
	pthread_mutex_lock(&m_wMutex);
	while(remain_size > 0)
	{
		bytes_written = ::write(m_fd, p, remain_size);
		if (bytes_written < 0) 
		{
			 META_LOG("[Meta] write data to device failed, return %d, errno=%d, m_fd=%d", bytes_written, errno, m_fd);
			 pthread_mutex_unlock(&m_wMutex);
			 return bytes_written;
		}
		else
		{
			META_LOG("[Meta] write %d bytes to device: m_fd = %d, ", bytes_written, m_fd);
		}
		remain_size -= bytes_written;
		p += bytes_written;
	}
	pthread_mutex_unlock(&m_wMutex);
	return (len - remain_size);
}

signed int Device::pump(IDevWatcher *p)
{
	unsigned char buf[4*1024]={0};
	int len = 0;

	assert (p != NULL);

	while (true)
	{
		len = read(buf, sizeof(buf));

		if (len > 0)
		{
			p->onReceived(buf, len);
		}
	}
	return 0;
}

signed int Device::pumpAsync(IDevWatcher *p)
{
	m_pWatcher = p;
	pthread_create(&m_thread, NULL, ThreadProc, this);
	return 0;
}

void *Device::ThreadProc(void *p)
{
	Device *inst = (Device*)p;
	inst->pump(inst->m_pWatcher);
	return NULL;
}

void Device::update()
{
}




