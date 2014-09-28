#ifndef __HTRACKERCONNECTION_H__
#define __HTRACKERCONNECTION_H__

#include "LThread.h"
#include <String.h>
#include <NetworkKit.h>
#include <iostream>

enum{
	H_RECEIVE_SERVER = 'REVS',
	H_END_SEARCH = 'HEDS'
};

class HTrackerConnection :public LThread {
public:
						HTrackerConnection(const char *address,uint16 port,BListItem *parent);
			virtual		~HTrackerConnection();
		virtual int32 	Main();
				int		b_read (BNetEndpoint *endpoint, void *bufp, size_t len);
				void	SJIS2UTF8(char** text);
private:
			BString fAddress;
			uint16 fPort;
			BListItem *fParentItem;
			int32 fEncoding;
			BNetEndpoint *fEndpoint;
};
#endif