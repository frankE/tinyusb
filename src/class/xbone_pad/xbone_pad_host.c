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

#include "tusb_option.h"

#if (CFG_TUH_ENABLED && CFG_TUH_XBONE_PAD)

#include "host/usbh.h"
#include "host/usbh_classdriver.h"
#include "xbone_pad.h"
#include "xbone_pad_host.h"

typedef struct {
    bool irq;
    uint8_t sequence_num;
    uint8_t device_addr;
    uint8_t iface_num;
    uint8_t ep_in;
    uint8_t ep_out;
    uint8_t epin_buf[XBONE_PAD_IN_BUF_SIZE];
    uint8_t epout_buf[XBONE_PAD_OUT_BUF_SIZE];
    uint16_t epin_size;
    uint16_t epout_size;
} pad_interface_t;

typedef struct {
    uint8_t iface_count;
    pad_interface_t ifaces[CFG_TUH_XBONE_PAD];
} pad_device_t;

static pad_device_t pad_devices[CFG_TUH_DEVICE_MAX];

pad_device_t *get_device(uint8_t dev_addr) {
    return &pad_devices[dev_addr-1];
}

pad_interface_t *get_interface(uint8_t dev_addr, uint8_t index) {
    return &get_device(dev_addr)->ifaces[index];
}

pad_interface_t *get_iface_by_num(uint8_t dev_addr, uint8_t iface_no) {
    pad_device_t *device = get_device(dev_addr);
    for(uint8_t i=0;i<CFG_TUH_XBONE_PAD; i++) {
        if (device->ifaces[i].iface_num == iface_no)
            return &device->ifaces[i];
    }
    return NULL;
}

pad_interface_t *get_iface_by_ep(uint8_t dev_addr, uint8_t ep_addr) {
    pad_device_t *device = get_device(dev_addr);
    for(uint8_t i=0;i<CFG_TUH_XBONE_PAD; i++) {
        if (device->ifaces[i].ep_in == ep_addr || device->ifaces[i].ep_out == ep_addr)
            return &device->ifaces[i];
    }
    return NULL;
}

void poll(pad_interface_t *interface) {
    if (!interface->irq)
        return;
    if (usbh_edpt_busy(interface->device_addr, interface->ep_in))
        return;
    usbh_edpt_xfer(interface->device_addr, interface->ep_in, interface->epin_buf, interface->epin_size);
}

bool send_command_to_iface(pad_interface_t *interface, const uint8_t *data, uint8_t len) {
    TU_ASSERT(len > 2);
    TU_ASSERT(len < XBONE_PAD_OUT_BUF_SIZE);

    memcpy(interface->epout_buf, data, len);
    interface->epout_buf[2] = interface->sequence_num++;
    tuh_xfer_t out = {
            .daddr       = interface->device_addr,
            .ep_addr     = interface->ep_out,
            .buflen      = len,
            .buffer      = interface->epout_buf,
            .complete_cb = NULL,
    };
    return tuh_edpt_xfer(&out);
}

bool send_command(uint8_t dev_addr, uint8_t iface_num, const uint8_t *data, uint8_t len) {
    pad_interface_t *interface = get_iface_by_num(dev_addr, iface_num);
    return send_command_to_iface(interface, data, len);
}

void send_init_code(pad_interface_t *interface) {
    TU_LOG3("  sending init code(s) to iface 0x%02x\r\n", interface->iface_num);
    tusb_desc_device_t descr_dev;
    tuh_descriptor_get_device_sync(interface->device_addr, &descr_dev, sizeof(descr_dev));
    for (size_t i=0; i<sizeof(pad_data)/sizeof(pad_data[0]); i++) {
        if (pad_data[i].idVendor != 0 && pad_data[i].idVendor != descr_dev.idVendor)
            continue;
        if (pad_data[i].idProduct != 0 && pad_data[i].idProduct != descr_dev.idProduct)
            continue;
        TU_LOG3("  sending init code %u to iface 0x%02x\r\n", i, interface->iface_num);
        if (!send_command_to_iface(interface, pad_data[i].data, pad_data[i].len)) {
            TU_LOG2("  Error sending init code.\r\n");
        }
    }
    return;
}

void xbone_pad_init(void) {
    memset(pad_devices, 0, sizeof(pad_devices));
}

bool xbone_pad_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_interface_t const *desc_itf, uint16_t max_len) {
    TU_VERIFY(XBONE_PAD_SUBCLASS == desc_itf->bInterfaceSubClass && XBONE_PAD_PROTOCOL == desc_itf->bInterfaceProtocol);

    uint8_t const *p_desc = (uint8_t const *) desc_itf;
    const tusb_desc_interface_t *entry = desc_itf;
    while (entry->bNumEndpoints == 0) {
        p_desc = tu_desc_next(p_desc);
        if (p_desc[0] == 0 || p_desc[1] != TUSB_DESC_INTERFACE)
            return false;
        entry = (tusb_desc_interface_t *) p_desc;
        if (entry->bInterfaceNumber != desc_itf->bInterfaceNumber)
            return false;
    }
    TU_LOG3("open:(%u:%u)\r\n", dev_addr, desc_itf->bInterfaceNumber);

    pad_device_t *device = get_device(dev_addr);
    if (device->iface_count >= CFG_TUH_XBONE_PAD) {
        TU_LOG1("  Maximum interface count would be higher than CFG_TUH_XBONE_PAD. Skipping.");
        return false;
    }
    pad_interface_t *iface = get_interface(dev_addr, device->iface_count);
    for (uint8_t j=0; j<entry->bNumEndpoints; j++) {
        p_desc = tu_desc_next(p_desc);
        TU_ASSERT(p_desc[0] > 0);
        TU_ASSERT(p_desc[1] == TUSB_DESC_ENDPOINT);
        tusb_desc_endpoint_t const *ep = (tusb_desc_endpoint_t *)p_desc;

        if (ep->bmAttributes.xfer != TUSB_XFER_INTERRUPT) {
            return false;
        }
        iface->device_addr = dev_addr;
        iface->irq = (ep->bmAttributes.xfer == TUSB_XFER_INTERRUPT);
        TU_ASSERT(tuh_edpt_open(dev_addr, ep));
        if (tu_edpt_dir(ep->bEndpointAddress) == TUSB_DIR_IN) {
            iface->ep_in = ep->bEndpointAddress;
            iface->epin_size = tu_edpt_packet_size(ep);
            TU_ASSERT(iface->epin_size <= XBONE_PAD_IN_BUF_SIZE);
        } else {
            iface->ep_out = ep->bEndpointAddress;
            iface->epout_size = tu_edpt_packet_size(ep);
            TU_ASSERT(iface->epout_size <= XBONE_PAD_OUT_BUF_SIZE);
        }
    }
    device->iface_count++;
    iface->iface_num = entry->bInterfaceNumber;
    return true;
}

bool xbone_pad_set_config(uint8_t dev_addr, uint8_t itf_num) {
    TU_LOG3("--xbone_pad_set_config(%u:%u)\r\n", dev_addr, itf_num);
    pad_interface_t *interface = get_iface_by_num(dev_addr, itf_num);
    if (!interface->irq) {
        usbh_driver_set_config_complete(dev_addr, itf_num);
        return true;
    }
    send_init_code(interface);
    if (xbone_pad_mounted_cb) xbone_pad_mounted_cb(dev_addr, itf_num);
    poll(interface);
    usbh_driver_set_config_complete(dev_addr, itf_num);
    return true;
}

bool xbone_pad_xfer_cb(uint8_t dev_addr, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    TU_LOG3("xbone_pad_xfer_cb(dev: 0x%02x, ep: 0x%02x)\r\n", dev_addr, ep_addr);
    uint8_t const dir = tu_edpt_dir(ep_addr);
    if (result != XFER_RESULT_SUCCESS) {
        if (dir == TUSB_DIR_IN)
            TU_LOG2("  Receiving data failed on (0x%02x:0x%02x): 0x%x\r\n", dev_addr, ep_addr, result);
        else
            TU_LOG2("  Sending data failed on (0x%02x:0x%02x): 0x%x\r\n", dev_addr, ep_addr, result);
        return true;
    }
    pad_interface_t *interface = get_iface_by_ep(dev_addr, ep_addr);
    if ( dir == TUSB_DIR_IN ) {
        xbone_pad_input_event_cb(dev_addr, interface->iface_num, interface->epin_buf, xferred_bytes);
        TU_LOG3("  Data received:\r\n");
        TU_LOG3_MEM(interface->epin_buf, interface->epin_size, 2);
        poll(interface);
    } else {
        if (xbone_pad_command_sent_cb)
            xbone_pad_command_sent_cb(dev_addr, interface->iface_num, interface->epout_buf, xferred_bytes);
        TU_LOG3("  Data sent\r\n");
    }
    return true;
}

void xbone_pad_close(uint8_t dev_addr) {
    TU_LOG3("xbone_pad_close(0x%02x)", dev_addr);
    if (xbone_pad_removed_cb) xbone_pad_removed_cb(dev_addr);
    pad_device_t *device = get_device(dev_addr);
    memset(device, 0, sizeof(*device));
}

#endif
