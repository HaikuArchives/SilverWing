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
#define u_int32_t uint32
#define u_int16_t uint16
#define u_int8_t uint8
/********************************/
/* codes utilises par le client */
/********************************/

/* code de header */
#define HTLC_HDR_LOGIN			 	((u_int32_t) 0x0000006b)		/* done */
#define HTLC_HDR_USER_GETLIST		((u_int32_t) 0x0000012c)		/* done */
#define HTLC_HDR_NEWS_GETFILE		((u_int32_t) 0x00000065)		/* done */
#define HTLC_HDR_FILE_LIST			((u_int32_t) 0x000000c8)		/* done */
#define HTLC_HDR_FILE_GET		 	((u_int32_t) 0x000000ca)		/* done */
#define HTLC_HDR_FILE_PUT		 	((u_int32_t) 0x000000cb)		/* done */
#define HTLC_HDR_FILE_MKDIR     ((u_int32_t) 0x000000cd)			/* not checked */
#define HTLC_HDR_MSG					((u_int32_t) 0x0000006c)		/* done */
#define HTLC_HDR_USER_CHANGE	 	((u_int32_t) 0x00000130)		/* done */
#define HTLC_HDR_FILE_GETINFO		((u_int32_t) 0x000000ce)		/* done */
#define HTLC_HDR_FILE_DELETE		((u_int32_t) 0x000000cc)		/* not checked */
#define HTLC_HDR_NEWS_POST      	((u_int32_t) 0x00000067)		/* done */
#define HTLC_HDR_CHAT_CREATE    	((u_int32_t) 0x00000070)		/* done */
#define HTLC_HDR_CHAT           	((u_int32_t) 0x00000069)		/* done */
#define HTLC_HDR_CHAT_LEAVE     	((u_int32_t) 0x00000074)		/* done */
#define HTLC_HDR_CHAT_SUBJECT   	((u_int32_t) 0x00000078)		/* done */
#define HTLC_HDR_CHAT_JOIN      	((u_int32_t) 0x00000073)		/* done */
#define HTLC_HDR_CHAT_INVITE    ((u_int32_t) 0x00000071)

#define HTLC_HDR_USER_KICK      ((u_int32_t) 0x0000006e)
#define HTLC_HDR_USER_GETINFO   ((u_int32_t) 0x0000012f)
#define HTLC_HDR_USER_CREATE    ((u_int32_t) 0x0000015e)
#define HTLC_HDR_USER_OPEN      ((u_int32_t) 0x00000160)
#define HTLC_HDR_FILE_SETINFO   ((u_int32_t) 0x000000cf)
#define HTLC_HDR_FILE_MOVE      ((u_int32_t) 0x000000d0)
#define HTLC_HDR_CHAT_DECLINE   ((u_int32_t) 0x00000072)
#define HTLC_HDR_NEWSDIRLIST	((u_int32_t) 0x00000172)
#define HTLC_HDR_NEWSCATLIST	((u_int32_t) 0x00000173)
#define HTLC_HDR_DELNEWSDIRCAT	((u_int32_t) 0x0000017C)
#define HTLC_HDR_MAKENEWSDIR	((u_int32_t) 0x0000017D)
#define HTLC_HDR_MAKECATEGORY	((u_int32_t) 0x0000017E)
#define HTLC_HDR_GETTHREAD		((u_int32_t) 0x00000190)
#define HTLC_HDR_POSTTHREAD		((u_int32_t) 0x0000019A)
#define HTLC_HDR_DELETETHREAD	((u_int32_t) 0x0000019B)


/* code de chunk de donnees */
#define HTLC_DATA_NICK			 	((u_int16_t) 0x0066)				/* supported */
#define HTLC_DATA_ICON			 	((u_int16_t) 0x0068)				/* supported */
#define HTLC_DATA_LOGIN				((u_int16_t) 0x0069)				/* supported */
#define HTLC_DATA_PASSWORD			((u_int16_t) 0x006a)				/* supported */
#define HTLC_DATA_DIR			  	((u_int16_t) 0x00ca)				/* supported */
#define HTLC_DATA_FILE			 	((u_int16_t) 0x00c9)				/* supported */
#define HTLC_DATA_RESUMEINFO		((u_int16_t) 0x00cb)
#define HTLC_DATA_RESUMEFLAG		((u_int16_t) 0x00cc)				/* supported (file transfert extra data) */
#define HTLC_DATA_SOCKET		  	((u_int16_t) 0x0067)				/* supported */
#define HTLC_DATA_MSG			  	((u_int16_t) 0x0065)				/* supported */
#define HTLC_DATA_NEWS_POST     	((u_int16_t) 0x0065)				/* supported */
#define HTLC_DATA_CHAT          	((u_int16_t) 0x0065)				/* supported */
#define HTLC_DATA_CHAT_REF      	((u_int16_t) 0x0072)				/* supported */
#define HTLC_DATA_CHAT_SUBJECT  	((u_int16_t) 0x0073)				/* supported */
#define HTLC_DATA_NEWS				((u_int16_t) 0x0145)                /* supported by me */

#define HTLC_DATA_HTXF_SIZE     ((u_int16_t) 0x006c)
#define HTLC_DATA_OPTION        ((u_int16_t) 0x006d)
#define HTLC_DATA_FILE          ((u_int16_t) 0x00c9)
#define HTLC_DATA_BAN           ((u_int16_t) 0x0071)
#define HTLC_DATA_FILE_RENAME   ((u_int16_t) 0x00d3)
#define HTLC_DATA_DIR_RENAME    ((u_int16_t) 0x00d4)



/*********************************/
/* codes utilises par le serveur */
/*********************************/

/* code de header */
#define HTLS_HDR_TASK						((u_int32_t) 0x00010000)	/* supported / partial */
#define HTLS_HDR_AGREEMENT					((u_int32_t) 0x0000006d)	/* supported */
#define HTLS_HDR_USER_LEAVE				((u_int32_t) 0x0000012e)	/* supported */
#define HTLS_HDR_USER_CHANGE				((u_int32_t) 0x0000012d)	/* supported */
#define HTLS_HDR_NEWS_POST					((u_int32_t) 0x00000066)	/* supported */
#define HTLS_HDR_MSG							((u_int32_t) 0x00000068)	/* supported (private message) */
#define HTLS_HDR_CHAT						((u_int32_t) 0x0000006a)	/* not supported */
#define HTLS_HDR_CHAT_INVITE				((u_int32_t) 0x00000071)	/* --- under test, not supported */
#define HTLS_HDR_CHAT_SUBJECT				((u_int32_t) 0x00000077)	/* supported */
#define HTLS_HDR_CHAT_USER_CHANGE		((u_int32_t) 0x00000075)	/* supported */
#define HTLS_HDR_CHAT_USER_LEAVE			((u_int32_t) 0x00000076)	/* supported */
#define HTLS_HDR_UNKNOWN					((u_int32_t) 0x00000162)   /* unsupported */
#define HTLS_HDR_POLITEQUIT             ((u_int32_t) 0x0000006f)


/* code de chunk de donnees */
#define HTLS_DATA_USER_LIST				((u_int16_t) 0x012c)			/* supported */
#define HTLS_DATA_NEWS						((u_int16_t) 0x0065)			/* supported */
#define HTLS_DATA_AGREEMENT				((u_int16_t) 0x0065)			/* supported */
#define HTLS_DATA_USER_INFO				((u_int16_t) 0x0065)			/* supported */
#define HTLS_DATA_CHAT						((u_int16_t) 0x0065)			/* supported */
#define HTLS_DATA_MSG						((u_int16_t) 0x0065)			/* supported */
#define HTLS_DATA_FILE_LIST				((u_int16_t) 0x00c8)			/* supported */
#define HTLS_DATA_HTXF_SIZE				((u_int16_t) 0x006c)			/* supported */
#define HTLS_DATA_HTXF_REF					((u_int16_t) 0x006b)			/* supported */
#define HTLS_DATA_TASKERROR				((u_int16_t) 0x0064)			/* supported */
#define HTLS_DATA_SOCKET					((u_int16_t) 0x0067)			/* supported */
#define HTLS_DATA_ICON						((u_int16_t) 0x0068)			/* supported */
#define HTLS_DATA_COLOUR					((u_int16_t) 0x0070)			/* supported */
#define HTLS_DATA_NICK						((u_int16_t) 0x0066)			/* supported */
#define HTLS_DATA_FILE_ICON				((u_int16_t) 0x00d5)			/* supported */
#define HTLS_DATA_FILE_TYPE				((u_int16_t) 0x00cd)			/* supported */
#define HTLS_DATA_FILE_CREATOR			((u_int16_t) 0x00ce)			/* supported */
#define HTLS_DATA_FILE_SIZE				((u_int16_t) 0x00cf)			/* supported */
#define HTLS_DATA_FILE_NAME				((u_int16_t) 0x00c9)			/* supported */
#define HTLS_DATA_FILE_CDATE				((u_int16_t) 0x00d0)			/* supported */
#define HTLS_DATA_FILE_MDATE				((u_int16_t) 0x00d1)			/* supported */
#define HTLS_DATA_FILE_COMMENT			((u_int16_t) 0x00d2)			/* supported */
#define HTLS_DATA_CHAT_REF        		((u_int16_t) 0x0072)			/* supported */
#define HTLS_DATA_CHAT_SUBJECT       	((u_int16_t) 0x0073)			/* supported */
#define HTLS_DATA_NEWS_FOLDER_ITEM		((u_int16_t) 0x0140)
#define HTLS_DATA_NEWS_CATLIST			((u_int16_t) 0x0141)
#define HTLS_DATA_NEWS_CATEGORY			((u_int16_t) 0x0142)
#define HTLS_DATA_NEWS_CATEGORYITEM		((u_int16_t) 0x0143)
#define HTLS_DATA_NEWS_NEWSPATH			((u_int16_t) 0x0145)
#define HTLS_DATA_NEWS_THREADID			((u_int16_t) 0x0146)
#define HTLS_DATA_NEWS_NEWSTYPE			((u_int16_t) 0x0147)
#define HTLS_DATA_NEWS_NEWSSUBJECT 		((u_int16_t) 0x0148)
#define HTLS_DATA_NEWS_NEWSAUTHOR 		((u_int16_t) 0x0149)
#define HTLS_DATA_NEWS_NEWSDATE 		((u_int16_t) 0x014a)
#define HTLS_DATA_NEWS_PREVTHREAD 		((u_int16_t) 0x014b)
#define HTLS_DATA_NEWS_NEXTTHREAD 		((u_int16_t) 0x014c)
#define HTLS_DATA_NEWS_NEWSDATA 		((u_int16_t) 0x014d)
#define HTLS_DATA_NEWS_PARENTTHREAD		((u_int16_t) 0x014e)
#define HTLS_DATA_NEWS_PARENT_POST		((u_int16_t) 0x014f)
#define HTLS_DATA_NEWS_CHILD_POST		((u_int16_t) 0x0150)

#define HTLS_SERVER_VERSION				((u_int16_t) 0x00a0)
#define HTLS_SERVER_NAME				((u_int16_t) 0x00a2)
#if 0
#define HTLS_DATA_OPTION                ((u_int16_t) 0x006d)
#endif
#endif
