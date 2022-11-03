/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Frank Entz
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

// Possible Xbox One Controllers
// Not all are supported.

/*
* Microsoft X-Box One pad
* Microsoft X-Box One pad (Firmware 2015)
* Microsoft X-Box One Elite pad
* Microsoft X-Box One Elite 2 pad
* Microsoft X-Box One S pad
* Afterglow Prismatic Wired Controller
* PDP Xbox One Controller
* Rock Candy Wired Controller for Xbox One
* PDP Marvel Xbox One Controller
* PDP Xbox One Arcade Stick
* PDP Xbox One Controller
* PDP Xbox One Controller
* PDP Xbox One Controller
* PDP Battlefield One
* PDP Titanfall 2
* Rock Candy Gamepad for Xbox One 2015
* PDP Xbox One Controller
* PDP Xbox One Controller
* PDP Wired Controller for Xbox One - Crimson Red
* PDP Wired Controller for Xbox One - Stealth Series
* PDP Wired Controller for Xbox One - Camo Series
* PDP Xbox One Controller
* PDP Xbox One Controller
* PDP Controller for Xbox One
* PDP Wired Controller for Xbox One - Stealth Series
* Afterglow Prismatic Wired Controller
* Afterglow Prismatic Wired Controller
* Rock Candy Gamepad for Xbox One 2016
* Hori Real Arcade Pro Hayabusa (USA) Xbox One
* HORIPAD ONE
* Hori Real Arcade Pro V Kai Xbox One
* Hori Fighting Commander ONE
* Razer Atrox Arcade Stick
* Razer Wildcat
* BDA Xbox Series X Wired Controller
* PowerA Enhanced Wired Controller for Xbox Series X|S
* Hyperkin Duke X-Box One pad
* PowerA Xbox One Mini Wired Controller
* Xbox ONE spectra
* PowerA Xbox One wired controller
* PowerA FUSION Pro Controller
* PowerA FUSION Controller
* 8BitDo Pro 2 Wired Controller fox Xbox
* Fanatec Speedster 3 Forceshock Wheel
* Chinese-made Xbox Controller
* Generic X-Box pad
*/

#ifndef _XBONE_PAD_H_
#define _XBONE_PAD_H_

#include "stdbool.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif


// Most of the information here is taken from:
// https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c

// Subclass and protocol is the same across all vendors/products
#define XBONE_PAD_SUBCLASS   71
#define XBONE_PAD_PROTOCOL  208

#define GIP_CMD_ACK             0x01
#define GIP_CMD_HEARBEAT        0x03
#define GIP_CMD_IDENTIFY        0x04
#define GIP_CMD_POWER           0x05
#define GIP_CMD_AUTHENTICATE    0x06
#define GIP_CMD_VIRTUAL_KEY     0x07
#define GIP_CMD_RUMBLE          0x09
#define GIP_CMD_LED             0x0a
#define GIP_CMD_FIRMWARE        0x0c
#define GIP_CMD_INPUT           0x20

#define GIP_SEQ0                0x00

#define GIP_OPT_ACK             0x10
#define GIP_OPT_INTERNAL        0x20


#define GIP_PL_LEN(N) (N)

#define GIP_PWR_ON 0x00
#define GIP_LED_ON 0x01

#define BIT(n) (1 << n)

#define GIP_MOTOR_R     BIT(0)
#define GIP_MOTOR_L     BIT(1)
#define GIP_MOTOR_RT    BIT(2)
#define GIP_MOTOR_LT    BIT(3)
#define GIP_MOTOR_ALL   (GIP_MOTOR_R | GIP_MOTOR_L | GIP_MOTOR_RT | GIP_MOTOR_LT)

/*
 * This packet is required for all Xbox One pads with 2015
 * or later firmware installed (or present from the factory).
 */
static const uint8_t pad_power_on[] = {
        GIP_CMD_POWER, GIP_OPT_INTERNAL, GIP_SEQ0, GIP_PL_LEN(1), GIP_PWR_ON
};

/*
 * This packet is required for Xbox One S (0x045e:0x02ea)
 * and Xbox One Elite Series 2 (0x045e:0x0b00) pads to
 * initialize the controller that was previously used in
 * Bluetooth mode.
 */
static const uint8_t pad_s_init[] = {
        GIP_CMD_POWER, GIP_OPT_INTERNAL, GIP_SEQ0, 0x0f, 0x06
};

/*
 * This packet is required to get additional input data
 * from Xbox One Elite Series 2 (0x045e:0x0b00) pads.
 * We mostly do this right now to get paddle data
 */
static const uint8_t extra_input_packet_init[] = {
        0x4d, 0x10, 0x01, 0x02, 0x07, 0x00
};

/*
 * This packet is required for the Titanfall 2 Xbox One pads
 * (0x0e6f:0x0165) to finish initialization and for Hori pads
 * (0x0f0d:0x0067) to make the analog sticks work.
 */
static const uint8_t pad_hori_ack_id[] = {
        GIP_CMD_ACK, GIP_OPT_INTERNAL, GIP_SEQ0, GIP_PL_LEN(9),
        0x00, GIP_CMD_IDENTIFY, GIP_OPT_INTERNAL, 0x3a, 0x00, 0x00, 0x00, 0x80, 0x00
};

/*
 * This packet is required for most (all?) of the PDP pads to start
 * sending input reports. These pads include: (0x0e6f:0x02ab),
 * (0x0e6f:0x02a4), (0x0e6f:0x02a6).
 */
static const uint8_t pad_pdp_led_on[] = {
        GIP_CMD_LED, GIP_OPT_INTERNAL, GIP_SEQ0, GIP_PL_LEN(3), 0x00, GIP_LED_ON, 0x14
};

/*
 * This packet is required for most (all?) of the PDP pads to start
 * sending input reports. These pads include: (0x0e6f:0x02ab),
 * (0x0e6f:0x02a4), (0x0e6f:0x02a6).
 */
static const uint8_t pad_pdp_auth[] = {
        GIP_CMD_AUTHENTICATE, GIP_OPT_INTERNAL, GIP_SEQ0, GIP_PL_LEN(2), 0x01, 0x00
};

/*
 * A specific rumble packet is required for some PowerA pads to start
 * sending input reports. One of those pads is (0x24c6:0x543a).
 */
static const uint8_t pad_rumblebegin_init[] = {
        GIP_CMD_RUMBLE, 0x00, GIP_SEQ0, GIP_PL_LEN(9),
        0x00, GIP_MOTOR_ALL, 0x00, 0x00, 0x1D, 0x1D, 0xFF, 0x00, 0x00
};

/*
 * A rumble packet with zero FF intensity will immediately
 * terminate the rumbling required to init PowerA pads.
 * This should happen fast enough that the motors don't
 * spin up to enough speed to actually vibrate the gamepad.
 */
static const uint8_t pad_rumbleend_init[] = {
        GIP_CMD_RUMBLE, 0x00, GIP_SEQ0, GIP_PL_LEN(9),
        0x00, GIP_MOTOR_ALL, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
 * This specifies the selection of init packets that a gamepad
 * will be sent on init *and* the order in which they will be
 * sent. The correct sequence number will be added when the
 * packet is going to be sent.
 */

#define PAD_DATA(_vendor, _product, _data) {           \
        .idVendor  = (_vendor),                        \
        .idProduct = (_product),                       \
        .data      = (_data),                          \
        .len       = sizeof(_data)                     \
}

static const struct {
    uint16_t idVendor;
    uint16_t idProduct;
    const uint8_t *data;
    uint8_t len;
} pad_data[] = {
        PAD_DATA(0x0e6f, 0x0165, pad_hori_ack_id),            // PDP Titanfall 2
        PAD_DATA(0x0f0d, 0x0067, pad_hori_ack_id),            // Horipad One
        PAD_DATA(0x0000, 0x0000, pad_power_on),               // Generic
        PAD_DATA(0x045e, 0x02ea, pad_s_init),                 // Microsoft X-Box One S pad
        PAD_DATA(0x045e, 0x0b00, pad_s_init),                 // Microsoft X-Box One Elite 2 pad
        PAD_DATA(0x045e, 0x0b00, extra_input_packet_init),    // Microsoft X-Box One Elite 2 pad
        PAD_DATA(0x0e6f, 0x0000, pad_pdp_led_on),             // PDP Xbox One Controller (multiple)
        PAD_DATA(0x0e6f, 0x0000, pad_pdp_auth),               // PDP Xbox One Controller (multiple)
        PAD_DATA(0x24c6, 0x541a, pad_rumblebegin_init),       // PowerA Xbox One Mini Wired Controller
        PAD_DATA(0x24c6, 0x542a, pad_rumblebegin_init),       // Xbox ONE spectra
        PAD_DATA(0x24c6, 0x543a, pad_rumblebegin_init),      // PowerA Xbox One wired controller
        PAD_DATA(0x24c6, 0x541a, pad_rumbleend_init),        // PowerA Xbox One Mini Wired Controller
        PAD_DATA(0x24c6, 0x542a, pad_rumbleend_init),        // Xbox ONE spectra
        PAD_DATA(0x24c6, 0x543a, pad_rumbleend_init),        // PowerA Xboc One wired controller
};

typedef struct {
	uint8_t type;
	uint8_t const_0;
	uint16_t id;

	bool sync : 1;
	bool dummy1 : 1;
	bool start : 1;
	bool back : 1;

	bool a : 1;
	bool b : 1;
	bool x : 1;
	bool y : 1;

	bool dpad_up : 1;
	bool dpad_down : 1;
	bool dpad_left : 1;
	bool dpad_right : 1;

	bool bumper_left : 1;
	bool bumper_right : 1;
	bool stick_left_click : 1;
	bool stick_right_click : 1;

	uint16_t trigger_left;
	uint16_t trigger_right;

	int16_t stick_left_x;
	int16_t stick_left_y;
	int16_t stick_right_x;
	int16_t stick_right_y;
} xbone_pad_input_t;

typedef struct {
	uint8_t type;
	uint8_t const_20;
	uint16_t id;

	uint8_t dummy_const_80;
	uint8_t first_after_controller;
	uint8_t dummy1;
	uint8_t dummy2;
} xbone_pad_heartbeat_t;

#ifdef __cplusplus
}
#endif
#endif // _XBONE_PAD_H_
