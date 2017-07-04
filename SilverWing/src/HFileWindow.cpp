#include <Autolock.h>
#include <Debug.h>
#include <ClassInfo.h>
#include <Beep.h>
#include "HFileWindow.h"
#include "RectUtils.h"
#include "HFileItem.h"
#include <santa/BetterScrollView.h>
#include <santa/Colors.h>
#include "TextUtils.h"
#include "HApp.h"
#include "HToolbar.h"
#include "HDialog.h"
#include "ResourceUtils.h"
#include "HFileCaption.h"
#include "HToolbarButton.h"
#include "hx_types.h"
#include "HFileList.h"
#include "HotlineClient.h"
#include "MAlert.h"
#include "HTaskWindow.h"
#include "HPrefs.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HFileWindow::HFileWindow(BRect rect,const char *name,bool IsSilverWing)
	:BWindow(rect,name,B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS),fFilePanel(NULL)
{
	fItemIndex = 2;
	InitGUI(IsSilverWing);
	fListView->MakeEmpty();
	fPath = "";
	//this->AddShortcut(B_UP_ARROW,0,new BMessage(FILE_PARENT_FOLDER_MSG));
	this->AddShortcut('/',0,new BMessage(B_ZOOM));
	this->AddShortcut('R',0,new BMessage(FILE_REFRESH_LSIT));
	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 200;
	min_height = 100;
	SetSizeLimits(min_width,max_width,min_height,max_height);
}
/***********************************************************
 * Destructor.
 ***********************************************************/
HFileWindow::~HFileWindow()
{
	delete fFilePanel;
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HFileWindow::InitGUI(bool IsSilverWing)
{
	BRect toolrect = this->Bounds();
	//toolrect.top += (KeyMenuBar()->Bounds()).Height();
	toolrect.bottom = toolrect.top + 30;
	toolrect.right += 2;
	toolrect.left -= 2;
	ResourceUtils utils;
	HToolbar *toolbox = new HToolbar(toolrect,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	//toolbox->AddButton("parentbtn",utils.GetBitmapResource('BBMP',"BMP:PARENT"),new BMessage(FILE_PARENT_FOLDER_MSG),"Parent Folder");
	toolbox->AddButton("folderbtn",utils.GetBitmapResource('BBMP',"BMP:FOLDER"),new BMessage(FILE_MAKE_FOLDER),_("Make folder"));
	toolbox->AddSpace();
	toolbox->AddButton("uploadbtn",utils.GetBitmapResource('BBMP',"BMP:UPLOAD"),new BMessage(FILE_PUT_FILE_MSG),_("Upload files"));
	toolbox->AddButton("downloadbtn",utils.GetBitmapResource('BBMP',"BMP:DOWNLOAD"),new BMessage(FILE_ITEM_CLICK_MSG),_("Download files"));
	toolbox->AddSpace();
	toolbox->AddButton("refreshbtn",utils.GetBitmapResource('BBMP',"BMP:REFRESH"),new BMessage(FILE_REFRESH_LSIT),_("Refresh file list"));
	toolbox->AddButton("infobtn",utils.GetBitmapResource('BBMP',"BMP:INFO"),new BMessage(FILE_INFO_MESSAGE),_("Get file infomation"));
	toolbox->AddSpace();
	toolbox->AddButton("trashbtn",utils.GetBitmapResource('BBMP',"BMP:TRASH"),new BMessage(FILE_DELETE_FILE),_("Delete"));
	this->AddChild(toolbox);
//******* ListView ***********************/
	BRect textrect = Bounds();
	textrect.top += 30;
	textrect.right -= B_V_SCROLL_BAR_WIDTH;
	textrect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	BetterScrollView *scroller;
	fListView = new HFileList(textrect,&scroller,"agreeview",IsSilverWing);
	fListView->SetSortKey(0);

	fListView->SetInvocationMessage(new BMessage(FILE_ITEM_CLICK_MSG));
	this->AddChild(scroller);

	const int32 CAPTION_WIDTH = 90;
	scroller->ScrollBar(B_HORIZONTAL)->ResizeBy(-CAPTION_WIDTH,0);
	scroller->ScrollBar(B_HORIZONTAL)->MoveBy(CAPTION_WIDTH,0);

/********** Caption ***********/
	BRect captionframe = scroller->Bounds();
	captionframe.top = captionframe.bottom - B_H_SCROLL_BAR_HEIGHT;
	captionframe.right = CAPTION_WIDTH+1;
	BBox *bbox = new BBox(captionframe,NULL,B_FOLLOW_BOTTOM);
	captionframe.OffsetTo(B_ORIGIN);
	captionframe.top += 2;
	captionframe.bottom -= 2;
	captionframe.right -= 2;
	captionframe.left += 2;
	HFileCaption *caption = new HFileCaption(captionframe,"caption",fListView);
	bbox->AddChild(caption);
	scroller->AddChild(bbox);
}

/***********************************************************
 * Insert file item.
 ***********************************************************/
void
HFileWindow::InsertFileItem(const char* name,
					const char* type,
					const char* creator,
					uint32 size,
					bool isFolder,
					uint32 index,
					uint32 modified)
{
	BAutolock lock(this);
	//printf("index: %d type: %s\n",index,type);
	if(index == 1)
		fListView->AddFileItem(new HFileItem(name,type,creator,size,fItemIndex++,modified,isFolder));
	else{
		HFileItem *item = fListView->FindItem(index);
		if(item != NULL)
		{
			fListView->AddFileItemUnder(new HFileItem(name,type,creator,size,fItemIndex++,modified,isFolder),
				item);
		}
	}
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HFileWindow::MessageReceived(BMessage *message)
{
	//message->PrintToStream();
	switch(message->what)
	{
	/********** Recv File List *****************/
	case H_FILELIST_RECEIVED:
	{
		BAutolock lock(this);
		if(lock.IsLocked())
		{
			int32 count;
			type_code type;

			//fListView->RemoveAll();
			message->GetInfo("name",&type,&count);
			uint32 index;
			if( message->FindInt32("index",(int32*)&index) != B_OK)
				index = 0;
			// remove sub items
			if(index != 0)
			{
				HFileItem *item = fListView->FindItem(index);
				if(item != NULL)
					fListView->RemoveChildItems(item);
			}
			for(register int32 i = 0;i < count;i++)
			{
				const char* name = message->FindString("name",i);
				const char* dtype = message->FindString("type",i);
				const char* creator = message->FindString("creator",i);
				uint32 modified;
				if(message->FindInt32("modified",i,(int32*)&modified) != B_OK)
					modified = 0;
				uint32 size = message->FindInt32("size",i);
				bool isFolder = false;
				if(::strncmp(dtype,"fldr",4) == 0)
					isFolder = true;
				this->InsertFileItem(name,dtype,creator,size,isFolder,index,modified);
			}
			fListView->SortItems();
			StartBarberPole(false);
		}
		break;
	}
	// Make Folder
	case FILE_MAKE_FOLDER:
	{
		BMessage *msg = new BMessage(H_FILE_CREATE_FOLDER);
		int32 sel = fListView->CurrentSelection();
		BString path = "";
		if(sel >= 0)
		{
			HFileItem *item = cast_as( fListView->ItemAt(sel),HFileItem);
			fListView->GetItemPath(item,path);
		}
		msg->AddString("path",path.String());
		HDialog *dlg = new HDialog( RectUtils().CenterRect(250,80),_("Create folder"),msg,_("Folder name:"),_("Create"));
		dlg->Show();
		break;
	}
	// ListItem Clicked
	case FILE_ITEM_CLICK_MSG:
	{
		BAutolock lock(this);
		if( lock.IsLocked())
		{
			if(fListView->CurrentSelection() >= 0)
			{
				int32 sel = fListView->CurrentSelection();
				if(sel < 0)
					break;
				HFileItem *item = cast_as(fListView->ItemAt(sel),HFileItem);
				if(item == NULL)
					break;
				if(!(item->isFolder()) )
				{
					BString alert_title = _("Filename:");
					alert_title += " ";
					alert_title<< item->DecodedName();

					int32 btn = (new MAlert(_("Would you like to download it?"),alert_title.String(),_("Cancel"),_("OK")))->Go();
					if(btn == 1)
					{
						/*BString remotepath = fPath;
						if( remotepath.Length() > 0 )
							remotepath << dir_char;
						remotepath << item->Name();
						*/
						BString remotepath;
						fListView->GetItemPath(item,remotepath);
						const char* download_path;
						((HApp*)be_app)->Prefs()->GetData("download_path",&download_path);
						BPath path(download_path);
						path.Append(item->Name());
						this->SendDownload(remotepath.String(),path.Path());
					}
				}else{
					if(item->IsExpanded())
						fListView->Collapse(item);
					else
						fListView->Expand(item);
				}
			}
		}
		break;
	}
	// ParentBtn Clicked
	case FILE_PARENT_FOLDER_MSG:
	{
		fListView->RemoveAll();
		int index = fPath.FindLast(dir_char);
		if(index >=0)
		{
			BString tmp;
			fPath.CopyInto(tmp,0,index);
			fPath = tmp;
			BMessage msg(FILE_FILE_REQUEST);
			msg.AddString("path",fPath.String());
			be_app->PostMessage(&msg);
			PRINT(("Send: %s\n" , fPath.String() ));
		}else{
			BMessage msg(FILE_FILE_REQUEST);
			msg.AddString("path","");
			be_app->PostMessage(&msg);
			PRINT(( "Send: %s\n" , fPath.String() ));
			fPath = "";
		}
		break;
	}
	// Filelist Refresh
	case FILE_REFRESH_LSIT:
	{
		fListView->RemoveAll();
		BMessage msg(FILE_FILE_REQUEST);
		msg.AddString("path","");
		msg.AddInt32("index",1);
		be_app->PostMessage(&msg);
		StartBarberPole(true);
		break;
	}
	case FILE_INFO_MESSAGE:
	{
		int32 sel = fListView->CurrentSelection();
		if(sel >= 0)
		{
		HFileItem *item = cast_as(fListView->ItemAt(sel),HFileItem);
		if(item->isFolder() != true)
		{
			BString path;/* = fPath;
			if(path.Length() > 0)
				path << dir_char;
			path << item->Name();
			*/
			fListView->GetItemPath(item,path);
			BMessage msg(H_FILE_GET_INFO);
			msg.AddString("path",path.String());
			((HApp*)be_app)->Client()->PostMessage(&msg);
		}
		}
		break;
	}
	// ツールバーのUpdate
	case M_UPDATE_TOOLBUTTON:
	{
		const char* name = message->FindString("name");
		int32 sel = fListView->CurrentSelection();
		void *pointer;
		message->FindPointer("pointer",&pointer);
		HToolbarButton *btn = static_cast<HToolbarButton*>(pointer);
		if(::strcmp(name,"uploadbtn") == 0 || ::strcmp(name,"folderbtn") == 0)
		{
			if(sel <0)
			{
				btn->SetEnabled(true);
				break;
			}else{
				HFileItem *item = cast_as( fListView->ItemAt(sel),HFileItem );
				if(item->isFolder())
					btn->SetEnabled(true);
				else
					btn->SetEnabled(false);
			}
		}else if(::strcmp(name,"downloadbtn") == 0 ||
			::strcmp(name,"infobtn") == 0){
			if(sel <0)
			{
				btn->SetEnabled(false);
				break;
			}else{
				HFileItem *item = cast_as( fListView->ItemAt(sel),HFileItem );
				if(item->isFolder())
					btn->SetEnabled(false);
				else
					btn->SetEnabled(true);
			}
		}else if( ::strcmp(name,"trashbtn") == 0) {
			if(sel < 0)
				btn->SetEnabled(false);
			else
				btn->SetEnabled(true);
		}
		break;
	}
	//******* TrackerからのDrop　****************//
	case 'DATA':
	{
		entry_ref ref;
		type_code type;
		int32 count;
		message->GetInfo("refs",&type,&count);
		//message->PrintToStream();
		if(count != 0)
		{
			for(register int i = 0;i<count;i++)
			{
			message->FindRef("refs",i,&ref);
			BEntry entry(&ref);
			BPath path(&entry);
			/* remotepathとlocalpathの生成
			BString tmppath = "";
			char *tmp = new char[strlen(path.Leaf())+1];
			strcpy(tmp,path.Leaf());

			int32 encoding;
			((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
			if(encoding)
				TextUtils().ConvertFromUTF8(&tmp,encoding-1);
			tmppath = tmp;
			delete[] tmp;*/
			BString localpath = path.Path();
			//BString remotepath = fPath;
			BString remotepath;
			HFileItem *parent = NULL;
			bool resume = false;
			int32 sel = fListView->CurrentSelection();
			if(sel < 0)
				remotepath = "";
			else{
				parent = cast_as( fListView->ItemAt(sel),HFileItem );
				fListView->GetItemPath(parent,remotepath);
			}
			resume = fListView->FindSameItem(path.Leaf(),parent);

			if(remotepath.Length() > 0)
				remotepath << dir_char;
			remotepath << path.Leaf();
			PRINT(("RemotePath:%s\n", remotepath.String() ));

			SendUpload(remotepath.String(),localpath.String(),resume);
			}
		}else{ // file move.
			//message->PrintToStream();
			BPoint drop_point;
			int32 dragged_item_index;
			message->FindPoint("_drop_point_",&drop_point);
			message->FindInt32("dragged_item",&dragged_item_index);
			fListView->ConvertFromScreen(&drop_point);

			HFileItem *dest = fListView->FindItem(drop_point);
			HFileItem *item = fListView->FindItem(dragged_item_index);
			if(item != NULL)
			{
				if(item == dest)
					break;
				int32 btn = (new MAlert("Would you like to move it?",item->Name(),"Cancel","OK"))->Go();
				if(btn == 1)
				{
				BString file_path = "";
				BString dest_path = "";
				fListView->GetItemPath(item,file_path);
				if( file_path.Compare(item->Name()) == 0)
					file_path = "";
				else{
					int32 i = file_path.FindLast(dir_char);
					file_path.Truncate(i);
					file_path << dir_char;
				}
				if(dest != NULL && dest->isFolder())
					fListView->GetItemPath(dest,dest_path);
				dest_path << dir_char;
				SendMove(item->Name(),file_path.String(),dest_path.String());
				}
			}else{
				PRINT(("item is NULL\n"));
			}
		}
		break;
	}
	case FILE_DELETE_FILE:
	{
		int32 sel = fListView->CurrentSelection();
		if(sel >= 0)
		{
			if( (new MAlert("Caution!","Would you like to delete it?","OK","Cancel",NULL,B_STOP_ALERT))->Go() == 0)
			{
				HFileItem *item = cast_as(fListView->ItemAt(sel),HFileItem);
				if(item == NULL)
					break;
				BString path;

				fListView->GetItemPath(item,path);
				BMessage msg(H_FILE_DELETE);
				msg.AddString("path",path.String());
				((HApp*)be_app)->Client()->PostMessage(&msg);
			}
		}
		break;
	}
	case FILE_PUT_FILE_MSG:
	{
		if(fFilePanel == NULL)
		{
			fFilePanel = new BFilePanel();
		}
		fFilePanel->SetTarget(this);
		fFilePanel->Show();
		break;
	}
	case B_REFS_RECEIVED:
	{
		entry_ref ref;
    	int32 count;
    	type_code type;

    	message->GetInfo( "refs", &type, &count );
  	 	if ( type != B_REF_TYPE )
    	{
    		beep();
        	return;
  	   	}
		message->FindRef( "refs", &ref );
		BEntry entry(&ref);
		BPath path(&entry);
     	//BString remotepath = fPath;
     	BString remotepath;
     	int32 sel = fListView->CurrentSelection();
     	HFileItem *parent = NULL;

     	if(sel < 0)
     		remotepath = "";
     	else{
     		parent = cast_as( fListView->ItemAt(sel),HFileItem);
     		if(parent == NULL)
     			break;
     		fListView->GetItemPath(parent,remotepath);
     	}
     	bool resume = fListView->FindSameItem(path.Leaf(),parent);
     	if(remotepath.Length() > 0)
			remotepath << dir_char;
     	remotepath << path.Leaf();
     	this->SendUpload(remotepath.String(),path.Path(),resume);
		break;
	}
	case 'MNOT':
	{
		BPoint drop_point = message->FindPoint("_old_drop_point_");
		BRect rect = this->Frame();
		if(rect.Contains(drop_point) == true)
			break;
		BMessage msg,reply;
		//message->PrintToStream();
		msg.what = B_GET_PROPERTY;
		msg.AddSpecifier("Path");
		if(message->SendReply(&msg,&reply) != B_OK)
			break;
		entry_ref ref;
		if(reply.FindRef("result",&ref) == B_OK)
		{
			BEntry entry(&ref);
			BPath path(&entry);
			PRINT(("Path:%s\n",path.Path() ));
			int32 count = fListView->FullListCountItems();
			for(register int i =0;i< count;i++)
			{
				HFileItem *item = cast_as(fListView->ItemAt(i),HFileItem);
				if(item == NULL)
					continue;
				if( item->IsSelected() == true)
				{
					if( item->isFolder() == false)
					{
						BString name = item->Name();

						path.Append(name.String());

						BString remotepath;/* = this->fPath;
						if(remotepath.Length() > 0)
							remotepath << dir_char;
						remotepath << name;
						*/
						fListView->GetItemPath(item ,remotepath);
						PRINT(("RemotePath:%s\n", remotepath.String() ));
						this->SendDownload(remotepath.String(),path.Path());
						path.GetParent(&path);
						break;
					}
				}
			}
		}// end if
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * QuitRequested
 ***********************************************************/
bool
HFileWindow::QuitRequested()
{
	fListView->RemoveAll();
	CLVColumn *col = fListView->ColumnAt(2);
	((HApp*)be_app)->Prefs()->SetData("file_column_width",col->Width());
	((HApp*)be_app)->SaveRect("file_rect",this->Frame());
	fTarget->PostMessage(FILE_REMOVE_POINTER);
	return true;
}

/***********************************************************
 * Send upload message to server.
 ***********************************************************/
void
HFileWindow::SendUpload(const char* remotepath,const char* localpath,bool is_resume)
{
	BString rPath(remotepath);
	int index = rPath.FindLast(dir_char,rPath.Length()-2);

	BString str;
	if(index != B_ERROR)
	{
		rPath.CopyInto(str,index,rPath.Length()-index+1);
		rPath.Truncate(index);
	}else{
		str = rPath;
		rPath = "";
	}

	TextUtils utils;

	char *tmp = new char[ str.Length() +1];
	::strcpy(tmp,str.String());
	int32 encoding;
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertFromUTF8(&tmp,encoding-1);
	rPath << tmp;

	delete[] tmp;

	uint16 resume = 0;
	if(is_resume)
		resume = 1;

	BMessage msg(FILE_PUT_FILE_MSG);
	msg.AddString("remotepath",rPath);
	msg.AddString("localpath",localpath);
	msg.AddInt16("resume",resume);
	((HApp*)be_app)->Client()->PostMessage(&msg);
}

/***********************************************************
 * Send download message to server.
 ***********************************************************/
void
HFileWindow::SendDownload(const char* remotepath,const char* localpath)
{
	off_t size =0;
	BFile file;
	if(file.SetTo(localpath,B_READ_ONLY) == B_OK)
	{
		file.GetSize(&size);
	}
	uint32 usize = size;
	BMessage msg(FILE_GET_FILE_MSG);
	msg.AddString("remotepath",remotepath);
	msg.AddString("localpath",localpath);
	msg.AddInt32("data_size",usize);
	((HApp*)be_app)->Client()->PostMessage(&msg);
}

/***********************************************************
 * Send get all subitems message to server.
 ***********************************************************/
void
HFileWindow::GetSubItems(HFileItem *item)
{
	BString path="";
	uint32 index = item->ItemIndex();
	fListView->GetItemPath(item,path);
	PRINT(("ItemPath:%s\n", path.String() ));

	BMessage msg(FILE_FILE_REQUEST);
	msg.AddString("path",path.String());
	msg.AddInt32("index",index);
	be_app->PostMessage(&msg);
	StartBarberPole(true);
}

/***********************************************************
 *
 ***********************************************************/
void
HFileWindow::SendMove(const char* filename,const char* path,
					const char* dest_path)
{
	BMessage msg(H_FILE_MOVE);
	msg.AddString("file_name",filename);
	msg.AddString("file_path",path);
	msg.AddString("dest_path",dest_path);
	((HApp*)be_app)->Client()->PostMessage(&msg);
}

/***********************************************************
 * Start barber pole
 ***********************************************************/
void
HFileWindow::StartBarberPole(bool start)
{
	HFileCaption *caption = cast_as(FindView("caption"),HFileCaption);

	if(start)
		caption->StartBarberPole();
	else
		caption->StopBarberPole();
}
