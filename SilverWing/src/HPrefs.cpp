#include "HPrefs.h"
#include <Autolock.h>
#include <Font.h>
#include "AppUtils.h"

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
	fDefaultMessage->AddString("nick", "unnamed");
	fDefaultMessage->AddInt32("icon",137);
	fDefaultMessage->AddBool("showlogin",false);
	fDefaultMessage->AddBool("preload",false);
	fDefaultMessage->AddBool("sound",true);
	fDefaultMessage->AddBool("refusechat",false);
	BFont font(be_fixed_font);
	font_family family;
	font_style style;
	font.GetFamilyAndStyle(&family,&style);
	fDefaultMessage->AddString("font_family",family);
	fDefaultMessage->AddString("font_style",style);
	fDefaultMessage->AddInt32("font_size",12);
	fDefaultMessage->AddInt32("encoding",0);
//************************* Firewall ***************//
	fDefaultMessage->AddBool("sock5",false);
	fDefaultMessage->AddBool("auth",false);
	fDefaultMessage->AddString("firewall","");
	fDefaultMessage->AddInt32("firewall_port",1080);
	fDefaultMessage->AddString("firewall_user","");
	fDefaultMessage->AddString("firewall_password","");
//**************************************************//
	rgb_color bgcolor = {0,0,0,0};
	uint32 indexColor = bgcolor.red << 16|bgcolor.green << 8|bgcolor.blue;
	fDefaultMessage->AddInt32("back_color",indexColor);
	rgb_color color = {0,255,0,0};
	indexColor = color.red << 16|color.green << 8|color.blue;
	fDefaultMessage->AddInt32("font_color",indexColor);
	
	BRect rect;
	rect.Set(50,50,500,350);
	fDefaultMessage->AddRect("window_rect", rect);
	rect.Set(50,50,300,200);
	fDefaultMessage->AddRect("task_rect", rect);
	rect.Set(30,30,300,400);
	fDefaultMessage->AddRect("news15_rect",rect);
	rect.Set(30,30,450,450);
	fDefaultMessage->AddRect("news_rect",rect);
	rect.Set(100,100,400,300);
	fDefaultMessage->AddRect("file_rect",rect);
	
	fDefaultMessage->AddFloat("file_column_width",140);
	fDefaultMessage->AddInt32("news_hbar_pos",150);
	fDefaultMessage->AddInt32("news_vbar_pos",150);
	fDefaultMessage->AddInt32("main_vbar_pos",300);
	
	//fDefaultMessage->AddBool("queue_download",true);
	fDefaultMessage->AddInt32("prvchat_vbar_pos",300);
	fDefaultMessage->AddString("language","English");
	
	rgb_color nick = {255,0,0,0};
	indexColor = nick.red << 16|nick.green << 8|nick.blue;
	fDefaultMessage->AddInt32("nick_color",indexColor);
	fDefaultMessage->AddBool("timestamp",false);
	fDefaultMessage->AddBool("taskiconfy",false);
	fDefaultMessage->AddBool("keep_alive",false);
	fDefaultMessage->AddInt32("interval",10);
	fDefaultMessage->AddInt32("main_hbar_pos",170);
	rect.Set(200,200,200+500,200+250);
	fDefaultMessage->AddRect("prvchat_window_rect",rect);
	fDefaultMessage->AddInt32("prvchat_hbar_pos",200);
	
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Downloads");
	fDefaultMessage->AddString("download_path",path.Path());

	rgb_color url_color = {0,0,255,0};
	indexColor = url_color.red << 16|url_color.green << 8|url_color.blue;
	fDefaultMessage->AddInt32("url_color",indexColor);
	fDefaultMessage->AddBool("msgchat",true);
	fDefaultMessage->AddBool("single_window",false);
	
	HSetting::MakeDefault();
}