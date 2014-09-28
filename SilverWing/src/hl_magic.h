#ifndef __HL_MAGIC_H__
#define __HL_MAGIC_H__

#define HTLC_MAGIC      "TRTPHOTL\0\1\0\2"
#define HTLC_MAGIC_LEN  12

#define HTLS_MAGIC      "TRTP\0\0\0\0"
#define HTLS_MAGIC_LEN  8

#define HTLS_TCPPORT    5500

#define HTXF_MAGIC_INT     0x48545846
#define HTRK_MAGIC	"HTRK\0\1"
#define HTRK_MAGIC_LEN	6
#define HTXF_MAGIC	"HTXF"
#define HTXF_MAGIC_LEN	4
#define HTXF_MAGIC_INT	0x48545846

#define HTRK_TCPPORT	5498
#define HTRK_UDPPORT	5499
#define HTLS_TCPPORT	5500
#define HTXF_TCPPORT	5501

/********************************/
/* codes utilises par le client */
/********************************/

/* code de header */
#define HTLC_HDR_LOGIN			 	((uint32) 0x0000006b)		/* done */
#define HTLC_HDR_USER_GETLIST		((uint32) 0x0000012c)		/* done */
#define HTLC_HDR_NEWS_GETFILE		((uint32) 0x00000065)		/* done */
#define HTLC_HDR_FILE_LIST			((uint32) 0x000000c8)		/* done */
#define HTLC_HDR_FILE_GET		 	((uint32) 0x000000ca)		/* done */
#define HTLC_HDR_FILE_PUT		 	((uint32) 0x000000cb)		/* done */
#define HTLC_HDR_FILE_MKDIR     ((uint32) 0x000000cd)			/* not checked */
#define HTLC_HDR_MSG					((uint32) 0x0000006c)		/* done */
#define HTLC_HDR_USER_CHANGE	 	((uint32) 0x00000130)		/* done */
#define HTLC_HDR_FILE_GETINFO		((uint32) 0x000000ce)		/* done */
#define HTLC_HDR_FILE_DELETE		((uint32) 0x000000cc)		/* not checked */
#define HTLC_HDR_NEWS_POST      	((uint32) 0x00000067)		/* done */
#define HTLC_HDR_CHAT_CREATE    	((uint32) 0x00000070)		/* done */
#define HTLC_HDR_CHAT           	((uint32) 0x00000069)		/* done */
#define HTLC_HDR_CHAT_LEAVE     	((uint32) 0x00000074)		/* done */
#define HTLC_HDR_CHAT_SUBJECT   	((uint32) 0x00000078)		/* done */
#define HTLC_HDR_CHAT_JOIN      	((uint32) 0x00000073)		/* done */
#define HTLC_HDR_CHAT_INVITE    ((uint32) 0x00000071)

#define HTLC_HDR_USER_KICK      ((uint32) 0x0000006e)
#define HTLC_HDR_USER_GETINFO   ((uint32) 0x0000012f)
#define HTLC_HDR_USER_CREATE    ((uint32) 0x0000015e)
#define HTLC_HDR_USER_OPEN      ((uint32) 0x00000160)
#define HTLC_HDR_FILE_SETINFO   ((uint32) 0x000000cf)
#define HTLC_HDR_FILE_MOVE      ((uint32) 0x000000d0)
#define HTLC_HDR_CHAT_DECLINE   ((uint32) 0x00000072)
#define HTLC_HDR_NEWSDIRLIST	((uint32) 0x00000172)
#define HTLC_HDR_NEWSCATLIST	((uint32) 0x00000173)
#define HTLC_HDR_DELNEWSDIRCAT	((uint32) 0x0000017C)
#define HTLC_HDR_MAKENEWSDIR	((uint32) 0x0000017D)
#define HTLC_HDR_MAKECATEGORY	((uint32) 0x0000017E)
#define HTLC_HDR_GETTHREAD		((uint32) 0x00000190)
#define HTLC_HDR_POSTTHREAD		((uint32) 0x0000019A)
#define HTLC_HDR_DELETETHREAD	((uint32) 0x0000019B)
#define HTLC_HDR_SILVERWING_MODE	((uint32) 0x10000001)

/* code de chunk de donnees */
#define HTLC_DATA_NICK			 	((uint16) 0x0066)				/* supported */
#define HTLC_DATA_ICON			 	((uint16) 0x0068)				/* supported */
#define HTLC_DATA_LOGIN				((uint16) 0x0069)				/* supported */
#define HTLC_DATA_PASSWORD			((uint16) 0x006a)				/* supported */
#define HTLC_DATA_DIR			  	((uint16) 0x00ca)				/* supported */
#define HTLC_DATA_FILE			 	((uint16) 0x00c9)				/* supported */
#define HTLC_DATA_RESUMEINFO		((uint16) 0x00cb)
#define HTLC_DATA_RESUMEFLAG		((uint16) 0x00cc)				/* supported (file transfert extra data) */
#define HTLC_DATA_SOCKET		  	((uint16) 0x0067)				/* supported */
#define HTLC_DATA_MSG			  	((uint16) 0x0065)				/* supported */
#define HTLC_DATA_NEWS_POST     	((uint16) 0x0065)				/* supported */
#define HTLC_DATA_CHAT          	((uint16) 0x0065)				/* supported */
#define HTLC_DATA_CHAT_REF      	((uint16) 0x0072)				/* supported */
#define HTLC_DATA_CHAT_SUBJECT  	((uint16) 0x0073)				/* supported */
#define HTLC_DATA_NEWS				((uint16) 0x0145)                /* supported by me */

#define HTLC_DATA_HTXF_SIZE     ((uint16) 0x006c)
#define HTLC_DATA_OPTION        ((uint16) 0x006d)
#define HTLC_DATA_FILE          ((uint16) 0x00c9)
#define HTLC_DATA_BAN           ((uint16) 0x0071)
#define HTLC_DATA_FILE_RENAME   ((uint16) 0x00d3)
#define HTLC_DATA_DIR_RENAME    ((uint16) 0x00d4)
#define HTLC_DATA_CHAT_SUBJECT		((uint16) 0x0073)


/*********************************/
/* codes utilises par le serveur */
/*********************************/

/* code de header */
#define HTLS_HDR_TASK						((uint32) 0x00010000)	/* supported / partial */
#define HTLS_HDR_AGREEMENT					((uint32) 0x0000006d)	/* supported */
#define HTLS_HDR_USER_LEAVE				((uint32) 0x0000012e)	/* supported */
#define HTLS_HDR_USER_CHANGE				((uint32) 0x0000012d)	/* supported */
#define HTLS_HDR_NEWS_POST					((uint32) 0x00000066)	/* supported */
#define HTLS_HDR_MSG							((uint32) 0x00000068)	/* supported (private message) */
#define HTLS_HDR_CHAT						((uint32) 0x0000006a)	/* not supported */
#define HTLS_HDR_CHAT_INVITE				((uint32) 0x00000071)	/* --- under test, not supported */
#define HTLS_HDR_CHAT_SUBJECT				((uint32) 0x00000077)	/* supported */
#define HTLS_HDR_CHAT_USER_CHANGE		((uint32) 0x00000075)	/* supported */
#define HTLS_HDR_CHAT_USER_LEAVE			((uint32) 0x00000076)	/* supported */
#define HTLS_HDR_USER_SELFINFO					((uint32) 0x00000162)   /* unsupported */
#define HTLS_HDR_POLITEQUIT             ((uint32) 0x0000006f)
#define HLTS_HDR_SERVERQUEUE			((uint32) 0x000000d3)
#define HTLS_HDR_MSG_BROADCAST			((uint32) 0x00000163)
#define HTLS_HDR_SILVERWING_MODE			((uint32) 0x10000001)
/* code de chunk de donnees */
#define HTLS_DATA_USER_LIST				((uint16) 0x012c)			/* supported */
#define HTLS_DATA_NEWS						((uint16) 0x0065)			/* supported */
#define HTLS_DATA_AGREEMENT				((uint16) 0x0065)			/* supported */
#define HTLS_DATA_AGREEMENT_NEW			((uint16) 0x009a)
#define HTLS_DATA_USER_INFO				((uint16) 0x0065)			/* supported */
#define HTLS_DATA_CHAT						((uint16) 0x0065)			/* supported */
#define HTLS_DATA_MSG						((uint16) 0x0065)			/* supported */
#define HTLS_DATA_FILE_LIST				((uint16) 0x00c8)			/* supported */
#define HTLS_DATA_HTXF_SIZE				((uint16) 0x006c)			/* supported */
#define HTLS_DATA_HTXF_REF					((uint16) 0x006b)			/* supported */
#define HTLS_DATA_TASKERROR				((uint16) 0x0064)			/* supported */
#define HTLS_DATA_SOCKET					((uint16) 0x0067)			/* supported */
#define HTLS_DATA_ICON						((uint16) 0x0068)			/* supported */
#define HTLS_DATA_COLOUR					((uint16) 0x0070)			/* supported */
#define HTLS_DATA_NICK						((uint16) 0x0066)			/* supported */
#define HTLS_DATA_FILE_ICON				((uint16) 0x00d5)			/* supported */
#define HTLS_DATA_FILE_TYPE				((uint16) 0x00cd)			/* supported */
#define HTLS_DATA_FILE_CREATOR			((uint16) 0x00ce)			/* supported */
#define HTLS_DATA_FILE_SIZE				((uint16) 0x00cf)			/* supported */
#define HTLS_DATA_FILE_NAME				((uint16) 0x00c9)			/* supported */
#define HTLS_DATA_FILE_CDATE				((uint16) 0x00d0)			/* supported */
#define HTLS_DATA_FILE_MDATE				((uint16) 0x00d1)			/* supported */
#define HTLS_DATA_FILE_COMMENT			((uint16) 0x00d2)			/* supported */
#define HTLS_DATA_CHAT_REF        		((uint16) 0x0072)			/* supported */
#define HTLS_DATA_CHAT_SUBJECT       	((uint16) 0x0073)			/* supported */
#define HTLS_DATA_FILE_QUEUE			((uint16) 0x0074)
#define HTLS_DATA_NEWS_FOLDER_ITEM		((uint16) 0x0140)
#define HTLS_DATA_NEWS_CATLIST			((uint16) 0x0141)
#define HTLS_DATA_NEWS_CATEGORY			((uint16) 0x0142)
#define HTLS_DATA_NEWS_CATEGORYITEM		((uint16) 0x0143)
#define HTLS_DATA_NEWS_NEWSPATH			((uint16) 0x0145)
#define HTLS_DATA_NEWS_THREADID			((uint16) 0x0146)
#define HTLS_DATA_NEWS_NEWSTYPE			((uint16) 0x0147)
#define HTLS_DATA_NEWS_NEWSSUBJECT 		((uint16) 0x0148)
#define HTLS_DATA_NEWS_NEWSAUTHOR 		((uint16) 0x0149)
#define HTLS_DATA_NEWS_NEWSDATE 		((uint16) 0x014a)
#define HTLS_DATA_NEWS_PREVTHREAD 		((uint16) 0x014b)
#define HTLS_DATA_NEWS_NEXTTHREAD 		((uint16) 0x014c)
#define HTLS_DATA_NEWS_NEWSDATA 		((uint16) 0x014d)
#define HTLS_DATA_NEWS_PARENTTHREAD		((uint16) 0x014e)
#define HTLS_DATA_NEWS_PARENT_POST		((uint16) 0x014f)
#define HTLS_DATA_NEWS_CHILD_POST		((uint16) 0x0150)
#define HTLS_SERVER_VERSION				((uint16) 0x00a0)
#define HTLS_SERVER_NAME				((uint16) 0x00a2)
#define HTLS_HDR_SILVERWING_MODE		((uint32) 0x10000001)
#if 0
#define HTLS_DATA_OPTION                ((uint16) 0x006d)
#endif
#endif
