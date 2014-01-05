
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "nkutil.h"
#include <time.h>
#include <sys/time.h>
#include "time_zones.h"
//#include <linux/mtd/mtd.h>
/*#ifndef CONFIG_TWO_FLASH_BANKS
					GQF1100 partaction
/ # cat /proc/mtd
dev:    size   erasesize  name
mtd0: 00080000 00020000 "bootloader"
mtd1: 01e00000 00020000 "image"
mtd2: 00100000 00020000 "DB"
mtd3: 00040000 00020000 "rg_factory"
#else
					QTM3000 partaction
dev:    size   erasesize  name
mtd0: 00800000 00020000 "image2"
mtd1: 01800000 00020000 "DB2"
mtd2: 00080000 00020000 "bootloader"
mtd3: 01e00000 00020000 "image"
mtd4: 00100000 00020000 "DB"
mtd5: 00040000 00020000 "rg_factory"
#endif
*/
#define PROC_MTD	"/proc/mtd"
#define IMAGE_SECT	"image"
#define BOOT_SECT	"bootloader"
#define MAX_PHYSMAP_PARTITIONS    8	// same value define in uClinux/linux/driver/mtd/maps/brecis_flashmap.c
#define START_IMAGE_SECT "image1"
#if 0
// mtd defination move to nkdef.h
#ifndef CONFIG_TWO_FLASH_BANKS
#define nk_FACTORY_MTD	"/dev/mtd3"
#else
#define nk_FACTORY_MTD	"/dev/mtd5"
#endif
#define nk_FACTORY_MTD_OFFEST	"0x0"
#define nk_FACTORY_MTD_LEN	"0x20000"
#ifndef CONFIG_TWO_FLASH_BANKS
#define nk_LOADER_MTD	"/dev/mtd0"
#else
#define nk_LOADER_MTD	"/dev/mtd2"
#endif
#define nk_LOADER_MTD_OFFEST	"0x0"
#define nk_LOADER_MTD_LEN	"0x80000"
#ifndef CONFIG_TWO_FLASH_BANKS
#define nk_IMAGE_MTD	"/dev/mtd1"
#else
#define nk_IMAGE_MTD	"/dev/mtd3"
#define nk_IMAGE2_MTD	"/dev/mtd0"
#endif
#define nk_IMAGE_MTD_OFFEST	"0x0"
#define nk_IMAGE_MTD_LEN	"0x1e00000"
#endif
//read,write FLASH,copy dir *
#define VAR_FLASH	"/etc/flash/var/"
#if 0 /*purpose:0013264, author:selena, description:new log backup path*/
#define VAR_LOG_FLASH	"/etc/flash/var/log/"
#endif
#define VAR_LOG_RAM	"/var/log/"
#define DB_FLASH	"/etc/flash/etc/"
#define DB_RAM	"/tmp/"
#define DB_FACTORY	"/etc/"
#define DB_FILENAME  "nk_sysconfig"

struct mtd_info_1 *mtd_head = NULL;
#define RG_FACTORY	"/tmp/.factory"
char nk_rg_factory[480];
void nk_rg_factory_read_file_fun(int len)
{
	int fd,err;
	if((fd = open(RG_FACTORY,O_SYNC|O_RDONLY)) < 0)
	{
		printf("open %s Error",RG_FACTORY);
		close(fd);		
	}
	err = read(fd,nk_rg_factory,len);
	nk_rg_factory[len+1]='\0';
	if(err < 0)
	{
		printf("open %s Error",RG_FACTORY);
	}
	close(fd);
}
 void nk_rg_factory_write_file_fun(void)
{

	FILE *fp;
	int i;
	i = 0;
	if (!(fp = fopen(RG_FACTORY, "w"))) {
		warn("could not open input file=%s",RG_FACTORY);
		return;
	}
	fprintf(fp, "%s \n",nk_rg_factory);
	fclose(fp);

}

#ifdef CONFIG_NK_DB_CHECKSUM
unsigned int doSum ( char *data, int len ) {

	int i;
	unsigned int sum=0;

	for ( i=0; i < len; i++ )
		sum += data[i];

	return sum;
}
#endif

int MergeToRAM(void)
{
    FILE  *srcFile, *destFile, *listFile;
    int i=0,idx;
    char fileLine[LENGTH_READ_BUF], mulu[LENGTH_READ_PATH],tmp[513];
    char opcode[LENGTH_READ_BUF];
    int l,j, fd;
#ifdef CONFIG_NK_DB_CHECKSUM
    unsigned int sum=0;
#endif

//protect merge process
    umask(0);
    fd = open("/tmp/test.txt", O_RDONLY | O_CREAT | O_EXCL, 0666);
    j = 0;
    while ( fd < 0)
    {
	srand(time(0));
	j = 700 + (int) (900.0 * rand()/(RAND_MAX + 1.0));
	for (i = 0; i < j; i ++);
	fd = open ("/tmp/test.txt", O_RDONLY | O_CREAT | O_EXCL, 0666);
    }
    i = close(fd);
    i = unlink("/tmp/test.txt");

    fileLine[0]='\0';
    tmp[0]='\0';

    if ( NULL == (destFile = fopen("/tmp/new_one", "w+")))
	    return 0;
    else
    {
	if ( NULL == (listFile = fopen("/tmp/category_list", "r+")))
	{
	    fclose(destFile);
	    return 0;
	}
	else
	{
	    while(fgets(fileLine,LENGTH_READ_SIZE,listFile) != NULL)
	    {
		idx = i = 0;
		while ( fileLine[idx] != ' ' && fileLine[idx] != '\0' && fileLine[idx] != '\n')
			opcode[i++] = fileLine[idx++];
		opcode[i] = '\0';
		sprintf(mulu,"/tmp/splitDB/%s",opcode);
	        if ( NULL == (srcFile = fopen(mulu, "r+")))
		{
		    fclose(destFile);
		    fclose(listFile);
		    return 0;
		}
		else
		{
		#ifdef CONFIG_NK_DB_CHECKSUM
			if ( !strcmp ( opcode, "CHECKSUM" ) ) {
			}
			else {
				while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
				{
					sum += doSum ( fileLine, strlen ( fileLine ) );
					fputs(fileLine,destFile);
				}//while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
				sum += doSum ( "\n", strlen ( "\n" ) );
				fputs("\n",destFile);
			}
			fclose(srcFile);
		#else
		    while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
		    {
			fputs(fileLine,destFile);
		    }//while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
		    fputs("\n",destFile);
		    fclose(srcFile);
		#endif
		}
		
	    }//while(fgets(fileLine,LENGTH_READ_SIZE,listFile) != NULL)
	    fclose(listFile);
	#ifdef CONFIG_NK_DB_CHECKSUM
		fputs ( "[CHECKSUM]\n", destFile );
		sprintf ( tmp, "SUM=%u\n", sum );
		fputs ( tmp, destFile );
	#endif
	}
    }

    fclose(destFile);
    rename("/tmp/new_one", "/tmp/nk_sysconfig");
    i = close(fd);
    i = unlink("/tmp/test.txt");
    return 1;
}

#if 0
int kd_readFlash(char * buf)
{
	FILE  *database, *ftmp;
	char  tpvalue[128];
	int i = 0, count;

    if ( NULL == (ftmp = fopen("/tmp/nk_sysconfig", "w+")))
        return 0;
	if ( NULL == (database = fopen("/etc/nk_sysconfig", "r+")))
	{
	     fclose(ftmp);
         return 0;
	}
    while (!feof(database))
    {
        count = fread (tpvalue, sizeof(char), 80, database);
        fwrite(tpvalue, sizeof(char), count, ftmp);
		i = i + count;
    }
	fclose(ftmp);
    fclose(database);

//	if (i <= 0)
//	    kd_Log("read nk_sysconfig fail!\n");
	
	return i;
}


int kd_updateFlash(int flag)
{
	FILE  *database, *ftmp;
	char  tpvalue[128];
	int i = 0, count;
	char cmdBuf[40];
	struct stat buf;

	//TT 20060115 check integrity of database, factory default if error -->  
	stat(SYS_FILE, &buf);
	sprintf(cmdBuf,"VERSION SIZE=%024d",buf.st_size);
	kd_doCommand(cmdBuf, CMD_WRITE, ASH_DO_NOTHING, (char *) NULL);
//	printf("%s \n", cmdBuf);
	// <---

    if ( NULL == (ftmp = fopen("/etc/nk_sysconfig", "w+")))
        return 0;
	if ( NULL == (database = fopen("/tmp/nk_sysconfig", "r+")))
	{
	     fclose(ftmp);
         return 0;
	}
    while (!feof(database))
    {
        count = fread (tpvalue, sizeof(char), 80, database);
        fwrite(tpvalue, sizeof(char), count, ftmp);
		i = i + count;
    }
	fclose(ftmp);
    fclose(database);

//	if (i <= 0)
//	    kd_Log("update nk_sysconfig fail!\n");
	
	return i;
}

void kd_eraseFlash(void)
{
	int fd;
	char buf[200];

	// add by johnli, restore to default
	sprintf(buf, "cd /etc; rm -fr .bak; mkdir .bak");
	//printf("%s\n",buf);
	system(buf);
	// end johnli
	//i = unlink("/etc/nk_sysconfig");
	//printf("unlink nk_sysconfig\n");
}
#endif
struct mtd_info_1 * kd_GetMtdImageInfo(char *sector)
{
    struct mtd_info_1 *head = NULL, **cur = &head , *next;
    int count;
    FILE *fd = NULL;
    char line[1024];

	if (mtd_head)
	{
		//printf("mtd_head[%s] sector[%s]\n",mtd_head->name,sector);
		if (!strncmp(mtd_head->name,sector,strlen(sector)))
			return mtd_head;
		else
		{
			next = mtd_head;
			/* free the mtd_head list*/
			for ( ; mtd_head ; mtd_head = next) {
				next = mtd_head->next;
				//printf("free %x\n",mtd_head);
				free(mtd_head);
			}
			mtd_head = NULL;
		}
	}

	if (!(fd = fopen(PROC_MTD, "r")))
		goto Exit;

	/* Run over /proc/mtd */
	fgets(line, sizeof(line), fd); /* Skip headers */
	while (fgets(line, sizeof(line), fd))
	{
		struct mtd_info_1 *tmp = (struct mtd_info_1 *)malloc(sizeof(struct mtd_info_1));

		if (!tmp)
		{
			printf("fail to alloc memory in kd_GetMtdInfo()");
			goto Exit;
		}

		memset(tmp,0,sizeof(struct mtd_info_1));
		count = sscanf(line, "mtd%d: %x %x %s",&tmp->index,&tmp->size,&tmp->erasesize,tmp->name);
		tmp->block_num = tmp->size / tmp->erasesize;

		if (!strncmp(tmp->name,sector,strlen(sector)))
		{
			*cur = tmp;
			cur = &tmp->next;
			//printf("alloc %x[%s]\n",tmp,tmp->name);
			continue;
		}

		free(tmp);
    }
Exit:
    if (fd)
		fclose(fd);
	mtd_head = head;
	return mtd_head;
}

unsigned int getmtderasesize(char *sector)
{
	struct mtd_info_1 *head;
	unsigned int ret;

	head = kd_GetMtdImageInfo(sector);
	while (head)
	{
		//printf("%d: size[%x] erasesize[%x] name[%s] block_num[%d] sector[%s][%d]\n",head->index,head->size,head->erasesize,head->name,head->block_num,sector,strlen(sector));
		if (!strncmp(head->name,sector,strlen(sector)))
		{
			ret = head->erasesize;
			return ret;
		}
		head = head->next;
	}
	return 0;
}

int getmtdbyblocknum(char *sector , unsigned int block_num , struct mtd_info_1 **ret_mtd , unsigned int *offset_blk)
{
	unsigned int i;
	struct mtd_info_1 *tmp;
	static char	name[] = START_IMAGE_SECT;	// "image1"
	i = 0;

	if (strncmp(IMAGE_SECT,sector,strlen(IMAGE_SECT)))
	{
		//printf("getmtdbyblocknum : name[%s]\n",sector);
		tmp = mtd_head;
		while (tmp)
		{
			if (!strncmp(tmp->name,sector,strlen(sector)))
			{
				if (block_num <= (tmp->block_num - 1))
				{
					*offset_blk = block_num;
					*ret_mtd = tmp;
					return 0;
				}
			}
			tmp = tmp->next;
		}
	}
	else
	{
tag:
		name[5] = '1' + i;
		//printf("getmtdbyblocknum : name[%s]\n",name);
		tmp = mtd_head;
		while (tmp)
		{
			if (!strncmp(tmp->name,name,strlen(name)))
			{
				//printf("block_num[%d] tmp->block_num[%d] name[%s] erasesize[%x]\n",block_num,tmp->block_num,tmp->name,tmp->erasesize);
				if (block_num <= (tmp->block_num - 1))
				{
					*offset_blk = block_num;
					*ret_mtd = tmp;
					return 0;
				}
				else
				{
					block_num = block_num - tmp->block_num;
					i ++;
					goto tag;
				}
			}
			tmp = tmp->next;
		}
	}
	*offset_blk = 0;
	*ret_mtd = 0;
	return -1;
}

int kd_EraseFlashOneBlock(unsigned int block_no)
{
	printf("kd_EraseFlashOneBlock not ready\n");
#if 0
	int fd , ret;
	mtd_info_t meminfo;
	struct mtd_info_1 *mtd;
	unsigned int offset , erasesize;
	char buffer[15];

	erasesize = getmtderasesize(IMAGE_SECT);
	ret = getmtdbyblocknum(IMAGE_SECT,block_no,&mtd,&offset);

	if (ret || !mtd)
	{
		printf("fail to get mtd information for %s\n",IMAGE_SECT);
		return -1;
	}
	//printf("block_no[%d] mtd->name[%s] index[%d] offset[%d]\n",block_no,mtd->name,mtd->index,offset);

	sprintf(buffer, "/dev/mtd%d",mtd->index);

	// Open and size the device
	if ((fd = open(buffer,O_RDWR)) < 0)
	{
		printf("File open error\n");
		return -1;
	}

	if (ioctl(fd,MEMGETINFO,&meminfo) == 0)
	{
		erase_info_t erase;

		erase.start = offset * erasesize;

		erase.length = meminfo.erasesize;

		// printf("Performing %s(mtd%d) Flash Erase of length %u at offset 0x%x\n",mtd->name,mtd->index,erase.length,erase.start);
		fflush(stdout);

		if (ioctl(fd,MEMERASE,&erase) != 0)
		{
			printf("\nMTD Erase failure");
			close(fd);
			return -1;
		}
	}
	close(fd);
	return 0;
#endif
}

int kd_BurnFlashOneBlock(unsigned int block_no, unsigned char *readBuf, unsigned int length)
{
	int fd , ret , err;
	struct mtd_info_1 *mtd;
	unsigned int offset , erasesize , writepos;
	char buffer[20];

	erasesize = getmtderasesize(IMAGE_SECT);
	ret = getmtdbyblocknum(IMAGE_SECT,block_no,&mtd,&offset);

	if (ret || !mtd)
	{
		printf("fail to get mtd information for %s\n",IMAGE_SECT);
		return -1;
	}

	writepos = offset * erasesize;
	sprintf(buffer, "/dev/mtdblock%d",mtd->index);
	//printf("file[%s] block_no[%d] mtd->name[%s] offset[%d]\n",buffer,block_no,mtd->name,offset);

	// Open and size the device
	if ((fd = open(buffer,O_SYNC|O_RDWR)) < 0)
	{
		printf("File open error\n");
		close (fd);
		return -1;
	}

	if (writepos != lseek (fd,writepos,SEEK_SET))
	{
		printf("lseek() error\n");
		close (fd);
		return -1;
	}

	err = write (fd,readBuf,length);
	if (err < 0)
	{
		printf("flash write() error\n");
		close (fd);
		return -1;
	}

	close (fd);
	return 0;
}

int kd_EraseFlashSector(char *sector , unsigned int block_no)
{
	printf("kd_EraseFlashSector not ready\n");
#if 0
	int fd , ret;
	mtd_info_t meminfo;
	struct mtd_info_1 *mtd;
	unsigned int offset , erasesize;
	char buffer[15];

	erasesize = getmtderasesize(sector);
	ret = getmtdbyblocknum(sector,block_no,&mtd,&offset);

	if (ret || !mtd)
	{
		printf("fail to get mtd information for %s\n",sector);
		return -1;
	}
	//printf("block_no[%d] mtd->name[%s] index[%d] offset[%d]\n",block_no,mtd->name,mtd->index,offset);

	sprintf(buffer, "/dev/mtd%d",mtd->index);

	// Open and size the device
	if ((fd = open(buffer,O_RDWR)) < 0)
	{
		printf("File open error\n");
		return -1;
	}

	if (ioctl(fd,MEMGETINFO,&meminfo) == 0)
	{
		erase_info_t erase;

		erase.start = offset * erasesize;

		erase.length = meminfo.erasesize;

		// printf("Performing %s(mtd%d) Flash Erase of length %u at offset 0x%x\n",mtd->name,mtd->index,erase.length,erase.start);
		fflush(stdout);

		if (ioctl(fd,MEMERASE,&erase) != 0)
		{
			printf("\nMTD Erase failure");
			close(fd);
			return -1;
		}
	}
	close(fd);
	return 0;
#endif
}

int kd_BurnFlash(char *sector , unsigned int block_no, unsigned char *readBuf, unsigned int block_offset , unsigned int length)
{
	int fd , ret , err;
	struct mtd_info_1 *mtd;
	unsigned int offset , erasesize , writepos;
	char buffer[20];

	erasesize = getmtderasesize(sector);
	ret = getmtdbyblocknum(sector,block_no,&mtd,&offset);

	if (ret || !mtd)
	{
		printf("fail to get mtd information for %s\n",sector);
		return -1;
	}

	writepos = offset * erasesize;
	//printf("writepos[%x]\n",writepos);
	if (block_offset)
		writepos = writepos + block_offset;
	sprintf(buffer, "/dev/mtdblock%d",mtd->index);
	//printf("file[%s] block_no[%d] mtd->name[%s] writepos[%x] boffset[%x] readbuf[%x] lenth[%d]\n",buffer,block_no,mtd->name,writepos,block_offset,readBuf,length);

	// Open and size the device
	if ((fd = open(buffer,O_SYNC|O_RDWR)) < 0)
	{
		printf("File open error\n");
		close (fd);
		return -1;
	}

	if (writepos != lseek (fd,writepos,SEEK_SET))
	{
		printf("lseek() error\n");
		close (fd);
		return -1;
	}

	err = write (fd,readBuf,length);
	if (err < 0)
	{
		printf("flash write() error\n");
		close (fd);
		return -1;
	}

	close (fd);
	return 0;
}

int kd_ReadFlashSector(char *sector , unsigned int offset , unsigned int length , unsigned char *readBuf)
{
	int fd , ret , err = 0;
	struct mtd_info_1 *mtd;
	unsigned int tmp_offset , erasesize;
	char buffer[20];

	/* this function is used by reading pmon and factory information only. */
	/* the block_no should be modify if want to support image sector */
	if (!strncmp(BOOT_SECT,sector,strlen(BOOT_SECT)))
	{
		erasesize = getmtderasesize(sector);
		ret = getmtdbyblocknum(sector,0,&mtd,&tmp_offset);
	}
	else
		return -1;

	if (ret || !mtd)
	{
		printf("fail to get mtd information for %s\n",sector);
		return -1;
	}

	sprintf(buffer, "/dev/mtdblock%d",mtd->index);
	//printf("file[%s] mtd->name[%s] offset[%x] length[%d]\n",buffer,mtd->name,offset,length);

	// Open and size the device
	if ((fd = open(buffer,O_SYNC|O_RDONLY)) < 0)
	{
		printf("File open error\n");
		close (fd);
		return -1;
	}

	if (offset != lseek (fd,offset,SEEK_SET))
		printf("lseek()\n");

	err = read(fd,readBuf,length);
	if (err < 0)
	{
		printf("File read error\n");
		close(fd);
		return -1;
	}
	close(fd);
	return err;
}
//---------------------use MTD util---->>2007/04/10 Ryoko
#ifndef CONFIG_NK_BY_LIBC
int kd_BurnFW(char *FileName,unsigned int len)//return 1 success ,0 fail
{
	char cmdBuf[200];
	int erase_block;
#ifdef CONFIG_TWO_FLASH_BANKS
	FILE *ftmp, *database;
	char tmp[120];
	int  count;
#endif
	struct stat buf;
	printf("kd_BurnFW FileName=%s,len=0x%x\n",FileName,len);
//	sprintf(cmdBuf,"mtd_debug erase /dev/mtd1 0x0 0x1e00000");
	erase_block = len / 0x20000;
	erase_block++;
	if(30*8 <= erase_block)
		erase_block =30*8;
//--> Jerry 2009/05/12 added. Print messages to console in order to know the upgrade status.
#ifdef CONFIG_TWO_FLASH_BANKS
	con_printf("Flash: Erasing flash block ... (1 of 2)\r\n");
#else
	con_printf("Flash: Erasing flash block ...\r\n");
#endif
//<--
	sprintf(cmdBuf,"mtd_debug erase %s %s 0x%x",nk_IMAGE_MTD,nk_IMAGE_MTD_OFFSET,erase_block*0x20000);
//	sprintf(cmdBuf,"mtd_debug erase %s %s %s",nk_IMAGE_MTD,nk_IMAGE_MTD_OFFEST,nk_IMAGE_MTD_LEN);
	printf("%s\n",cmdBuf);
	system(cmdBuf);
#ifndef CONFIG_TWO_FLASH_BANKS
//--> Jerry 2009/05/12 added. Print messages to console in order to know the upgrade status.
	con_printf("Flash: Writing image to flash ...\r\n");
//<--
	stat(FileName,&buf);
	sprintf(cmdBuf,"mtd_debug write %s %s 0x%x %s",nk_IMAGE_MTD,nk_IMAGE_MTD_OFFSET,buf.st_size,FileName);
	printf("%s\n",cmdBuf);
	system(cmdBuf);
#else
	if (len <= 0x1e00000) //--> TT 20081226, nk_IMAGE_MTD size
	{
//--> Jerry 2009/05/12 added. Print messages to console in order to know the upgrade status.
		con_printf("Flash: Writing image to flash ...\r\n");
//<--
		sprintf(cmdBuf,"mtd_debug write %s %s 0x%x %s",nk_IMAGE_MTD,nk_IMAGE_MTD_OFFSET,len,FileName);
		printf("%s\n",cmdBuf);
		system(cmdBuf);
	}
	else
	{
//--> Jerry 2009/05/12 added. Print messages to console in order to know the upgrade status.
		con_printf("Flash: Writing image to flash ... (1 of 2)\r\n");
//<--
//--> TT 20081226, burn to nk_IMAGE_MTD
		sprintf(cmdBuf,"mtd_debug write %s %s 0x%x %s",nk_IMAGE_MTD,nk_IMAGE_MTD_OFFSET,0x1e00000,FileName);
		printf("%s\n",cmdBuf);
		system(cmdBuf);

//--> Jerry 2009/05/12 added. Print messages to console in order to know the upgrade status.
		con_printf("Flash: Erasing flash block ... (2 of 2)\r\n");
//<--
//--> TT 20081226, erase nk_IMAGE2_MTD
		sprintf(cmdBuf,"mtd_debug erase %s %s 0x%x",nk_IMAGE2_MTD,nk_IMAGE_MTD_OFFSET,(((len-0x1e00000)/0x20000+1)*0x20000));
		printf("%s\n",cmdBuf);
		system(cmdBuf);

		//--> TT 20081226, strip content of file before nk_IMAGE_MTD_LEN(0x1e00000)
		if ( NULL == (database = fopen(FileName, "r+")))
		{
			return (0);
		}
		if ( 0 != fseek(database, 0x1e00000, SEEK_SET))
		{
			fflush(database);
			fclose(database);
			return (0);
		}
		if ( NULL == (ftmp = fopen("/tmp/f1.img", "w+")))
		{
			return (0);
		}

		while (!feof(database))
		{
			count=fread (tmp, sizeof(char), 40, database);
			fwrite(tmp, sizeof(char), count, ftmp);
		}
		fclose(database);
		fclose(ftmp);

//--> Jerry 2009/05/12 added. Print messages to console in order to know the upgrade status.
		con_printf("Flash: Writing image to flash ... (2 of 2)\r\n");
//<--
//--> TT 20081226, burn to nk_IMAGE2_MTD
		sprintf(cmdBuf,"mtd_debug write %s %s 0x%x %s",nk_IMAGE2_MTD,nk_IMAGE_MTD_OFFSET,len-0x1e00000,"/tmp/f1.img");
		printf("%s\n",cmdBuf);
		system(cmdBuf);

	}
#endif
}

int kd_BurnLoader(char *FileName,unsigned int len)//return 1 success ,0 fail
{
	char cmdBuf[200];
	printf("kd_BurnLoader FileName=%s,len=0x%x\n",FileName,len);
//	sprintf(cmdBuf,"mtd_debug erase /dev/mtd0 0x0 0x80000");
	sprintf(cmdBuf,"mtd_debug erase %s %s %s",nk_LOADER_MTD,nk_LOADER_MTD_OFFSET,nk_LOADER_MTD_LEN);
	printf("%s\n",cmdBuf);
	system(cmdBuf);
	sprintf(cmdBuf,"mtd_debug write %s %s 0x%x %s",nk_LOADER_MTD,nk_LOADER_MTD_OFFSET,len,FileName);
	printf("%s\n",cmdBuf);
	system(cmdBuf);
}

int kd_BurnFactory(char *FileName,unsigned int len)//return 1 success ,0 fail
{
	char cmdBuf[200];
	printf("kd_BurnLoader FileName=%s,len=0x%x\n",FileName,len);
//	sprintf(cmdBuf,"mtd_debug erase /dev/mtd0 0x0 0x80000");
	sprintf(cmdBuf,"mtd_debug erase %s %s %s",nk_FACTORY_MTD,nk_FACTORY_MTD_OFFSET,nk_FACTORY_MTD_LEN);
	printf("%s\n",cmdBuf);
	system(cmdBuf);
	sprintf(cmdBuf,"mtd_debug write %s %s 0x%x %s",nk_FACTORY_MTD,nk_FACTORY_MTD_OFFSET,len,FileName);
	printf("%s\n",cmdBuf);
	system(cmdBuf);
}

// --> Jerry 2009/04/13 modified. In order to handle error, 
//     define the return value 0 as successful and else as failed.
//void nk_strip_header(char *FileName)
int nk_strip_header(char *FileName)
//<--
{
//--> Jerry 2009/04/13 modified for handling error. change path for security reason
//#define tempfile "/tmp/upgrade_file_temp"
#define tempfile "/tmp/tmpfs/upgrade_file_temp"
//<--

	FILE *source;
	FILE *temp;
	int sfd;
	char buf[120];
	int  count;
	//--> Jerry 2009/04/13 added for handling error
	int ret;
	//<--

	if( (source = fopen(FileName, "r")) == NULL) {
		printf("can't open source file\n");
		return -1;
	}
	if( (sfd = open(tempfile,O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR | S_IWUSR)) == -1) {
		printf("can't open temp file\n");
		return -1;
	}
	if( (temp = fdopen(sfd, "w")) == NULL) {
		printf("fdopen temp file failed\n");
		return -1;
	}

	fseek(source, sizeof(struct image_header),SEEK_SET);
	while (!feof(source))
	{
		count = fread (buf, sizeof(char), 40, source);
		fwrite(buf, sizeof(char), count, temp);
	}
	/*del source file, replace temp file */
	fclose(source);
	fclose(temp);

	//--> Jerry 2009/04/13 modified for handling error
	//rename(tempfile, FileName);
	ret = rename(tempfile, FileName);
	if(ret != 0)
		con_printf("Error: failed to rename file.\r\n");
	return ret;
	//<--

}
#endif
int kd_readFlash(char * buf)
{
	char cmdBuf[200];
	sprintf(cmdBuf,"cp -f %s%s %s%s",DB_FLASH,DB_FILENAME,DB_RAM,DB_FILENAME);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);
}
#if 0 /*purpose:0013264, author:selena, description:new log backup path*/
int kd_readFlash_var_log(char * buf)
{
	char cmdBuf[200];
	sprintf(cmdBuf,"cp -f %s* %s",VAR_LOG_FLASH,VAR_LOG_RAM);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);
}
#endif

long get_current_time_for_time_stamp(char* current_t,int type)
{
	static char buf[50];
	int i=0;
	struct timeval	tv;
	struct timezone	tz;
	struct tm		tm;
	struct tm tm_time;
	gettimeofday(&tv, &tz);
	
	kd_doCommand("SYSTEM TIMEZONE", 3, 0, buf);
	for (i=0; time_zones[i].name; i++)
	{
		if ( strlen(buf) && !strncmp(buf, time_zones[i].name, strlen(time_zones[i].name)) )
			break;
	}
	
	tv.tv_sec = tv.tv_sec + time_zones[i].gmt_offset * 3600;
	
	kd_doCommand("SYSTEM NTPSTATUS", 3, 0, buf);
	if(!strcmp(buf,"YES"))
	{
		// daylight saving
		kd_doCommand("SYSTEM DAYLIGHTSAVING", 3, 0, buf);	
		if(!strcmp(buf,"YES"))
		{
			int smonth,sday,emonth,eday;
			int sDateValue=0,eDateValue=0,nowDateValue=0; // formate=%2d%2d(month,day)
	
			kd_doCommand("SYSTEM DAYLIGHTSTARTDATE", 3, 0, buf);		
			sscanf(buf,"%d:%d",&smonth,&sday);
	
			kd_doCommand("SYSTEM DAYLIGHTENDDATE", 3, 0, buf);	
			sscanf(buf,"%d:%d",&emonth,&eday);
	
			memcpy(&tm, localtime(&tv.tv_sec), sizeof(struct tm));
	
			sDateValue=smonth*100+sday;
			eDateValue=emonth*100+eday;
			nowDateValue=(tm.tm_mon+1)*100+tm.tm_mday;
	
			if( ((sDateValue <= eDateValue) && ((sDateValue<=nowDateValue)&&(nowDateValue<=eDateValue))) ||
					( (sDateValue > eDateValue) && ((nowDateValue > sDateValue) ||(nowDateValue<eDateValue))) )
			{
				tv.tv_sec = tv.tv_sec + 3600;
			}
		}
	}
	memcpy(&tm_time, localtime(&tv.tv_sec), sizeof(struct tm));
	switch(type)
	{
		case  0: // EX:091016
			sprintf(current_t, "%02d%02d%02d", (tm_time.tm_year+1900)%100, (tm_time.tm_mon+1), tm_time.tm_mday);
			break;
		case  1: // EX:2009-10-16
			sprintf(current_t, "%04d-%02d-%02d", (tm_time.tm_year+1900), (tm_time.tm_mon+1), tm_time.tm_mday);
			break;
		case  2: // registr_time
			return tv.tv_sec;
			break;
	}
	return 0;
}

int kd_updateFlash(int flag)
{
	char cmdBuf[200];
//	sprintf(cmdBuf,"mkdir %s",DB_FLASH);
//	printf("%s\n",cmdBuf);
//	system(cmdBuf);

#ifdef CONFIG_MODEL_RV0XX
	char db_buf[100],registr_time[20];

    if(flag==USER_CHANGE_DB)
    {
	    sprintf(db_buf, "SYSTEM CONFIG_TIME_STAMP=%ld", get_current_time_for_time_stamp(&registr_time,2));
	    kd_doCommand(db_buf, 1, 0, NULL);
    }
#endif
	MergeToRAM();

#ifdef CONFIG_MODEL_RV0XX //oolong 03/09
	sprintf(cmdBuf,"cp -f %s%s %s%s.temp; mv %s%s.temp %s%s",DB_RAM,DB_FILENAME,DB_FLASH,DB_FILENAME,DB_FLASH,DB_FILENAME,DB_FLASH,DB_FILENAME);
#else
	sprintf(cmdBuf,"cp -f %s%s %s%s",DB_RAM,DB_FILENAME,DB_FLASH,DB_FILENAME);
#endif	
//	printf("%s\n",cmdBuf);
	system(cmdBuf);

#ifdef NK_CONFIG_EASYACCESS
	system("cp -f /usr/local/EasyAccess/var/conf/* /etc/flash/etc");
#endif

}
#if 0 /*purpose:0013264, author:selena, description:new log backup path*/
int kd_updateFlash_var_log(void)
{
	char cmdBuf[200];

	sprintf(cmdBuf,"mkdir %s",VAR_FLASH);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);

	sprintf(cmdBuf,"mkdir %s",VAR_LOG_FLASH);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);

	sprintf(cmdBuf,"cp -f %s* %s",VAR_LOG_RAM,VAR_LOG_FLASH);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);
}
#endif
void kd_eraseFlash(void)
{
	char cmdBuf[200];
#if 0 /*purpose:0013264, author:selena, description:new log backup path*/
	sprintf(cmdBuf,"rm -rf %s",VAR_LOG_FLASH);
	printf("%s\n",cmdBuf);
	system(cmdBuf);
#endif

#ifdef NK_CONFIG_EASYACCESS
	system("rm -rf /etc/flash/etc");
#else
	sprintf(cmdBuf,"mkdir %s",DB_FLASH);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);
	sprintf(cmdBuf,"cp -f %s%s %s%s",DB_FACTORY,DB_FILENAME,DB_FLASH,DB_FILENAME);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);
#endif
}
int kd_updateFlash_factory(char *write_buf,unsigned offest,unsigned len)
{
	char cmdBuf[200];
	char FileName[]="/tmp/.factory";
//	printf("kd_factory write_buf=%s,offest=0x%x,len=0x%x\n",write_buf,offest,len);
//	sprintf(cmdBuf,"mtd_debug erase /dev/mtd1 0x0 0x1e00000");
	if(strlen(write_buf)<=len)
		len=strlen(write_buf);
	sprintf(nk_rg_factory,"%s",write_buf);
	nk_rg_factory_write_file_fun();
	sprintf(cmdBuf,"mtd_debug erase %s %s %s",nk_FACTORY_MTD,nk_FACTORY_MTD_OFFSET,nk_FACTORY_MTD_LEN);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);
	sprintf(cmdBuf,"mtd_debug write %s 0x%x 0x%x %s",nk_FACTORY_MTD,offest,len,FileName);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);
	return 1;
}
char* kd_readFlash_factory(unsigned offest,unsigned len)
{
	char cmdBuf[200];
	char FileName[]="/tmp/.factory";
//	printf("kd_factory read=offest=0x%x,len=0x%x\n",offest,len);
//	sprintf(cmdBuf,"mtd_debug erase /dev/mtd1 0x0 0x1e00000");
	sprintf(cmdBuf,"mtd_debug read %s 0x%x 0x%x %s",nk_FACTORY_MTD,offest,len,FileName);
//	printf("%s\n",cmdBuf);
	system(cmdBuf);
	nk_rg_factory_read_file_fun(len);
	return nk_rg_factory;
}
//<<---------------------copy by MTD util---
