
#ifndef _PORTINTERFACE_H_
#define _PORTINTERFACE_H_


class SerPort;

void destroyPortHandle();
META_COM_TYPE getComType();
SerPort * createSerPort();
SerPort * getSerPort();
void querySerPortStatus();


#endif	// _PORTINTERFACE_H_


