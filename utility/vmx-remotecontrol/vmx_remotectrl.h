#ifndef SYNA_VMX_REMOTE_CONTROL_H
#define SYNA_VMX_REMOTE_CONTROL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <getopt.h>
#include <linux/videodev2.h>
#include <linux/i2c-dev.h>
#include <errno.h>

#define DEFAULT_I2C_BUS_ID 0
#define DEFAULT_I2C_SLAVE_ADDRESS 0x39

// Global structure for I2C Operations
struct vmx_remctl {
	int i2c_bus_id;
	int i2c_slave_address;
	int i2c_file;
}vmx_remctl;

// FUNCTIONS
struct vmx_remctl * syna_vmxrc_open_device(int i2c_device_id, int i2c_slave_address);
int syna_vmxrc_get_chip_temperature(struct vmx_remctl *vmxrc, int *pTemp);
int syna_vmxrc_reset_chip(struct vmx_remctl *vmxrc);
int syna_vmxrc_write_flash_data_command(struct vmx_remctl *vmxrc, FILE *fw_fpin);
int syna_vmxrc_get_current_firmware_version(struct vmx_remctl *vmxrc, int *major, int *minor, int *build);
int syna_vmxrc_close_device(struct vmx_remctl *vmxrc);

#endif
