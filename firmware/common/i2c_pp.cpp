/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "i2c_pp.hpp"
#include "usb_serial_asyncmsg.hpp"

void I2C::start(const I2CConfig& config) {
    i2cStart(_driver, &config);
}

void I2C::stop() {
    i2cStop(_driver);
}

bool I2C::probe(i2caddr_t addr, systime_t timeout) {
    i2cAcquireBus(_driver);
    chSysLock();
    _driver->errors = I2CD_NO_ERROR;
    _driver->state = I2C_ACTIVE_TX;
    msg_t rdymsg = i2c_lld_master_transmit_timeout(_driver, addr, nullptr, 0, nullptr, 0, timeout);
    
    if (rdymsg == RDY_TIMEOUT)
        _driver->state = I2C_LOCKED;
    
    else
        _driver->state = I2C_READY;
        
    // _driver->state = I2C_READY;
    chSysUnlock();
    i2cReleaseBus(_driver);
    // return true;
    return (rdymsg == RDY_OK);
}

// transfer是与web交互，是否与语音交互待测试

bool I2C::transfer(
    const address_t slave_address,
    const uint8_t* const data_tx,
    const size_t count_tx,
    uint8_t* const data_rx,
    const size_t count_rx,
    systime_t timeout) {
    
    if (_driver->state != I2C_READY) {
        // 如果状态是LOCKED，尝试重置
        std::string tmp_status = " NOT CHANGE! \n";
        
        if (_driver->state == I2C_LOCKED) {
            chSysLock();
            _driver->state = I2C_READY;  // 强制设置为READY状态
            chSysUnlock();
            tmp_status = " CHANGE!\n";
        }
        std::string debug_string = "@@@@@@@@@ I2C::transfer " +to_string_hex(slave_address)+  + "\t"+ std::to_string(_driver->state) +"\t" + " out of func the status is :" +tmp_status;
        UsbSerialAsyncmsg::asyncmsg(debug_string);
    }
    
    i2cAcquireBus(_driver);
    // 这只是记录断开的情况？
    const msg_t status = i2cMasterTransmitTimeout(
        _driver, slave_address, data_tx, count_tx, data_rx, count_rx, timeout);
    i2cReleaseBus(_driver);
    // source
    return (status == RDY_OK);

    // if(status != RDY_OK && slave_address == 0x51)
    // {
    //     std::string debug_string = "@@@@@@@@@ transfer " +to_string_hex(slave_address)+ " force RDY_OK tag ok\n";

    //     UsbSerialAsyncmsg::asyncmsg(debug_string);
    // }


    // return true;
}


bool I2C::receive(
    const address_t slave_address,
    uint8_t* const data,
    const size_t count,
    systime_t timeout) {

    i2cAcquireBus(_driver);
    const msg_t status = i2cMasterReceiveTimeout(
        _driver, slave_address, data, count, timeout);
    i2cReleaseBus(_driver);
    // source
    return (status == RDY_OK);

    // if(status != RDY_OK && slave_address == 0x51)
    // {
    //     std::string debug_string = "@@@@@@@@@ receive " +to_string_hex(slave_address)+ " RDY_OK force tag true \n";
    
    //     UsbSerialAsyncmsg::asyncmsg(debug_string);
    // }
    // return true;
}

bool I2C::transmit(
    const address_t slave_address,
    const uint8_t* const data,
    const size_t count,
    systime_t timeout) {
    return transfer(slave_address, data, count, NULL, 0, timeout);
}
