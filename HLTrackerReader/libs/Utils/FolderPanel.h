#ifndef __FolderPanel_H__
#define __FolderPanel_H__

#include <FilePanel.h>

class FolderPanel :public BFilePanel {
public:
						FolderPanel(file_panel_mode mode = B_OPEN_PANEL
									,BMessenger *target = NULL
									,entry_ref *panel_directory = NULL);
		virtual			~FolderPanel();
protected:
		virtual bool	Filter(const entry_ref *ref
								,BNode *node
								,struct stat *st
								,const char *filetype);		

private:
};
#endif