//
// ������ ��������
//
// robocraft.ru
//


#include "parse.h"

// ���������� ��������
#define EXTRACT_FIRMWARE	1
// ������� CRC-�����
#define CALC_CRC			0
// ��������� ��������
#define PACK_FIRMWARE		0

// ���������� �������������� ���������
void info()
{
	printf("\nhttp:\/\/robocraft.ru\n");
}

// ���������� ��������� �� �������������
void help()
{
	printf("[i] Usage:\n");
	printf("ParseImage ImageName\n");
	info();
}

// ���������� ��������� �� �������������
void help2()
{
	printf("[i] Usage:\n");
	printf("ParseImage bcm.bin cfe.bin sqsh.bin kernel.bin\n");
	printf("[i] if D-link firmware:\n");
	printf("ParseImage bcm.bin cfe.bin sqsh.bin kernel.bin name.bin\n");
	info();
}

#if EXTRACT_FIRMWARE
//
// ������������� ��������
//

// ParseImage.exe

int main(int argc, char* argv[])
{
	printf("[i] Simple Firmware Parser.\n");

	if(argc < 2){
		// �� ������ ��� ����� ��������
		printf("[!] Set file name!\n");
		help();
		return -1;
	}

	// ��� ����� ������� ������ ����������
	char* filename = argv[1];
	printf("[i] file: %s\n", filename);

	bcmtag tag;

	parseFirmware(filename, &tag);

	printf("[i] Done.\n");
	info();
	return 0;
}
#elif CALC_CRC
//
// ������� CRC �����
//

// crc.exe

int main(int argc, char* argv[])
{
	printf("[i] Calc CRC.\n");

	if(argc < 2){
		// �� ������ ��� ����� ��������
		printf("[!] Set file name!\n");
		help();
		return -1;
	}

	// ��� ����� ������� ������ ����������
	char* filename = argv[1];
	printf("[i] file: %s\n", filename);


	FILE* fs=0;
	fs = fopen(filename, "rb");

	if(!fs){
		printf("[!] Error: cant open file: %s!\n", filename);
		return -2;
	}

	//
	// ������ ������ �����
	//
	fseek(fs, 0, SEEK_END); // ������� � ����� ����
	unsigned fileSize = ftell(fs); // �������� �������
	printf("[i] file size: %d\n", fileSize);

	//
	// ��������� ���� � �����
	//
	uchar* buf = new uchar [fileSize];
	if(!buf){
		printf("[!] Error: cant allocate memory!\n");
		fclose(fs);
		return -4;
	}
	// ��������������� �� ������
	fseek(fs, 0, SEEK_SET);
	fread(buf, fileSize, 1, fs);
	fclose(fs);

	//
	// CRC
	//
	uint32_t crc;
	char file_crc[16];

	// all file
	crc = CRC32_INIT_VALUE;
	crc = getCrc32((uint8_t *) (buf), (uint32_t) (fileSize), crc);
	sprintf(file_crc, "%08lX", (long unsigned int) crc);
	printf("[i] CRC file\t\t: 0x%s\n", file_crc);

	return 0;
}
#elif PACK_FIRMWARE
//
// ����������� ������ � ��������
//

// packImage.exe

int main(int argc, char* argv[])
{
	printf("[i] Make fimware.\n");

	if(argc < 5){
		// �� ������ ��� ����� ��������
		printf("[!] Set file names!\n");
		help2();
		return -1;
	}

	bool isDlink = false;

	size_t size_result=0, size_bcm=0, size_cfe=0, size_sqsh=0, size_kernel=0, size_name=0;

	FILE* fresult=0, *fbcm=0, *fcfe=0, *fsqsh=0, *fkernel=0, *fname=0;

	// ��� ����� ������� ������ ����������
	char* file_bcm = argv[1];
	printf("[i] bcm file: %s\n", file_bcm);
	fbcm = fopen(file_bcm, "rb");
	if(fbcm){
		size_bcm = getFileSize(fbcm);
	}
	// �������� ����� ���������
	if(size_bcm!=BCM_LENGTH){
		printf("[!] Error: wrong bcm file size: %s (%d bytes)\n", file_bcm, size_bcm);
		help2();
		return -1;
	}

	char* file_cfe = argv[2];
	printf("[i] cfe file: %s\n", file_cfe);
	fcfe = fopen(file_cfe, "rb");
	if(fcfe){
		size_cfe = getFileSize(fcfe);
	}

	char* file_sqsh = argv[3];
	printf("[i] sqsh file: %s\n", file_sqsh);
	fsqsh = fopen(file_sqsh, "rb");
	if(fsqsh){
		size_sqsh = getFileSize(fsqsh);
	}
	
	char* file_kernel = argv[4];
	printf("[i] kernel file: %s\n", file_kernel);
	fkernel = fopen(file_kernel, "rb");
	if(fkernel){
		size_kernel = getFileSize(fkernel);
	}

	char* file_name = 0;
	// ��� �������� �� D-link-�
	if(argc>5){
		isDlink = true;
		file_name = argv[5];
		printf("[i] name file: %s\n", file_name);
		fname = fopen(file_name, "rb");
		if(fname){
			size_name = getFileSize(fname);
		}
		// �������� ����� ��������
		if(size_name!=NAME_LENGTH){
			printf("[!] Error: wrong name file size: %s (%d bytes)\n", file_name, size_name);
			help2();
			return -1;
		}
	}


	size_result = size_bcm + size_cfe + size_sqsh + size_kernel + size_name;

	if(!fbcm || !fcfe || !fsqsh || !fkernel){
		printf("[!] Error: cant open files!\n");
		return -2;
	}
	if(isDlink && !fname){
		printf("[!] Error: cant open name-file!\n");
		return -2;
	}

	printf("[i] result size: %d\n", size_result);

	fresult = fopen(result_file, "wb+");
	if(!fresult){
		printf("[!] Error: cant open file for writing: %s!\n", result_file);
		return -2;
	}

	//
	// ����� ��� ����������
	//
	uchar* buf = new uchar [size_result];
	if(!buf){
		printf("[!] Error: cant allocate memory!\n");
		return -4;
	}

	//
	// ��������� ����� � ����� �����
	//

	// bcm
	fread(buf, size_bcm, 1, fbcm);
	// cfe
	fread((uint8_t *) (buf + size_bcm), size_cfe, 1, fcfe);
	// sqsh
	fread((uint8_t *) (buf + size_bcm + size_cfe), size_sqsh, 1, fsqsh);
	// kernel
	fread((uint8_t *) (buf + size_bcm + size_cfe + size_sqsh), size_kernel, 1, fkernel);
	// name
	if(isDlink){
		fread((uint8_t *) (buf + size_bcm + size_cfe + size_sqsh + size_kernel), size_name, 1, fname);	
	}

	//
	// ������� CRC
	//
	uint32_t crc=CRC32_INIT_VALUE, crc_bcm=CRC32_INIT_VALUE, crc_image=CRC32_INIT_VALUE, crc_rootfs=CRC32_INIT_VALUE, crc_kernel=CRC32_INIT_VALUE;
	char file_crc[16], header_crc[16], image_crc[16], rootfs_crc[16], kernel_crc[16];

	// CRC ������ cfe+sqsh+kernel
	crc_image = getCRC32String((uint8_t *) (buf + size_bcm), size_cfe+size_sqsh+size_kernel, image_crc);
	printf("[i] CRC image\t\t: 0x%s\n", image_crc);
	if(isDlink){
		// CRC ������ cfe+sqsh+kernel+name
		crc_image = getCRC32String((uint8_t *) (buf + size_bcm), size_cfe+size_sqsh+size_kernel+size_name, image_crc);
		printf("[i] D-link CRC image\t\t: 0x%s\n", image_crc);
	}

	// CRC �������� �������� �������
	crc_rootfs = getCRC32String((uint8_t *) (buf + size_bcm + size_cfe), size_sqsh, rootfs_crc);
	printf("[i] CRC sqsh\t\t: 0x%s\n", rootfs_crc);

	// CRC ����
	crc_kernel = getCRC32String((uint8_t *) (buf + size_bcm + size_cfe + size_sqsh), size_kernel, kernel_crc);
	printf("[i] CRC kernel\t\t: 0x%s\n", kernel_crc);

	//
	// ������ ��������
	//
	printf("[i] update size...\n");

	// totalImageLen  = cfe+sqsh+kernel
	uint32_t place = TAG_VER_LEN+SIG_LEN+SIG_LEN_2+CHIP_ID_LEN+BOARD_ID_LEN+FLAG_LEN;
	updateLength(buf, place, size_cfe + size_sqsh + size_kernel);
	if(isDlink){
		// totalImageLen = cfe+sqsh+kernel+name
		updateLength(buf, place, size_cfe + size_sqsh + size_kernel + size_name);
	}
	// cfe
	place += IMAGE_LEN + ADDRESS_LEN;
	updateLength(buf, place, size_cfe);
	// rootFS
	place += IMAGE_LEN + ADDRESS_LEN;
	updateLength(buf, place, size_sqsh);
	// kernel
	place += IMAGE_LEN + ADDRESS_LEN;
	updateLength(buf, place, size_kernel);

	
	//
	// ������ CRC 
	//

	// ������ (cfe+sqsh+kernel)
	printf("[i] update image CRC...\n");
	updateCRC(buf, TAG_LEN - 2*TOKEN_LEN, crc_image);
	// �������� �������
	printf("[i] update sqsh CRC...\n");
	updateCRC(buf, TAG_LEN - 2*TOKEN_LEN + 4, crc_rootfs);
	// ����
	printf("[i] update kernel CRC...\n");
	updateCRC(buf, TAG_LEN - 2*TOKEN_LEN + 8, crc_kernel);

	//
	// ������, � ������ CRC � ��������� - �����
	// ����������� CRC ������ ���������
	//

	// ������� CRC ���������
	crc_bcm = getCRC32String((uint8_t *) (buf), size_bcm-TOKEN_LEN, header_crc);
	printf("[i] CRC header\t\t: 0x%s\n", header_crc);
	// ���������
	printf("[i] update header CRC...\n");
	updateCRC(buf, TAG_LEN - TOKEN_LEN, crc_bcm);

	printf("[i] write file: %s\n", result_file);
	//
	// ���������� ��������� � ����
	//
	fwrite(buf, size_result, 1, fresult);

	// �������� ������
	fclose(fbcm);
	fclose(fcfe);
	fclose(fsqsh);
	fclose(fkernel);

	fclose(fresult);

	printf("[i] Done.\n");
	info();
	return 0;
}
#endif