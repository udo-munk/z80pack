/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2024 Thomas Eberhardt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"
#include "hw_config.h"
#include "sd_card.h"
#include "stdio_msc_usb.h"

typedef enum {
	SCSI_CMD_VERIFY_10		= 0x2f,
	SCSI_CMD_SYNCHRONIZE_CACHE_10	= 0x35
} scsi_cmd_type_2_t;

// whether mass storage interface is active
static bool msc_ejected = true;

void stdio_msc_usb_do_msc(void)
{
	stdio_msc_usb_disable_irq_tud_task();
	msc_ejected = false;
	while (!msc_ejected)
		tud_task();
	stdio_msc_usb_enable_irq_tud_task();
}

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up
// to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8],
			uint8_t product_id[16], uint8_t product_rev[4])
{
	(void) lun;

	const char vid[] = "Z80pack";
	const char pid[] = "Mass Storage";
	const char rev[] = "1.0";

	memcpy(vendor_id, vid, strlen(vid));
	memcpy(product_id, pid, strlen(pid));
	memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
	(void) lun;

	if (msc_ejected) {
		// Additional Sense 3A-00 is NOT_FOUND
		tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
		return false;
	}

	return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and
// SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count,
			 uint16_t* block_size)
{
	sd_card_t *sd_card_p = sd_get_by_num(0);

	(void) lun;

	if (sd_card_p == NULL || msc_ejected) {
		*block_count = 0;
		*block_size = 0;
	} else {
		*block_count = sd_card_p->get_num_sectors(sd_card_p);
		*block_size = 512;
	}
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start,
			   bool load_eject)
{
	(void) lun;
	(void) power_condition;

	if (load_eject) {
		if (start) {
			// load disk storage
		} else {
			// unload disk storage
			msc_ejected = true;
		}
	}

	return true;
}

// Invoked when we receive the Prevent / Allow Medium Removal command
bool tud_msc_prevent_allow_medium_removal_cb(uint8_t lun,
					     uint8_t prohibit_removal,
					     uint8_t control)
{
	(void) lun;
	(void) prohibit_removal;
	(void) control;

	return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of
// copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
			  void* buffer, uint32_t bufsize)
{
	sd_card_t *sd_card_p = sd_get_by_num(0);
	uint32_t blockcnt;
	block_dev_err_t rc;

	(void) lun;
	(void) offset;

	if (sd_card_p == NULL || msc_ejected)
		return -1;

	if (lba >= sd_card_p->get_num_sectors(sd_card_p))
		return -1;

	blockcnt = bufsize / 512;

	rc = sd_card_p->read_blocks(sd_card_p, buffer, lba, blockcnt);
	if (rc != SD_BLOCK_DEVICE_ERROR_NONE)
		return -1;
	else
		return blockcnt * 512;
}

bool tud_msc_is_writable_cb (uint8_t lun)
{
	(void) lun;

	return sd_get_by_num(0) != NULL;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes.
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
			   uint8_t* buffer, uint32_t bufsize)
{
	sd_card_t *sd_card_p = sd_get_by_num(0);
	uint32_t blockcnt;
	block_dev_err_t rc;

	(void) lun;
	(void) offset;

	if (sd_card_p == NULL || msc_ejected)
		return -1;

	if (lba >= sd_card_p->get_num_sectors(sd_card_p))
		return -1;

	blockcnt = bufsize / 512;

	rc = sd_card_p->write_blocks(sd_card_p, buffer, lba, blockcnt);
	if (rc != SD_BLOCK_DEVICE_ERROR_NONE)
		return -1;
	else
		return blockcnt * 512;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16],
			 void* buffer, uint16_t bufsize)
{
	// read10 & write10 has their own callback and MUST not be handled here

	void const* response = NULL;
	int32_t resplen = 0;

	// most scsi handled is input
	bool in_xfer = true;

	switch (scsi_cmd[0]) {
	case SCSI_CMD_VERIFY_10:
		if (msc_ejected)
			resplen = -1;
		else
			resplen = 0;		// report success
		break;

	case SCSI_CMD_SYNCHRONIZE_CACHE_10:
		if (msc_ejected)
			resplen = -1;
		else
			resplen = 0;		// report success
		break;

	default:
		// Set Sense = Invalid Command Operation
		tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

		// negative means error -> tinyusb could stall and/or response
		// with failed status
		resplen = -1;
		break;
	}

	// return resplen must not larger than bufsize
	if (resplen > bufsize)
		resplen = bufsize;

	if (response && (resplen > 0)) {
		if (in_xfer) {
			memcpy(buffer, response, (size_t) resplen);
		} else {
			// SCSI output
		}
	}

	return (int32_t) resplen;
}
