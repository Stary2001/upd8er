#include <3ds.h>
#include <arpa/inet.h>
#include <string.h>
#include <fstream>
#include "util.h"

size_t get_tmd_off(u8 *buffer, size_t len, size_t& tmd_len)
{
	u32 header_size = util::align_up(*(u32*)(buffer), 64);
	u32 cert_size = util::align_up(*(u32*)(buffer + 0x08), 64);
	u32 tik_size = util::align_up(*(u32*)(buffer + 0x0c), 64);
	u32 tmd_size = util::align_up(*(u32*)(buffer + 0x10), 64);

	tmd_len = tmd_size;
	printf("TMD at %x + %x + %x\n", header_size, cert_size, tik_size);

	return header_size + cert_size + tik_size;
}

u64 get_tmd_title_id(u8* buffer, size_t len)
{
	u32 sig_type = ntohl(*(u32*)buffer);

	u32 sig_size = 0;
	u32 sig_padding = 0;
	switch(sig_type)
	{
		case 0x010003:
			sig_size = 0x200;
			sig_padding = 0x3c;
		break;

		case 0x010004:
			sig_size = 0x100;
			sig_padding = 0x3c;
		break;

		case 0x010005:
			sig_size = 0x3c;
			sig_padding = 0x40;
		break;

		default:
		{
			printf("Unk TMD sig type %08x!!\n", sig_type);
			return 0;
		}
	}
	
	u32 tmd_header_off = sig_size + sig_padding + 4;
	u32 title_id_off = tmd_header_off + 0x4c;

	u64 title_id = 0;
	memcpy(&title_id, buffer + title_id_off, 8);

	return util::bswap64(title_id);
}

u64 util::get_cia_title_id(u8 *buffer, size_t len)
{
	size_t tmd_len;
	size_t tmd_off = get_tmd_off(buffer, len, tmd_len);

	return get_tmd_title_id(buffer + tmd_off, tmd_len);
}

Result util::install_cia(u8 *buffer, size_t len)
{
	Result res;

	Handle cia_handle;
	if(R_FAILED(res = AM_StartCiaInstall(MEDIATYPE_SD, &cia_handle)))
	{
		printf("AM_StartCiaInstall failed with %08x!\n", res);
		return res;
	}

	u32 bytes_written;
	printf("Writing entire CIA at once...\n");
	if(R_FAILED(res = FSFILE_Write(cia_handle, &bytes_written, 0, buffer, len, 0)))
	{
		printf("FSFILE_Write failed with %08x!\n", res);
		svcCloseHandle(cia_handle);
		return res;
	}

	if(R_FAILED(res = AM_FinishCiaInstall(cia_handle)))
	{
		printf("AM_FinishCiaInstall failed with %08x!\n", res);
		svcCloseHandle(cia_handle);
		return res;
	}
	
	return 0;
}