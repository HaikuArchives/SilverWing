#include <Autolock.h>
#include "HPrefs.h"
#include <santa/Colors.h>


HPrefs::HPrefs(const char* name, const char* dir_name)
	: HSetting(name,dir_name)
{
	MakeDefault();
}


HPrefs::~HPrefs()
{
}

void
HPrefs::MakeDefault()
{
	BAutolock lock(this);
	fDefaultMessage->AddString("server_name","Silverwing server");
	fDefaultMessage->AddString("server_desc","");
	fDefaultMessage->AddBool("mutithread_news",false);
	fDefaultMessage->AddString("address", "127.0.0.1");
	fDefaultMessage->AddInt32("port",5500);
	fDefaultMessage->AddInt32("sim_download",10);
	fDefaultMessage->AddInt32("max_users",20);
	fDefaultMessage->AddBool("save_log",false);
	fDefaultMessage->AddRect("window_rect",BRect(30,30,500,230));
	fDefaultMessage->AddInt32("sim_upload",10);
	fDefaultMessage->AddInt32("encoding",-1);
	fDefaultMessage->AddBool("sound",true);


	_inherited::MakeDefault();
}
