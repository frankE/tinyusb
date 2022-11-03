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

// https://github.com/quantus/xbox-one-controller-protocol

#ifndef _TUSB_XBONE_PAD_HOST_H_
#define _TUSB_XBONE_PAD_HOST_H_
#include "xbone_pad.h"
#include "common/tusb_common.h"
#include "common/tusb_compiler.h"
#include "common/tusb_types.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef XBONE_PAD_BUF_SIZE
#define XBONE_PAD_BUF_SIZE 64
#endif

#ifndef XBONE_PAD_IN_BUF_SIZE
#define XBONE_PAD_IN_BUF_SIZE XBONE_PAD_BUF_SIZE
#endif

#ifndef XBONE_PAD_OUT_BUF_SIZE
#define XBONE_PAD_OUT_BUF_SIZE XBONE_PAD_BUF_SIZE
#endif

#ifndef XBONE_PAD_POLLING_INTERVAL_MS
#define XBONE_PAD_POLLING_INTERVAL_MS 5
#endif

//--------------------------------------------------------------------+
// API
//--------------------------------------------------------------------+
void xbone_pad_input_event_cb(uint8_t dev_addr, uint8_t interface, uint8_t *input, uint16_t len);
TU_ATTR_WEAK void xbone_pad_command_sent_cb(uint8_t dev_addr, uint8_t interface, uint8_t *data, uint16_t len);
TU_ATTR_WEAK void xbone_pad_mounted_cb(uint8_t dev_addr, uint8_t interface_num);
TU_ATTR_WEAK void xbone_pad_removed_cb(uint8_t dev_addr);

bool xbone_pad_send_command(uint8_t dev_addr, uint8_t iface_addr, const uint8_t *data, uint8_t len);
//--------------------------------------------------------------------+
// Class Driver Configuration
//--------------------------------------------------------------------+
void xbone_pad_init       (void);
bool xbone_pad_open       (uint8_t rhport, uint8_t dev_addr, tusb_desc_interface_t const *desc_itf, uint16_t max_len);
bool xbone_pad_set_config (uint8_t dev_addr, uint8_t itf_num);
bool xbone_pad_xfer_cb    (uint8_t dev_addr, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes);
void xbone_pad_close      (uint8_t dev_addr);

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_XBONE_PAD_HOST_H_ */
