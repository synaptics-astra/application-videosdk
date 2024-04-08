#include "vmx_remotectrl.h"
//#define DEBUG_READ_WRITE

// The unit size in which unit/size the write operation takes place for FW Flash
#define VMM_UNIT_SIZE	32

// The Command data that describes the Command set to be triggered
#define I2C_REMOTE_CONTROL_COMMAND_MSG 0xF2
// The Command data that describes the Chip reset Command to be triggered
#define CHIP_RESET_CODE 0xF5

// Currently only 0x50000 bytes of the actual FW fullrom is required to be
// flashed during the WRITE_FLASH COMMMAND
#define FW_LENGTH 0x50000

// Register Addresses
#define RC_TRIGGER_REGISTER		0x2020021C
#define RC_COMMAND_REGISTER		0x20200280
#define RC_OFFSET_REGISTER		0x20200284
#define RC_LENGTH_REGISTER		0x20200288
#define RC_DATA_REGISTER		0x20200290
#define FW_VERSION_REGISTER		0x20200230

#define RC_COMMAND_ACTIVE_LOW_SET 0x80
#define RC_COMMAND_WRITE_PRIUS_TO_CHIP "PRIUS"
#define RC_COMMAND_WRITE_PRIUS_TO_CHIP_LENGTH 5

// Sleep / wait paramters
#define WAIT_TIME_AFTER_ERASE_FLASH 3
#define WAIT_TIME_AFTER_WRITE_FLASH 1
#define WAIT_TIME_ON_RC_ACTIVE_LOW (50 * 1000)
// This is currenlty set to be a multiple of 50ms
// So at max 40*50ms = 2s wait after that it Times Out
#define WAIT_FOR_RC_ACTIVE_LOW_CNT 40

// Variables used for updating the FW Flash Progress
int update_per, prev_update_per;

// RC COMMANDS - CURRENTLY SUPPORTED
// Commands not supported for now
//VMM_WRITE_MEMORY = 0x21,
//VMM_READ_MEMORY = 0x31,
typedef enum
{
	VMM_RC_ENABLE = 0x1,
	VMM_RC_DISABLE = 0x2,
	VMM_FW_CHECKSUM_CALC = 0x11,
	VMM_ERASE_FLASH = 0x14,
	VMM_ACTIVATE_FW = 0x18,
	VMM_WRITE_FLASH_DATA = 0x20,
	VMM_READ_FLASH_DATA = 0x30,
	VMM_GET_CHIP_CORE_TEMPERATURE = 0x64
}VMM_CMD;

typedef enum
{
	RC_RET_SUCCESS = 0,
	RC_RET_INVALID_ARGUMENT,
	RC_RET_UNSUPPORTED_COMMAND,
	RC_RET_UNKNOWN_ERROR,
	RC_RET_RC_INTERFACE_DISABLED,
	RC_RET_CONFIG_SIGN_FAILED,
	RC_RET_FW_SIGN_FAILED,
	RC_RET_ROLLBACK_FAILED
}RC_RET;

struct vmx_remctl * syna_vmxrc_open_device(int i2c_dev_id, int slave_address)
{
	struct vmx_remctl *vmxrc;
	int ret = 0;
	char filename[20];

	// Populate the data in the global structure
	vmxrc = (struct vmx_remctl *) malloc(sizeof(vmx_remctl));
	if (vmxrc == NULL)
	{
		printf("Allocation failed for vmx_remctl");
		return NULL;
	}
	vmxrc->i2c_bus_id = i2c_dev_id;
	vmxrc->i2c_slave_address = slave_address;

	snprintf(filename, 19, "/dev/i2c-%d", i2c_dev_id);
	vmxrc->i2c_file = open(filename, O_RDWR);
	if (vmxrc->i2c_file < 0) {
		/* ERROR HANDLING; you can check errno to see what went wrong */
		free(vmxrc);
		printf("I2C Device Open Failed. Errno : %d\n", errno);
		return NULL;
	}
	printf("I2C DEVICE OPEN\n");

	if (ioctl(vmxrc->i2c_file, I2C_SLAVE, vmxrc->i2c_slave_address) < 0) {
		/* ERROR HANDLING; you can check errno to see what went wrong */
		printf("I2C Device Set Slave Address Failed. Errno : %d\n", errno);
		free(vmxrc);
		return NULL;
	}

	// Disable RC COMMAND
	ret = syna_i2c_disable_rc_cmd(vmxrc->i2c_file);
	if (ret != 0)
	{
		printf("RC Disable Cmd Failed with error : %d\n", ret);
		printf("Proceeding with ENABLE RC COMMAND, as by default the Remote Control feature will be disabled state\n");
	}

	// Enable RC COMMAND
	ret = syna_i2c_enable_rc_cmd(vmxrc->i2c_file);
	if (ret != 0)
	{
		printf("RC Enable Cmd Failed with error : %d\n", ret);
		free(vmxrc);
		return NULL;
	}

	return vmxrc;
}

// Read from Register
int syna_vmx_read_from_reg(int file, long int regAddr, char *rData, int len)
{
	char buf[10];
	//NOTE: need to update in case we want to read the flash data
	char readBuf[4096];
	buf[0] = (regAddr & 0xFF);
	buf[1] = (regAddr >> 8) & 0xFF;
	buf[2] = (regAddr >> 16) & 0xFF;
	buf[3] = (regAddr >> 24) & 0xFF;
	if (write(file, buf, 4) != 4) {
		/* ERROR HANDLING: I2C transaction failed */
		printf("Write Reg Addr Failed: %d\n", errno);
		return -1;
	}
	if (read(file, readBuf, len) == len) {
		//printf("RC_DATA : 0x%x\n", ((readBuf[3] << 24) | (readBuf[2] << 16) | (readBuf[1] << 8) | (readBuf[0])));
		for (int i = 0; i < len; i++)
			rData[i] = readBuf[i];
	}
	else
		return -1;
	return 0;
}

// Write to Register
int syna_vmx_write_to_reg(int file, long int regAddr, char *wData, int len)
{
	char buf[4096];
	int ret = 0;
	buf[0] = (regAddr & 0xFF);
	buf[1] = (regAddr >> 8) & 0xFF;
	buf[2] = (regAddr >> 16) & 0xFF;
	buf[3] = (regAddr >> 24) & 0xFF;

	memcpy(&buf[4], wData, len);
	if (write(file, buf, (len+4)) != (len+4)) {
		/* ERROR HANDLING: I2C transaction failed */
		printf("Write Reg Addr Failed: %d\n", errno);
		return -1;
	}
	return 0;
}

// Wait till the Active bit goes low (bit[23]) of RC_Command Register
// indicating a complete transaction.
// Do not forget to check the bit[31:24] for the final results of the
// requested command in the RC_Command Register
int syna_waitOnRC_Command_ActiveLow(int file)
{
	// TODO: Optimize the sleep time
	int cnt = WAIT_FOR_RC_ACTIVE_LOW_CNT; //with a 50ms sleep this counter corresponds to 2s
	int ret = 0;
	char buf[10];
	char readBuf[4096];

	// Wait on RC_COMMAND's Active bit going low
	while (cnt)
	{
		ret = syna_vmx_read_from_reg(file, RC_COMMAND_REGISTER, readBuf, 4);
		if (ret != 0)
			return -1;

		if (readBuf[2] & RC_COMMAND_ACTIVE_LOW_SET)
		{
			//printf("Retry RC DISABLE cmd\n");
			cnt--;
			usleep(WAIT_TIME_ON_RC_ACTIVE_LOW);
			continue;
		}
		else
		{
			break;
		}
	}

	if (!cnt)
	{
		printf("%s Timed Out\n", __func__);
		return -1;
	}
	else if ((readBuf[3] & 0xFF) == 0)
	{
		//printf("RC_COMMAND Status : 0x%x\n", ((readBuf[3] << 24) | (readBuf[2] << 16) | (readBuf[1] << 8) | (readBuf[0])));
		return 0;
	}
	else
	{
		printf("RC COMMAND Error Status : 0x%x\n", (readBuf[3] & 0xFF));
		return -1;
	}
}

// Function to Disable Remote Control (RC) feature
int syna_i2c_disable_rc_cmd(int file)
{
	char buf[10];
	char readBuf[4096];
	int ret = 0;
	// DISABLE RC COMMAND
	int cmd = VMM_RC_DISABLE;
	int active = 1;
	unsigned int data = (cmd << 16) | (active << 23);

	printf ("\nProcess : CMD : RC DISABLE\n");

	memcpy(buf, (char *)(&data), 4);
	ret = syna_vmx_write_to_reg(file, RC_COMMAND_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// Set RC_TRIGGER
	memset(buf, 0, sizeof (buf));
	buf[0] = I2C_REMOTE_CONTROL_COMMAND_MSG;
	ret = syna_vmx_write_to_reg(file, RC_TRIGGER_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	ret = syna_waitOnRC_Command_ActiveLow(file);
	if (ret == 0)
		printf("Disable RC Command Complete\n");
	else
		printf("Disable RC Command Failed\n");

	return ret;
}

// Function to Enable Remote Control (RC) feature
int syna_i2c_enable_rc_cmd(int file)
{
	int ret;
	char buf[10] = {0};
	int cnt = RC_COMMAND_WRITE_PRIUS_TO_CHIP_LENGTH;
	int cmd = VMM_RC_ENABLE;
	char *pData = RC_COMMAND_WRITE_PRIUS_TO_CHIP;
	int active = 1;
	unsigned int data = (cmd << 16) | (active << 23);

	printf ("\nProcess : CMD : RC ENABLE\n");
	// To make sure we always write in multiples of 4Bytes
	// RC_LENGTH
	buf[0] = cnt;
	ret = syna_vmx_write_to_reg(file, RC_LENGTH_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_DATA
	cnt = ((cnt + 3) / 4) * 4;
	memset(buf, 0, sizeof (buf));
	// WRITE PRIUS in RC_DATA Register
	strncpy(buf, pData, strlen(pData));
	ret = syna_vmx_write_to_reg(file, RC_DATA_REGISTER, buf, cnt);
	if (ret != 0)
		return -1;

	// RC_OFFSET
	memset(buf, 0, sizeof (buf)); // offset of 0 to be set
	ret = syna_vmx_write_to_reg(file, RC_OFFSET_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_COMMAND
	memcpy(buf, (char *)(&data), 4);
	ret = syna_vmx_write_to_reg(file, RC_COMMAND_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_TRIGGER
	memset(buf, 0, sizeof (buf));
	buf[0] = I2C_REMOTE_CONTROL_COMMAND_MSG;
	ret = syna_vmx_write_to_reg(file, RC_TRIGGER_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	ret = syna_waitOnRC_Command_ActiveLow(file);
	if (ret == 0)
		printf("Enable RC Command Complete\n");
	else
		printf("Enable RC Command Failed\n");

	return ret;
}

// Function to report the Write Flash Progress
void syna_i2c_fw_flash_update(int iWritten, int iTotalSize)
{
	update_per = (iWritten * 100 / iTotalSize);
	if (prev_update_per != update_per)
	{
		printf ("FW Flash progress : %d%s\n", update_per, "%");
		prev_update_per = update_per;
	}
}

// FW CHECKSUM CODE
int syna_i2c_fw_checksum(int file)
{
	int ret;
	char buf[40] = {0};
	char readBuf[4096];
	int cmd = VMM_FW_CHECKSUM_CALC;
	int active = 1;
	unsigned int data = (cmd << 16) | (active << 23);

	printf ("\nProcess : CMD : FW CHECKSUM\n");
	// RC_LENGTH
	buf[0] = 4;
	ret = syna_vmx_write_to_reg(file, RC_LENGTH_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_OFFSET
	memset(buf, 0, sizeof (buf));
	ret = syna_vmx_write_to_reg(file, RC_OFFSET_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_COMMAND
	memset(buf, 0, sizeof (buf));
	memcpy(buf, (char *)(&data), 4);
	ret = syna_vmx_write_to_reg(file, RC_COMMAND_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_TRIGGER
	memset(buf, 0, sizeof (buf));
	buf[0] = I2C_REMOTE_CONTROL_COMMAND_MSG;
	ret = syna_vmx_write_to_reg(file, RC_TRIGGER_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	ret = syna_waitOnRC_Command_ActiveLow(file);
	if (ret == 0)
	{
		printf("FW Checksum Command Complete\n");
		ret = syna_vmx_read_from_reg(file, RC_DATA_REGISTER, readBuf, 4);
		if (ret == 0)
			printf("FW CHecksum data is : 0x%x\n", ((readBuf[3] << 24) | (readBuf[2] << 16) | (readBuf[1] << 8) | (readBuf[0])));
		else
			return -1;
	}
	else
		printf("FW Checksum Command Failed\n");

	return ret;
}

// READ FLASH DATA
int syna_i2c_read_flash_data(int file)
{
	int ret;
	char buf[40];
	char readBuf[4096];
	// READ FLASH DATA
	int cmd = VMM_READ_FLASH_DATA;
	int active = 1;
	unsigned int data = (cmd << 16) | (active << 23);

	printf ("\nProcess : CMD : READ FLASH DATA\n");
	// RC_LENGTH
	memset(buf, 0, sizeof (buf));
	buf[0] = 32;
	ret = syna_vmx_write_to_reg(file, RC_LENGTH_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_OFFSET
	memset(buf, 0, sizeof (buf));
	ret = syna_vmx_write_to_reg(file, RC_OFFSET_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_COMMAND
	memset(buf, 0, sizeof (buf));
	memcpy(buf, (char *)(&data), 4);
	ret = syna_vmx_write_to_reg(file, RC_COMMAND_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_TRIGGER
	memset(buf, 0, sizeof (buf));
	buf[0] = I2C_REMOTE_CONTROL_COMMAND_MSG;
	ret = syna_vmx_write_to_reg(file, RC_TRIGGER_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	ret = syna_waitOnRC_Command_ActiveLow(file);
	if (ret == 0)
	{
		printf("Read Flash Data Command Complete\n");
		ret = syna_vmx_read_from_reg(file, RC_DATA_REGISTER, readBuf, 32);
		if (ret == 0)
		{
			for (int j = 0; j < 32; j++)
			{
				if (j % 4 == 0)
					printf("\n");
				printf("\tDB%d : 0x%x", j, readBuf[j]);
			}
			printf("\n");
		}
		else
			return -1;
	}
	else
		printf("Read Flash Data Command Failed\n");

	return ret;
}

// CHIP CORE TEMPERATURE
int syna_vmxrc_get_chip_temperature(struct vmx_remctl *vmxrc, int *pTemp)
{
	int ret;
	char buf[40];
	char readBuf[4096];
	// CHIP CORE TEMPERATURE COMMAND
	int cmd = VMM_GET_CHIP_CORE_TEMPERATURE;
	int active = 1;
	unsigned int data = (cmd << 16) | (active << 23);

	printf ("\nProcess : CMD : CHIP CORE TEMPERATURE\n");
	// RC_LENGTH
	memset(buf, 0, sizeof (buf));
	buf[0] = 2;
	ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_LENGTH_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	*pTemp = -1;

	// RC_OFFSET
	memset(buf, 0, sizeof (buf));
	ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_OFFSET_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_COMMAND
	memset(buf, 0, sizeof (buf));
	memcpy(buf, (char *)(&data), 4);
	ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_COMMAND_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_TRIGGER
	memset(buf, 0, sizeof (buf));
	buf[0] = I2C_REMOTE_CONTROL_COMMAND_MSG;
	ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_TRIGGER_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	ret = syna_waitOnRC_Command_ActiveLow(vmxrc->i2c_file);
	if (ret == 0)
	{
		printf("Get Chip Core Temperature Command Complete\n");
		ret = syna_vmx_read_from_reg(vmxrc->i2c_file, RC_DATA_REGISTER, readBuf, 2);
		if (ret == 0)
		{
#ifdef DEBUG_READ_WRITE
			for (int j = 0; j < 2; j++)
			{
				printf("\tDB%d : 0x%x", j, readBuf[j]);
			}
#endif
			*pTemp = (readBuf[1] | (readBuf[0] << 8));
		}
		else
			return -1;
	}
	else
		printf("Get Chip Core Temperature Command Failed\n");

	return ret;
}

// ERASE FLASH
int syna_i2c_erase_flash_data_command(int file)
{
	int ret;
	char buf[40];
	char readBuf[4096];
	// SPI ERASE FLASH
	int cmd = VMM_ERASE_FLASH;
	int active = 1;
	unsigned int data = (cmd << 16) | (active << 23);

	update_per = prev_update_per = 0;

	printf ("\nProcess : CMD : ERASE FLASH\n");
	// RC_LENGTH
	memset(buf, 0, sizeof (buf));
	buf[0] = 2;
	ret = syna_vmx_write_to_reg(file, RC_LENGTH_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_OFFSET
	memset(buf, 0, sizeof (buf));
	ret = syna_vmx_write_to_reg(file, RC_OFFSET_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_COMMAND
	memset(buf, 0, sizeof (buf));
	memcpy(buf, (char *)(&data), 4);
	ret = syna_vmx_write_to_reg(file, RC_COMMAND_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_DATA
	memset(buf, 0, sizeof (buf));
	buf[0] = 0xFF;
	buf[1] = 0xFF;
	ret = syna_vmx_write_to_reg(file, RC_DATA_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_TRIGGER
	memset(buf, 0, sizeof (buf));
	buf[0] = I2C_REMOTE_CONTROL_COMMAND_MSG;
	ret = syna_vmx_write_to_reg(file, RC_TRIGGER_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	ret = syna_waitOnRC_Command_ActiveLow(file);
	if (ret == 0)
	{
		ret = syna_vmx_read_from_reg(file, RC_DATA_REGISTER, readBuf, 2);
		if (ret == 0)
		{
#ifdef DEBUG_READ_WRITE
			for (int j = 0; j < 2; j++)
			{
				printf("\tDB%d : 0x%x", j, readBuf[j]);
			}
			printf("\n");
#endif
			printf("ERASE FLASH Command Complete. Sleep for 3s\n");
			sleep(WAIT_TIME_AFTER_ERASE_FLASH);
		}
		else
			return -1;
	}
	else
		printf("ERASE FLASH Command Failed\n");

	return ret;
}

// ACTIVATE FW
int syna_i2c_activate_fw(int file)
{
	int ret;
	char buf[40];
	char readBuf[4096];
	// ACTIVATE FW COMMAND
	int cmd = VMM_ACTIVATE_FW;
	int active = 1;
	int fw_len = FW_LENGTH;
	unsigned int data = (cmd << 16) | (active << 23);

	printf ("\nProcess : CMD : ACTIVATE FW\n");
	// RC_LENGTH
	memset(buf, 0, sizeof (buf));
	memcpy(buf, (char *)(&fw_len), 4);
	ret = syna_vmx_write_to_reg(file, RC_LENGTH_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_OFFSET
	memset(buf, 0, sizeof (buf));
	ret = syna_vmx_write_to_reg(file, RC_OFFSET_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_COMMAND
	memset(buf, 0, sizeof (buf));
	memcpy(buf, (char *)(&data), 4);
	ret = syna_vmx_write_to_reg(file, RC_COMMAND_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	// RC_TRIGGER
	memset(buf, 0, sizeof (buf));
	buf[0] = I2C_REMOTE_CONTROL_COMMAND_MSG;
	ret = syna_vmx_write_to_reg(file, RC_TRIGGER_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	ret = syna_waitOnRC_Command_ActiveLow(file);
	if (ret == 0)
	{
		printf("ACTIVATE FW Command Complete\n");
		ret = syna_vmx_read_from_reg(file, RC_DATA_REGISTER, readBuf, 4);
		if (ret == 0)
		{
#ifdef DEBUG_READ_WRITE
			for (int j = 0; j < 4; j++)
			{
				printf("\tDB%d : 0x%x", j, readBuf[j]);
			}
#endif
		}
		else
			return -1;
	}
	else
		printf("ACTIVATE FW Command Failed\n");

	// Disable RC Command after FW Flash
	ret = syna_i2c_disable_rc_cmd(file);
	if (ret != 0)
	{
		printf("RC Disable Cmd Failed After FW acivation\n");
	}

	return ret;
}

// RESET CHIP COMMAND
int syna_vmxrc_reset_chip(struct vmx_remctl *vmxrc)
{
	int ret;
	char buf[40];

	// RESET CHIP
	printf ("\nProcess : CMD : RESET CHIP\n");

	// RC_TRIGGER
	memset(buf, 0, sizeof (buf));
	buf[0] = CHIP_RESET_CODE; // Reset Cmd
	ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_TRIGGER_REGISTER, buf, 4);
	if (ret != 0)
		return -1;

	ret = syna_waitOnRC_Command_ActiveLow(vmxrc->i2c_file);
	if (ret == 0)
	{
		printf("RESET CHIP Command Complete\n");
	}
	else
		printf("RESET CHIP Command Failed\n");

	return ret;
}

// WRITE FLASH DATA
int syna_vmxrc_write_flash_data_command(struct vmx_remctl *vmxrc, FILE *fw_fpin)
{
	int ret = 0;
	char buf[40];
	// WRITE FLASH DATA
	int cmd = VMM_WRITE_FLASH_DATA;
	int active = 1;
	unsigned int data = (cmd << 16) | (active << 23);

	printf ("\nProcess : CMD : WRITE FLASH DATA\n");
	if (fw_fpin == NULL)
	{
		printf("No FW FILE Found\n");
	}
	else
	{
		ret = syna_i2c_erase_flash_data_command(vmxrc->i2c_file);
		if (ret != 0)
		{
			printf("Erase FLASH Failed\n");
			return -1;
		}

		int size_fw, fw_len;
		fseek(fw_fpin, 0, SEEK_END);
		size_fw = ftell(fw_fpin);
		fseek(fw_fpin, 0, SEEK_SET);
		int unit_size = VMM_UNIT_SIZE;
		int offset = 0;
		int len = 0;
		int bytes_read = 0;
		if (size_fw > FW_LENGTH)
			size_fw = FW_LENGTH;
		fw_len = size_fw;
		int retry = 0;
		while (size_fw)
		{
			if (size_fw > unit_size)
				len = unit_size;
			else
				len = size_fw;

			len = ((len + 3) / 4) * 4;
			retry = 0;
			// RC_LENGTH
			memset(buf, 0, sizeof (buf));
			buf[0] = len; // Reset Cmd
			ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_LENGTH_REGISTER, buf, 4);
			if (ret != 0)
				return -1;

			// RC_OFFSET
			memset(buf, 0, sizeof (buf));
			buf[0] = (offset & 0xFF);
			buf[1] = ((offset >> 8) & 0xFF);
			buf[2] = ((offset >> 16) & 0xFF);
			buf[3] = ((offset >> 24) & 0xFF);
			ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_OFFSET_REGISTER, buf, 4);
			if (ret != 0)
				return -1;

			// RC_COMMAND
			memset(buf, 0, sizeof (buf));
			memcpy(buf, (char *)(&data), 4);
			ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_COMMAND_REGISTER, buf, 4);
			if (ret != 0)
				return -1;

			// RC_DATA
			memset(buf, 0, sizeof (buf));
			bytes_read = fread(buf, 1, len, fw_fpin);
			ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_DATA_REGISTER, buf, len);
			if (ret != 0)
				return -1;

once_again:
			// RC_TRIGGER
			memset(buf, 0, sizeof (buf));
			buf[0] = I2C_REMOTE_CONTROL_COMMAND_MSG;
			ret = syna_vmx_write_to_reg(vmxrc->i2c_file, RC_TRIGGER_REGISTER, buf, 4);
			if (ret != 0)
				return -1;

			ret = syna_waitOnRC_Command_ActiveLow(vmxrc->i2c_file);
			if (ret == 0)
			{
				offset += bytes_read;
				size_fw -= bytes_read;
				syna_i2c_fw_flash_update(offset, fw_len);
			}
			else
			{
				if (!retry)
				{
					retry = 1;
					goto once_again;
				}
				else
				{
					printf("WRITE FLASH DATA Command Failed\n");
					return -1;
				}
			}
		}
		sleep(WAIT_TIME_AFTER_WRITE_FLASH);
		// activate the firmware after a Write Flash Command
		ret = syna_i2c_activate_fw(vmxrc->i2c_file);
		// Reset the Chip once the FW is updated.
		// Followed by a User initiated Power Off/On to the VMR SoC
		if (!ret)
			ret = syna_vmxrc_reset_chip(vmxrc);
	}

	return ret;
}

// GET FIRMWARE VERSION
int syna_vmxrc_get_current_firmware_version(struct vmx_remctl *vmxrc, int *major, int *minor, int *build)
{
	char buf[10];
	long int regAddr;
	int ret = 0;

	memset(buf, 0, sizeof (buf));
	ret = syna_vmx_read_from_reg(vmxrc->i2c_file, FW_VERSION_REGISTER, buf, 4);
	if (ret != 0)
		return -1;
	else {
		//printf("FW VERSION INFO : 0x%x\n", ((buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | (buf[0])));
		*major = buf[1];
		*minor = buf[0];
		*build = buf[2];
		// buf[3] represents the "CONFIGURATION FILE VERSION"
	}
	return 0;
}

// Close the I2C Device
int syna_vmxrc_close_device(struct vmx_remctl *vmxrc)
{
	if (vmxrc != NULL)
	{
		if (close(vmxrc->i2c_file) < 0) {
			/* ERROR HANDLING; you can check errno to see what went wrong */
			printf("I2C Device Close Failed. Errno : %d\n", errno);
			return -1;
		}
		free(vmxrc);
		vmxrc = NULL;
		printf("I2C DEVICE CLOSE\n");
	}
	return 0;
}

