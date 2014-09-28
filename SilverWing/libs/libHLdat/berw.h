#ifndef __berw_h
#define __berw_h

//#include "hx_types.h"
#include <File.h>

#define u_int32_t uint32
#define int32_t int32
#define u_int16_t uint16
#define int16_t int16
#define u_int8_t uint8
#define int8_t int8

class be {
public:
static  u_int32_t be_read32 (BFile& fd);

static u_int32_t be_read24 (BFile& fd);

static u_int16_t be_read16 (BFile& fd);

static ssize_t be_write16 (BFile& fd, u_int16_t word);

static ssize_t be_write24 (BFile& fd, u_int32_t word);

static ssize_t be_write32 (BFile& fd, u_int32_t word);
};
#endif
