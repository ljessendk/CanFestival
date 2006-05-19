#include <stdio.h>


/* size of NVRAM in bytes */
#define NVRAM_MAX_SIZE 262144

/* size of block in byte */
#define NVRAM_BLOCK_SIZE 256

#define NVRAM_FILE_NAME "__nvram__"

FILE *nvram_file = NULL;


void iat_flash_read_regs();
void iat_flash_write_regs();
void iat_flash_write_page(unsigned int);
void iat_flash_read_page(unsigned int);


short data_len; /* 0 to NVRAM_BLOCK_SIZE bytes */
short data_num_pages;
unsigned int *data_page = NULL;
unsigned int data_addr;

unsigned int *regs_page = NULL;

int iat_init()
{
	int i;

	nvram_file = fopen(NVRAM_FILE_NAME, "wr");
	if (nvram_file  == NULL)
		return -1;

	int n = NVRAM_BLOCK_SIZE / sizeof(unsigned int);

	/* some actions to initialise the flash */
	data_len = 0;
	data_num_pages = 0;

	data_page = (unsigned int *)malloc(sizeof(unsigned int) * n);
	memset(data_page, 0, sizeof(unsigned int)*n);

	if (data_page == NULL)
		return -1;

	regs_page = (unsigned int *)malloc(sizeof(unsigned int) * n);
	memset(regs_page, 0, sizeof(unsigned int)*n);
	if (regs_page == NULL)
		return -2;

	iat_flash_read_regs();

	/* start the data at the location specified in the registers */ 
	if (0) /* for now it is 0, but put here a test to know whether
                  or not the NVRAM has been written before */
		data_addr = regs_page[1];
	else
		data_addr = NVRAM_BLOCK_SIZE; /* let start at block 1 */

	/* create a file the size of the simulated NVRAM */
	for(i=0; i<NVRAM_MAX_SIZE/NVRAM_BLOCK_SIZE + 1; i++)
		fwrite(data_page, sizeof(unsigned int), n, nvram_file);

	return 0;
}


void iat_end()
{
	iat_flash_write_page(data_addr);

	iat_flash_write_regs();

	fclose(nvram_file);
}

 
void iat_flash_read_regs()
{
	fseek(nvram_file, 0, SEEK_SET);
	fread(regs_page, sizeof(unsigned int), NVRAM_BLOCK_SIZE, nvram_file);
}


void iat_flash_write_regs()
{
	fseek(nvram_file, 0, SEEK_SET);
	fwrite(regs_page, sizeof(unsigned int), NVRAM_BLOCK_SIZE, nvram_file);
}


void iat_flash_read_page(unsigned int addr)
{
	fseek(nvram_file, addr*sizeof(unsigned int), SEEK_SET);
	fread(data_page, sizeof(unsigned int), NVRAM_BLOCK_SIZE, nvram_file);
}


void iat_flash_write_page(unsigned int addr)
{
	fseek(nvram_file, addr*sizeof(unsigned int), SEEK_SET);
	fwrite(data_page, sizeof(unsigned int), NVRAM_BLOCK_SIZE, nvram_file);
}


/*------------------------------------------------------*/
int main()
{
	iat_init();
	iat_end();
}
/*------------------------------------------------------*/
