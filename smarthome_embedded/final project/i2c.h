/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    i2c.h
 *
 * Description:
 *    Implements a generic I2C function library for the LPC2xxx processor.
 *
 *****************************************************************************/
#ifndef _I2C_H
#define _I2C_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "pre_emptive_os/api/general.h"


/******************************************************************************
 * Defines, macros, and typedefs
 *****************************************************************************/
//Return codes
#define I2C_CODE_OK     1
#define I2C_CODE_ERROR  0
#define I2C_CODE_EMPTY -1
#define I2C_CODE_BUSY  -2

//Command codes for transmitting extra bytes
#define I2C_EXTRA_NONE 0
#define I2C_EXTRA_BYTE 1
#define I2C_EXTRA_WORD 2


/******************************************************************************
 * Public functions
 *****************************************************************************/

/******************************************************************************
 *
 * Description:
 *    Checks the I2C status.
 *
 *  Returns:
 *      00h Bus error
 *      08h START condition transmitted
 *      10h Repeated START condition transmitted
 *      18h SLA + W transmitted, ACK received
 *      20h SLA + W transmitted, ACK not received
 *      28h Data byte transmitted, ACK received
 *      30h Data byte transmitted, ACK not received
 *      38h Arbitration lost
 *      40h SLA + R transmitted, ACK received
 *      48h SLA + R transmitted, ACK not received
 *      50h Data byte received in master mode, ACK transmitted
 *      58h Data byte received in master mode, ACK not transmitted
 *      60h SLA + W received, ACK transmitted
 *      68h Arbitration lost, SLA + W received, ACK transmitted
 *      70h General call address received, ACK transmitted
 *      78h Arbitration lost, general call addr received, ACK transmitted
 *      80h Data byte received with own SLA, ACK transmitted
 *      88h Data byte received with own SLA, ACK not transmitted
 *      90h Data byte received after general call, ACK transmitted
 *      98h Data byte received after general call, ACK not transmitted
 *      A0h STOP or repeated START condition received in slave mode
 *      A8h SLA + R received, ACK transmitted
 *      B0h Arbitration lost, SLA + R received, ACK transmitted
 *      B8h Data byte transmitted in slave mode, ACK received
 *      C0h Data byte transmitted in slave mode, ACK not received
 *      C8h Last byte transmitted in slave mode, ACK received
 *      F8h No relevant status information, SI=0
 *      FFh Channel error
 *
 *****************************************************************************/
tU8 i2cCheckStatus(void);


/******************************************************************************
 *
 * Description:
 *    Reset and initialize the I2C peripheral.
 *
 * Params:
 *    [in]  i2cFrequency  - frequency on clock signal in Hz (max 400 kHz)
 *
 *****************************************************************************/
void i2cInit(tU32 i2cFrequency);


/******************************************************************************
 *
 * Description:
 *    Generates a start condition on I2C when bus is free.
 *    Master mode will also automatically be entered.
 *
 * Returns:
 *    I2C_CODE_OK or I2C status code
 *
 *****************************************************************************/
tS8 i2cStart(void);


/******************************************************************************
 *
 * Description:
 *    Generates a start condition on I2C when bus is free.
 *    Master mode will also automatically be entered.
 *
 * Returns:
 *    I2C_CODE_OK or I2C status code
 *
 *****************************************************************************/
tS8 i2cRepeatStart(void);


/******************************************************************************
 *
 * Description:
 *    Generates a stop condition in master mode or recovers from an error
 *    condition in slave mode.
 *
 * Returns:
 *    I2C_CODE_OK
 *
 *****************************************************************************/
tS8 i2cStop(void);


/******************************************************************************
 *
 * Description:
 *    Sends a character on the I2C network
 *
 * Params:
 *    [in] data - the character to send
 *
 * Returns:
 *    I2C_CODE_OK   - successful
 *    I2C_CODE_BUSY - data register is not ready -> byte was not sent
 *
 *****************************************************************************/
tS8 i2cPutChar(tU8 data);


/******************************************************************************
 *
 * Description:
 *    Read a character. I2C master mode is used.
 *    This function is also used to prepare if the master shall generate
 *    acknowledge or not acknowledge.
 *
 * Params:
 *    [in]  mode  - I2C_MODE_ACK0 Set ACK=0. Slave sends next byte
 *                  I2C_MODE_ACK1 Set ACK=1. Slave sends last byte
 *                  I2C_MODE_READ Read data from data register
 *    [out] pData - a pointer to where the data shall be saved.
 *
 * Returns:
 *    I2C_CODE_OK    - successful
 *    I2C_CODE_EMPTY - no data is available
 *
 *****************************************************************************/
tS8 i2cGetChar(tU8 mode, tU8* pData);


/******************************************************************************
 *
 * Description:
 *    Wait after transmission of a byte until done
 *
 * Returns:
 *    I2C_CODE_OK    - successful / done
 *    I2C_CODE_ERROR - an error occured
 *
 *****************************************************************************/
tS8 i2cWaitAfterTransmit(void);


/******************************************************************************
 *
 * Description:
 *    Sends a character on the I2C network and wait until done
 *
 * Params:
 *    [in] data - the character to send
 *
 * Returns:
 *    I2C_CODE_OK    - successful
 *    I2C_CODE_BUSY  - data register is not ready -> byte was not sent
 *    I2C_CODE_ERROR - an error occured
 *
 *****************************************************************************/
tS8 i2cPutCharAndWait(tU8 data);


/******************************************************************************
 *
 * Description:
 *    Sends data on the I2C network
 *
 *    Note: No stop condition is generated after the transmission - this must
 *          be done after calling this function.
 *
 * Params:
 *    [in] addr     - address
 *    [in] extraCmd - controls if 0, 1 or 2 extra bytes shall be transmitted
 *                    before data buffer
 *    [in] extra    - byte of word to be transmitted before data buffer
 *    [in] pData    - data to transmit
 *    [in] len      - number of bytes to transmit
 *
 * Returns:
 *    I2C_CODE_OK    - successful
 *    I2C_CODE_ERROR - an error occured
 *
 *****************************************************************************/
tS8 i2cWrite(tU8 addr, tU8 extraCmd, tU16 extra, tU8* pData, tU16 len);


/******************************************************************************
 *
 * Description:
 *    Waits till slave device returns ACK (after busy period)
 *
 * Params:
 *    [in] addr  - address
 *
 * Returns:
 *    I2C_CODE_OK or I2C_CODE_ERROR
 *
 *****************************************************************************/
tS8 i2cPoll(tU8 addr);


/******************************************************************************
 *
 * Description:
 *    Read a specified number of bytes from the I2C network.
 *
 *    Note: No start condition is generated in the beginningis must be
 *          done before calling this function.
 *
 * Params:
 *    [in] addr - address
 *    [in] pBuf - receive buffer
 *    [in] len  - number of bytes to receive
 *
 * Returns:
 *    I2C_CODE_OK or I2C status code
 *
 *****************************************************************************/
tS8 i2cRead(tU8 addr, tU8* pBuf, tU16 len);


#endif
