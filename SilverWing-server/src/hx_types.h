/**************************************************************
 * This file is from hxd.
 **************************************************************/
#ifndef __HX_TYPES_H__
#define __HX_TYPES_H__
#include <sys/types.h>

struct hx_file
{
	uint32 size;
	uint32 fnlen;
	char	ftype[4], fcreator[4];
	char fname[4096];
};

struct hx_tracker
{
	uint16 port;
	uint16 users;
	char address[15];
	char name[1024];
	char desc[4096];
};

struct hx_hdr
{
/*	uint32	type,trans,flag,len,len2;
	uint16	hc;
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
	uint8        data[0];
#else
	uint8        data[1];
#endif*/
	long	type;
	long	trans;
	long	flag;
	long	len;
	long	len2;
	uint16  hc;
	short	data;
};

struct xfer {
	struct xfer *next;
	int nr, type;
	uint32 size, data_pos, rsrc_pos;
	char *local, *remote;
	time_t time;
	pid_t pid;
};

struct x_fhdr {
	uint16 enc;
	uint8 len, name[1];
};

struct hx_data_hdr
{
	uint16	type,len;
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
	uint8        data[0];
#else
	uint8        data[1];
#endif
};

struct hx_news_snd_data_hdr
{
	uint16	type,len;
	uint16	d_type;
	uint16	flag;
	uint8  d_len;
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
	uint8        data[0];
#else
	uint8        data[1];
#endif
};

struct hx_news_data_hdr
{
	uint16	type,len;
	uint8	d_type;
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
	uint8        data[0];
#else
	uint8        data[1];
#endif
};

struct htxf_hdr {
	uint32 magic, ref, type, len;
};


struct hx_userlist_hdr {
	uint16 type, len;
	uint16 sock, icon, colour, nlen;
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
	uint8	nick[0];
#else
	uint8	nick[1];
#endif
};

#define SIZEOF_HX_HDR           22
#define SIZEOF_HX_DATA_HDR      4
#define SIZEOF_HX_FILELIST_HDR  24
#define SIZEOF_HX_USERLIST_HDR  12
#define SIZEOF_HTXF_HDR         16
#define XFER_GET 0
#define XFER_PUT 1

struct hx_filelist_hdr {
	uint16 type, len;
	uint8	ftype[4], fcreator[4];
	uint32 fsize, unknown, fnlen;
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
	uint8	fname[0];
#else
	uint8	fname[1];
#endif
};
#define CR2LF(_ptr, _len) \
{ register unsigned int _i;\
	for (_i = 0; _i < (unsigned)_len; _i++) {\
		if (_ptr[_i] == '\r')\
			_ptr[_i] = '\n';\
		else if (_ptr[_i] == 14)\
			_ptr[_i] = 'N';\
		else if (_ptr[_i] == '\010')\
			_ptr[_i] = 'b';\
	}\
}
#endif
