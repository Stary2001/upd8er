#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include "http.h"
#include "util.h"

int main(int argc, char **argv)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	httpcInit(0x10000);

	printf("Attempting to download...\n");

	Result res;

	if((res = util::http::download_file("http://d0k3.secretalgorithm.com/latest/Decrypt9WIP", "/d9.zip")) == 0)
	{
		printf("Downloaded successfully!\n");
		printf("Checking sha256..\n");
		u8 hash[0x20];

		if((res = util::file_sha256("/d9.zip", hash) == 0))
		{
			u8 d9_hash[0x20] = {0xe9,0xf1,0xe3,0x12,0x5d,0xba,0x76,0x68,0x8c,0xc8,0xd1,0xd4,0xaa,0x40,0xa6,0xd8,0x61,0x4a,0x17,0xdf,0xc8,0x2e,0x0f,0x6d,0xca,0xc4,0x14,0x38,0x1e,0x26,0xfe,0xa1};

			if(memcmp(hash, d9_hash, 0x20) == 0)
			{
				printf("SHA check success!\n");
			}
			else
			{
				for(int i = 0; i < 0x20; i++)
				{
					printf("%02x", hash[i]);
				}
				printf("\n");
			}
		}
		else
		{
			printf(":( sha got %08x\n", res);
		}
	}
	else
	{
		printf(":( http got %08x\n", res);
	}

	while (aptMainLoop())
	{
		hidScanInput();
		if (hidKeysDown() & KEY_A) break;
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	httpcExit();
	gfxExit();
	return 0;
}