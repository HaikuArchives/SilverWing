#ifndef __HPRVCHAT_H__
#define __HPRVCHAT_H__

#include <List.h>
#include <iostream>
#include <String.h>

class TServer;
class HLPacket;
class HUserItem;

class HPrvChat :public BLocker{
public:
				HPrvChat(uint32 pcref,TServer* server);
	virtual		~HPrvChat();	

		void	AddClient(HUserItem *user,bool send_to_all = false);
		void	RemoveClient(HUserItem *user);
		void	RemovePrvChat(HPrvChat *chat);
		uint32 	Pcref() const {return fPcref;}
		uint32  Users() const;
		bool	FindUser(uint32 sock);
		void	SendPrvUserList(HUserItem *user,uint32 trans);
		void	SendToAllUsers(HLPacket &data,uint32 type,uint32 data_size,uint32 hc);
		void	SetTopic(const char* topic);
	const char* Topic() {return fTopic.String();}

protected:
		void	SendTopicChanged();
private:
	TServer* fServer;
	BString	fTopic;
	BList	fUserList;
	uint32	fPcref;
};
#endif