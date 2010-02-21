/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    i2c.c
 *
 * Description:
 *    Implements a generic I2C function library for the LPC2xxx processor.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "i2c.h"
#include <lpc2xxx.h>
#include "../startup/config.h"


/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD

//different modes
#define I2C_MODE_ACK0 0
#define I2C_MODE_ACK1 1
#define I2C_MODE_READ 2


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
tU8
i2cCheckStatus(void)
{
  tU8 status;

  //wait for I2C Status changed
  while( (I2C_I2CONSET & 0x08) == 0)   //while SI == 0
    ;

  //read I2C State
  status = I2C_I2STAT;

  //NOTE! SI flag is not cleared here

  return status;
}

/******************************************************************************
 *
 * Description:
 *    Reset and initialize the I2C peripheral.
 *
 * Params:
 *    [in]  i2cFrequency  - frequency on clock signal in Hz (max 400 kHz)
 *
 *****************************************************************************/
void
i2cInit(tU32 i2cFrequency)
{
	//connect I2C signals (SDA & SCL) to P0.2 and P0.3
  PINSEL0 &= ~0x000000F0;
  PINSEL0 |=  0x00000050;

  //clear flags
  I2C_I2CONCLR = 0x6c;

  //set bit timing ans start with checking for maximum speed (400kHz)
  if (i2cFrequency > 400000)
    i2cFrequency = 400000;
  I2C_I2SCLH   = (((CRYSTAL_FREQUENCY * PLL_FACTOR) / VPBDIV_FACTOR) / i2cFrequency + 1) / 2;
  I2C_I2SCLL   = (((CRYSTAL_FREQUENCY * PLL_FACTOR) / VPBDIV_FACTOR) / i2cFrequency) / 2;

  //reset registers
  I2C_I2ADR    = 0x00;
  I2C_I2CONSET = 0x40;
}

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
tS8
i2cStart(void)
{
  tU8   status  = 0;
  tS8   retCode = 0;

  //issue a start condition
  I2C_I2CONSET |= 0x20;   //STA = 1, set start flag

  //wait until START transmitted
  while(1)
  {
    status = i2cCheckStatus();

    //start transmitted
    if((status == 0x08) || (status == 0x10))
    {
      retCode = I2C_CODE_OK;
      break;
    }

    //error
    else if(status != 0xf8)
    {
      retCode = (tS8) status;
      break;
    }

    else
    {
      //clear SI flag
      I2C_I2CONCLR = 0x08;
    }    
  }

  //clear start flag
  I2C_I2CONCLR = 0x20;

  return retCode;
}

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
tS8
i2cRepeatStart(void)
{
  tU8   status  = 0;
  tS8   retCode = 0;

  //issue a start condition
  I2C_I2CONSET |= 0x20;   //STA = 1, set start flag
  I2C_I2CONCLR = 0x08;    //clear SI flag

  //wait until START transmitted
  while(1)
  {
    status = i2cCheckStatus();

    //start transmitted
    if((status == 0x08) || (status == 0x10))
    {
      retCode = I2C_CODE_OK;
      break;
    }

    //error
    else if(status != 0xf8)
    {
      retCode = (tS8) status;
      break;
    }

    else
    {
      //clear SI flag
      I2C_I2CONCLR = 0x08;
    }    
  }

  //clear start flag
  I2C_I2CONCLR = 0x20;

  return retCode;
}

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
tS8
i2cStop(void)
{
  I2C_I2CONSET |= 0x10;  //STO = 1, set stop flag
  I2C_I2CONCLR = 0x08;   //clear SI flag

  //wait for STOP detected (while STO = 1)
  while((I2C_I2CONSET & 0x10) == 0x10)
    ;

  return I2C_CODE_OK;
}

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
tS8
i2cPutChar(tU8 data)
{
  tS8 retCode = 0;

  //check if I2C Data register can be accessed
  if((I2C_I2CONSET & 0x08) != 0)    //if SI = 1
  {
    /* send data */
    I2C_I2DAT    = data;
    I2C_I2CONCLR = 0x08;       //clear SI flag
    retCode    = I2C_CODE_OK;
  }
  else
  {
    //data register not ready
    retCode = I2C_CODE_BUSY;
  }

  return retCode;
}

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
tS8
i2cGetChar(tU8  mode,
           tU8* pData)
{
  tS8 retCode = I2C_CODE_OK;

  if(mode == I2C_MODE_ACK0)
  {
    //the operation mode is changed from master transmit to master receive
    //set ACK=0 (informs slave to send next byte)
    I2C_I2CONSET |= 0x04;   //AA=1
    I2C_I2CONCLR = 0x08;    //clear SI flag
  }
  else if(mode == I2C_MODE_ACK1)
  {
    //set ACK=1 (informs slave to send last byte)
    I2C_I2CONCLR = 0x04;     
    I2C_I2CONCLR = 0x08;   //clear SI flag
  }
  else if(mode == I2C_MODE_READ)
  {
    //check if I2C Data register can be accessed
    if((I2C_I2CONSET & 0x08) != 0)    //SI = 1
    {
      //read data
      *pData = (tU8)I2C_I2DAT;
    }
    else
    {
      //No data available
      retCode = I2C_CODE_EMPTY;
    }
  }

  return retCode;
}


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
tS8
i2cWaitAfterTransmit(void)
{
  tU8 status = 0;

  //wait until data transmitted
  while(1)
  {
    //get new status
    status = i2cCheckStatus();

    /* 
     * SLA+W transmitted, ACK received or
     * data byte transmitted, ACK received
     */
    if( (status == 0x18) || (status == 0x28) )
    {
      //data transmitted and ACK received
      return I2C_CODE_OK;
    }

    //no relevant status information
    else if(status != 0xf8 )
    {
      //error
      return I2C_CODE_ERROR;
    }
  }
}


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
tS8
i2cPutCharAndWait(tU8 data)
{
  tS8 retCode;
  
  retCode = i2cPutChar(data);
  while(retCode == (tS8)I2C_CODE_BUSY)
    retCode = i2cPutChar(data);

  if(retCode == I2C_CODE_OK)
    retCode = i2cWaitAfterTransmit();

  return retCode;
}


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
tS8
i2cWrite(tU8  addr,
         tU8  extraCmd,
         tU16 extra,
         tU8* pData,
         tU16 len)
{
  tS8 retCode = 0;
  tU8 i       = 0;

  do
  {
    //generate Start condition
    retCode = i2cStart();
    if(retCode != I2C_CODE_OK)
      break;
    
    //Transmit slave address
    retCode = i2cPutCharAndWait(addr);
    if(retCode != I2C_CODE_OK)
      break;

    //Transmit MSB of extra word (if wanted)
   	if (extraCmd == I2C_EXTRA_WORD)
  	{
      retCode = i2cPutCharAndWait((tU8)(extra >> 8));
      if(retCode != I2C_CODE_OK)
        break;
    }

    //Transmit LSB of extra work (if wanted)
   	if ((extraCmd == I2C_EXTRA_BYTE) || (extraCmd == I2C_EXTRA_WORD))
   	{
      retCode = i2cPutCharAndWait((tU8)(extra & 0xff));
      if(retCode != I2C_CODE_OK)
        break;
	  }

    //wait until address transmitted and transmit data
    for(i = 0; i < len; i++)
    {
      retCode = i2cPutCharAndWait(*pData);
      if(retCode != I2C_CODE_OK)
        break;
      pData++;
    }
  } while(0);

  return retCode;
}


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
tS8 
i2cPoll(tU8 addr)
{
  tS8 retCode  = I2C_CODE_OK;
  tU8 status   = 0;
  volatile tU8 deviceReady = FALSE;

  while(deviceReady == FALSE)
  {
    //Generate Start condition
    retCode = i2cStart();

    //Transmit SLA+W
    if(retCode == I2C_CODE_OK)
    {
      //write SLA+W
      retCode = i2cPutChar(addr);
      while(retCode == (tS8)I2C_CODE_BUSY)
        retCode = i2cPutChar(addr);
    }

    if(retCode == I2C_CODE_OK)
    {
      //Wait until SLA+W transmitted
      //Get new status
      status = i2cCheckStatus();

      if(status == 0x18)
      {
        //data transmitted and ACK received
        deviceReady = TRUE;
      }
      else if(status == 0x20)
      {
        //data transmitted and ACK not received
        //send start bit, start again
        deviceReady = FALSE;
      }
      else if( status != 0xf8 )
      {
        //error
        retCode = I2C_CODE_ERROR;
        deviceReady = TRUE;
      }
    }

    //Generate Stop condition
    i2cStop();
  }

  return retCode;
}


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
tS8
i2cRead(tU8  addr,
        tU8* pBuf,
        tU16 len)
{
  tS8 retCode = 0;
  tU8 status  = 0;
  tU8 i       = 0;

  //write SLA+R
  retCode = i2cPutChar(addr);
  while(retCode == (tS8)I2C_CODE_BUSY)
    retCode = i2cPutChar(addr);

  if(retCode == I2C_CODE_OK )
  {
    //wait until address transmitted and receive data
    for(i = 1; i <= len; i++ )
    {
      //wait until data transmitted
      while(1)
      {
        //get new status
        status = i2cCheckStatus();

        /*
         * SLA+R transmitted, ACK received or
         * SLA+R transmitted, ACK not received
         * data byte received in master mode, ACK transmitted
         */
        if((status == 0x40 ) || (status == 0x48 ) || (status == 0x50 ))
        {
          //data received
          if(i == len)
          {
            //Set generate NACK
            retCode = i2cGetChar(I2C_MODE_ACK1, pBuf);
          }
          else
          {
            retCode = i2cGetChar(I2C_MODE_ACK0, pBuf);
          }

          //Read data
          retCode = i2cGetChar(I2C_MODE_READ, pBuf);
          while(retCode == (tS8)I2C_CODE_EMPTY)
            retCode = i2cGetChar(I2C_MODE_READ, pBuf);
          pBuf++;
          break;
        }

        //no relevant status information
        else if(status != 0xf8 )
        {
          //error
          i = len;
          retCode = I2C_CODE_ERROR;
          break;
        }
      }
    }
  }

  //Generate Stop condition
  i2cStop();

  return retCode;
}



