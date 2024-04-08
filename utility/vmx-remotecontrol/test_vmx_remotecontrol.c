#include "vmx_remotectrl.h"

void print_usage()
{
	printf("Usage of vmx_remotectrl_test:\n");
	printf("/usr/bin/vmx_remotectrl_test -b <bus_id> -i 0x<slave_address> \
			-f <firmware_image_to_write> -t 0/1 -v 0/1\n");
	printf("-t 1 - for getting the core chip temperature, 0 - otherwise\n");
	printf("-v 1 - for getting the firmware version, 0 - otherwise\n");
}

int main(int argc, char *argv[])
{
	int i2c_devId = DEFAULT_I2C_BUS_ID;
	int i2c_slave_addr = DEFAULT_I2C_SLAVE_ADDRESS;
	int ret = 0;
	int iTemp = 0;
	int iGetFWVersion = 0;
	int iGetChipTemp = 0;
	int major, minor, build;
	major = minor = build = -1;
	FILE *fw_fp = NULL;
	int res;
	struct vmx_remctl *test_vmx_remctl;
	char ops[30] = "b:i:f:t:v:";

	// Parse input arguments
	while ((res = getopt(argc, argv, ops)) >= 0) {
		switch (res) {
			case 'b':
				i2c_devId = atoi(optarg);
				break;
			case 'i':
				i2c_slave_addr = strtol(optarg, NULL ,16);
				break;
			case 'f':
				fw_fp = fopen(optarg,"rb");
				break;
			case 't':
				iGetChipTemp = atoi(optarg);
				break;
			case 'v':
				iGetFWVersion = atoi(optarg);
				break;
			default:
				print_usage();
				printf("unsupport arg %c\n",res);
				return -1;
		}
	}

	test_vmx_remctl = syna_vmxrc_open_device(i2c_devId, i2c_slave_addr);
	if (test_vmx_remctl == NULL)
	{
		printf("syna_vmxrc_open_device Failed\n");
		return -1;
	}

	if (iGetChipTemp)
	{
		ret = syna_vmxrc_get_chip_temperature(test_vmx_remctl, &iTemp);
		if (ret != 0)
		{
			printf("syna_vmxrc_get_chip_temperature Failed\n");
			return -1;
		}
		else
		{
			printf("Chip Core Temperatire is %d\n", iTemp);
		}
	}

	// User needs to power off and power on after the flash is completed
	// only then the fw version fetched will be of the one flashed earliest.
	if (iGetFWVersion)
	{
		ret = syna_vmxrc_get_current_firmware_version(test_vmx_remctl, &major, &minor, &build);
		if (ret != 0)
		{
			printf("syna_vmxrc_get_current_firmware_version Failed\n");
			return -1;
		}
		else
		{
			printf("FW Version Before Writing Flash Data is %2d.%2d.%2d\n", major, minor, build);
		}
	}

	if (fw_fp != NULL)
	{
		ret = syna_vmxrc_write_flash_data_command(test_vmx_remctl, fw_fp);
		if (ret != 0)
		{
			printf("syna_vmxrc_write_flash_data_command Failed\n");
			return -1;
		}
	}

	ret = syna_vmxrc_close_device(test_vmx_remctl);
	if (ret != 0)
	{
		printf("syna_vmxrc_close_device Failed\n");
		return -1;
	}

	return 0;
}

