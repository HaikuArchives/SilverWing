#ifndef __TRACKERUTILS_H__
#define __TRACKERUTILS_H__

#include <Message.h>
#include <Entry.h>
#include <Messenger.h>
#include <File.h>
#include <Directory.h>

class TrackerUtils {
public:
			void	MoveToTrash(entry_ref file_ref);
			void	OpenFolder(entry_ref folder_ref);
			
		status_t	SmartCreateFile(BFile *file,
									BDirectory *destDir,
									const char* name,
									const char* suffix = " copy ",
									uint32 mode = B_READ_WRITE,
									entry_ref* outref=NULL);

		status_t	SmartMoveFile(entry_ref &ref,
									BDirectory *destDir,
									const char* suffix = " copy ");
			
};
#endif