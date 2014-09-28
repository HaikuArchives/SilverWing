#ifndef __HAPP_H__
#define __HAPP_H__
/*********** SYSTEM HEADER ***********/
#include <Application.h>
#include <Resources.h>
#include <String.h>

class HTaskWindow;
class HWindow;
class HotlineClient;
class HPrefs;
class HFileWindow;
class HLDat;
enum{
SOUND_CHAT_SND = 'SDCH',
SOUND_LOGGEDIN_SND = 'SDLD',
SOUND_LOGIN_SND = 'SDLI',
SOUND_LOGOUT_SND = 'SDLO',
SOUND_INVITE_SND = 'SDIV',
SOUND_SRVMSG_SND = 'SDSM',
SOUND_POST_NEWS_SND = 'SDPN',
SOUND_FILE_DOWN_SND = 'SDDN',
SOUND_NOTIFY_SND = 'SDNT',
M_RESET_RESOURCE = 'MRES'
};


#define _(String) ((HApp*)be_app)->reserved(String)
//#define __HOTLINE_DAT__

class HApp: public BApplication {
public:
				HApp();
virtual			~HApp();
virtual void	MessageReceived(BMessage *message);
		void	Download(const char* address,uint32 port);
		bool	IsConnected() const;
HTaskWindow*	TaskWindow() const {return fTaskWindow;}
HWindow*		MainWindow() const {return fMainWindow;}	

		uint32	ServerVersion() const {return fServerVersion;}
HotlineClient*	Client()const {return client;}
		void	PlaySound(uint32 type);
/******** Setting **********/
const char*		Nick() const;
	int32		Icon() const;
	void		CloseResource();
	void		LoadRect(const char* name,BRect *rect);
	void		SaveRect(const char* name,BRect rect);
	void		SetNick(const char* nick);
	void		SetIcon(int32 icon);
	BBitmap*	GetIcon(int16 num);
	HPrefs*		Prefs() const {return fPrefs;}
	status_t	GetMovie(const char* name,BPath &path);
	void		EnableSound(bool which) {fEnableSound = which;}
	BResources* IconResource() {return fIconResource;}
	bool		IsBadmoon() const {return fIsBadmoon;}
	HLDat*		HotlineDat() {return fHLDat;}
	const char*	reserved(const char* text){return text;} 
/************ PROTECTED ************/
protected:
	void		OpenResource();

	void		AddSoundEvents();
	void		InitMimes();
virtual bool	QuitRequested();
virtual void	AboutRequested();
virtual void	ReadyToRun();
virtual void	RefsReceived(BMessage* message);
/*********** PRIVATE *************/
private:
		HTaskWindow			*fTaskWindow;
		HWindow				*fMainWindow;
		/******** Hotline socket *********/
		HotlineClient		*client;
		/******** login info ***********/
		BString 			fLogin;
		BString				fAddress;
		BString				fPassword;
		/******** private chat window list *********/
		BList				prvchatlist;
		uint32				fServerVersion;
		/******** setting files **********/
		HPrefs				*fPrefs;
		/******** icon resources *******/
		BResources			*fIconResource;
		bool				fEnableSound;
		/******** path to sound file directory ********/
		BString				fSoundPath;
		HLDat				*fHLDat;
		bool				fIsBadmoon;
};
#endif