#include <stdio.h>
#include <stdlib.h>


/*
	CiA DS-301, p.20
*/
#define MAX_BIT_TIMING 9
int table_bit_timing_settings[MAX_BIT_TIMING][3] =
{
	{1000, 8, 6}, /* baudrate, number of time quanta per bit, tsync+tseg1 */
	{800, 10, 8},
	{500, 16, 14},
	{250, 16, 14},
	{125, 16, 14},
	{100, 16, 14},
	{ 50, 16, 14},
	{ 25, 16, 14},
	{ 10, 16, 14}
};


void can_timing_registers(double f, int *v)
/* fill the vector v with the proper setting for TIMER 0 and TIMER 1
   regarding the clock and the baudrate */
{
	int i;

	int BRP, TSEG1, TSEG2;

	double nominal, tq, tscl;

	double tclk = 1 / (f*1e6); /* sec */

	for(i=0; i<MAX_BIT_TIMING; i++)
	{
		nominal = 1. / (table_bit_timing_settings[i][0]*1e3); /* nominal bit time */

		tq = nominal / table_bit_timing_settings[i][1]; /* time quanta */

		tscl = tq; /* Tsyncseg = Tscl (ref. SJA 1000 datasheet, p.51) */

		/* tcsl = 2 tclk (BRP + 1) */
		BRP = (int)(tscl / (2. * tclk) - 1.);

		/* 
			BRT0 = SJW * 64 + BRP 
			SJW : synchonisation jump Width, user defined (0..3)
		*/

		/* tseg1 = tscl * (TSEG1 + 1) */
		TSEG1 = (int)((table_bit_timing_settings[i][2] - 1)*tq/tscl - 1.);

		/* tseg2 = tscl * (TSEG2 + 1) */
		TSEG2 = (int)(2.*tq/tscl - 1.);

		/*
			BRT1 = SAM*128 + TSEG2*16 + TSEG1
			SAM = Sampling (0 = single sampling, 1 = triple sampling)
		*/

		if (BRP < 64)
		{
			v[2*i] = BRP;
			v[2*i+1] = TSEG1+16*TSEG2;
		}
		else
		{
			v[2*i] = 0;
			v[2*i+1] = 0;
		}
	}
}


void print_header(double frequency, /* clock rate in MHz*/
                  int    sjw)       /* 0..3 */
{
	int i, array[18];
	FILE *f = fopen("baudrate_table.h", "w");

	can_timing_registers(frequency, array);

	fprintf(f, "#if !defined(_BAUDRATE_TABLE_H_)\n");
	fprintf(f, "#define _BAUDRATE_TABLE_H_\n");
	fprintf(f, "\n");
	fprintf(f, "/*\n");
	fprintf(f, "this file must be include only once in the project\n");
	fprintf(f, "*/\n");
	fprintf(f, "\n");
	fprintf(f, "/* clock speed in MHz */\n");
	fprintf(f, "#define CAN_CONTROLER_CLOCK_SPEED %.3lf\n", frequency);
	fprintf(f, "#define CAN_CONTROLER_PHASE_SHIFT_TOLERANCE %d\n", sjw);

	sjw = sjw << 6;

	fprintf(f, "\n");
	fprintf(f, "static int can_baudrate_registers[9][3] =\n");
	fprintf(f, "{  /* BTR0    BTR1 */\n");

	for(i=0; i<MAX_BIT_TIMING; i++)
	{
		if (array[2*i] == 0  && array[2*i+1] == 0)
			fprintf(f, "    {0, 0x00, 0x00}, /* %4d kbits/s -- out of range*/\n", 
				table_bit_timing_settings[i][0]);
		else
			fprintf(f, "    {1, 0x%02x, 0x%02x}, /* %4d kbits/s */\n", 
				sjw|array[2*i], array[2*i+1],
				table_bit_timing_settings[i][0]);
	}

	fprintf(f, "};\n");
	fprintf(f, "\n");
	fprintf(f, "#endif\n\n");

	fclose(f);
}


int main(int argc, char *argv[])
{	
	if (argc == 3)
		print_header(atof(argv[1]), atoi(argv[2]));
	else
		printf("usage: %s clock_in_Mhz error_rate_in_percent\n", argv[0]);
}
