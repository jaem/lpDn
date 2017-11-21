//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the
//  distribution.
//
//  Neither the name of Texas Instruments Incorporated nor the names of
//  its contributors may be used to endorse or promote products derived
//  from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//****************************************************************************
#include "i2c_driver.h"

//***** Definitions *****

//***** Function Prototypes *****

//***** Global Data *****
volatile EUSCI_B_I2C_initMasterParam i2cConfig =
{
        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
		0,
		EUSCI_B_I2C_SET_DATA_RATE_400KBPS,      // Desired I2C Clock of 400khz
        0,                                      // No byte counter threshold
        EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD                // Autostop
};

volatile eUSCI_status ui8Status;
uint8_t* pData;
uint8_t ui8DummyRead;
uint32_t g_ui32ByteCount;
bool     burstMode = false;

/***********************************************************
  Function: initI2C
*/
void initI2C(void)
{
	i2cConfig.i2cClk = CS_getSMCLK();
    i2cConfig.autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP;

    // Assign I2C pins to eUSCI_B2 for MSP430FR5994
	GPIO_setAsPeripheralModuleFunctionInputPin( GPIO_PORT_P7, GPIO_PIN0 + GPIO_PIN1,
	                                            GPIO_PRIMARY_MODULE_FUNCTION );


	EUSCI_B_I2C_initMaster(EUSCI_B0_BASE,  (EUSCI_B_I2C_initMasterParam *) &i2cConfig);
}

bool writeI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint8_t ui8ByteCount)
{
	//WAIT FOR PREVIOUS TRAFFIC TO CLEAR
	while(UCB0STAT & UCBBUSY);

	//ASSIGN DATA TO LOCAL POINTER
	pData = Data;

  	// RESET USCI MODULE
  	UCB0CTLW0 |= UCSWRST;

  	// SETUP THE NUMBER OF BYTES TO TRANSMIT + 1 TO ACCOUNT FOR REGISTER BYTE
  	// NEXT, AUTOMATIC STOP BIT WILL BE GENERATED AFTER LAST BYTE SENT
	UCB0TBCNT = ui8ByteCount + 1;

	// GENERATE AUTOMATIC STOP BIT WHEN UCB0TBCNT = TARGET
  	UCB0CTLW1 |= UCASTP_2;

	// LOAD THE DEVICE SLAVE ADDRESS
	UCB0I2CSA = ui8Addr;

  	// ENABLE USCI MODULE
  	UCB0CTLW0 &= ~UCSWRST;

  	// ENABLE STOP AND NACK INTERRUPTS
	UCB0IE |= UCSTPIE + UCNACKIE;

	ui8Status = eUSCI_BUSY;

	// ENABLE TRANSMIT, GENERATE START BIT
  	UCB0CTLW0 |= (UCTR + UCTXSTT);

  	// WAIT FOR THE FIRST TXBUF EMPTY INTERRUPT
  	while(!(UCB0IFG & UCTXIFG));

  	// LOAD TXBUF WITH THE TARGET REGISTER
  	UCB0TXBUF = ui8Reg;

  	// NOW ENABLE TX INTERRUPT FOR THE REMAINING DATA
  	UCB0IE |= UCTXIE;

	// NOW WAIT FOR DATA BYTES TO BE SENT
	while(ui8Status == eUSCI_BUSY)
	{
#ifdef USE_LPM
		LPM0;
#else
		_no_operation();
#endif
	}

	// DISABLE INTERRUPTS
	UCB0IE &= ~(UCSTPIE + UCNACKIE + UCTXIE);

	if(ui8Status == eUSCI_NACK)
	{
		return(false);
	}
	else
	{
		return(true);
	}
}

bool readI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint8_t ui8ByteCount)
{

	// MAY WANT TO COME BACK AND PUT A TIME-OUT ON WAIT FOR BUSY
	//WAIT FOR PREVIOUS TRAFFIC TO CLEAR
	while(UCB0STAT & UCBBUSY);

	//ASSIGN DATA TO LOCAL POINTER
	pData = Data;

  	// RESET EUSCI MODULE
  	UCB0CTLW0 |= UCSWRST;

  	// SETUP THE NUMBER OF BYTES TO RECEIVE (CAN ONLY BE DONE HERE WHEN EUSCI IN RESET)
  	UCB0TBCNT = ui8ByteCount;

	// GENERATE AUTOMATIC STOP BIT WHEN UCB0TBCNT = TARGET
  	UCB0CTLW1 |= UCASTP_2;

	// LOAD THE DEVICE SLAVE ADDRESS
	UCB0I2CSA = ui8Addr;

  	// RESUME
  	UCB0CTLW0 &= ~UCSWRST;

  	// ENABLE STOP, BYTECOUNT AND NACK INTERRUPTS
	UCB0IE |= (UCSTPIE + UCNACKIE);

	ui8Status = eUSCI_BUSY;

	// ENABLE TRANSMIT, GENERATE START BIT
  	UCB0CTLW0 |= (UCTR + UCTXSTT);

  	// WAIT FOR THE FIRST TXBUF EMPTY INTERRUPT BEFORE LOADING TARGET REGISTER
  	while(!(UCB0IFG & UCTXIFG));

  	// LOAD TXBUF WITH THE TARGET REGISTER
  	UCB0TXBUF = ui8Reg;

  	// TRICKY SPOT
  	// NOTE: IF NUMBER OF BYTES TO RECEIVE = 1, THEN AS TARGET REGISTER IS BEING SHIFTED OUT DURING THE WRITE PHASE,
  	// UCB0TBCNT WILL BE COUNTED AND WILL TRIGGER STOP BIT PREMATURELY.
  	// IF BYTE COUNT IS > 1, WAIT FOR THE NEXT TXBUF EMPTY INTERRUPT (JUST AFTER REGISTER VALUE HAS BEEN SHIFTED OUT)
  	// THE FOLLOWING CODE HANDLES BOTH SCENARIOS.

	while(ui8Status == eUSCI_BUSY)
	{
		if(UCB0IFG & UCTXIFG)
			ui8Status = eUSCI_IDLE;
	}

	ui8Status = eUSCI_BUSY;

	//TURN OFF TRANSMIT (ENABLE RECEIVE)
   	UCB0CTL1 &= ~UCTR;

   	// GENERATE (RE-)START BIT
  	UCB0CTL1 |= UCTXSTT;

  	// NOW ENABLE RX INTERRUPT TO RECEIVE THE DATA
  	UCB0IE |= UCRXIE;

	// WAITING FOR ALL THE DATA TO BE RECEIVED
	while(ui8Status == eUSCI_BUSY)
	{

#ifdef USE_LPM
		LPM0;
#else
		_no_operation();
#endif
	}

	// DISABLE INTERRUPTS
	UCB0IE &= ~(UCSTPIE + UCNACKIE + UCRXIE);

	if(ui8Status == eUSCI_NACK)
	{
		return(false);
	}
	else
	{
		return(true);
	}
}

bool readBurstI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint32_t ui32ByteCount)
{
	// MAY WANT TO COME BACK AND PUT A TIME-OUT ON WAIT FOR BUSY
	//WAIT FOR PREVIOUS TRAFFIC TO CLEAR
	while(UCB0STAT & UCBBUSY);;

	/* Assign Data to local Pointer */
	pData = Data;

  	// RESET EUSCI MODULE
  	UCB0CTLW0 |= UCSWRST;

	// GENERATE AUTOMATIC STOP BIT WHEN UCB0TBCNT = TARGET
  	UCB0CTLW1 &= ~UCASTP_2;

  	g_ui32ByteCount = ui32ByteCount;
    burstMode = true;

	// LOAD THE DEVICE SLAVE ADDRESS
	UCB0I2CSA = ui8Addr;

  	// RESUME
  	UCB0CTLW0 &= ~UCSWRST;

  	// ENABLE STOP, BYTECOUNT AND NACK INTERRUPTS
	UCB0IE |= (UCSTPIE + UCNACKIE);

	ui8Status = eUSCI_BUSY;

	// ENABLE TRANSMIT, GENERATE START BIT
  	UCB0CTLW0 |= (UCTR + UCTXSTT);

  	// WAIT FOR THE FIRST TXBUF EMPTY INTERRUPT BEFORE LOADING TARGET REGISTER
  	while(!(UCB0IFG & UCTXIFG));

  	// LOAD TXBUF WITH THE TARGET REGISTER
  	UCB0TXBUF = ui8Reg;

  	// TRICKY SPOT
  	// NOTE: IF NUMBER OF BYTES TO RECEIVE = 1, THEN AS TARGET REGISTER IS BEING SHIFTED OUT DURING THE WRITE PHASE,
  	// UCB0TBCNT WILL BE COUNTED AND WILL TRIGGER STOP BIT PREMATURELY.
  	// IF BYTE COUNT IS > 1, WAIT FOR THE NEXT TXBUF EMPTY INTERRUPT (JUST AFTER REGISTER VALUE HAS BEEN SHIFTED OUT)
  	// THE FOLLOWING CODE HANDLES BOTH SCENARIOS.
	while(ui8Status == eUSCI_BUSY)
	{
		if(UCB0IFG & UCTXIFG)
			ui8Status = eUSCI_IDLE;
	}

	ui8Status = eUSCI_BUSY;

	//TURN OFF TRANSMIT (ENABLE RECEIVE)
   	UCB0CTL1 &= ~UCTR;

   	// GENERATE (RE-)START BIT
  	UCB0CTL1 |= UCTXSTT;

  	// NOW ENABLE RX INTERRUPT TO RECEIVE THE DATA
  	UCB0IE |= UCRXIE;

	// WAITING FOR ALL THE DATA TO BE RECEIVED
	while(ui8Status == eUSCI_BUSY)
	{

#ifdef USE_LPM
		LPM0;
#else
		_no_operation();
#endif
	}

	// DISABLE INTERRUPTS
	UCB0IE &= ~(UCSTPIE + UCNACKIE + UCRXIE);

	if(ui8Status == eUSCI_NACK)
	{
		return(false);
	}
	else
	{
		return(true);
	}
}

#pragma vector=USCI_B2_VECTOR
__interrupt void USCI_B2_ISR(void)
{
  switch(__even_in_range(UCB0IV, 0x1E))
  {
	  case 0x00:break;		// NO INTERRUPT
	  case 0x02:break;		// ARBITRATION LOST INTERRUPT

	  case 0x04: // NACK INTERRUPT
	  {
		  // MUST GENERATE STOP BIT WHEN SLAVE NACKS
		  UCB0CTL1 |= UCTXSTP;

		  // MUST CLEAR ANY PENDING TX INTERRUPTS ACCORDING TO MIGRATION DOC
		  UCB0IFG &= ~UCTXIFG;

		  // SET OUR LOCAL STATE TO NACK
		  ui8Status = eUSCI_NACK;
		  break;
	  }
	  case 0x06: // START BIT INTERRUPT - SLAVE MODE ONLY
	  {
		  ui8Status = eUSCI_START;
		  break;
	  }
	  case 0x08: // STOP BIT INTERRUPT
	  {
		  ui8Status = eUSCI_STOP;
		   break;
	  }
	  case 0x0A: break;   	// DATA - SLAVE3_RXIFG
	  case 0x0C: break; 	// DATA - SLAVE3_TXIFG
	  case 0x0E: break;  	// DATA - SLAVE2_RXIFG
	  case 0x10: break;		// DATA - SLAVE2_TXIFG
	  case 0x12: break;		// DATA - SLAVE1_RXIFG
	  case 0x14: break;   	// DATA - SLAVE1_TXIFG
	  case 0x16: 			// DATA - MASTER/SLAVE0_RXIFG
	  {
		  *pData++ = UCB0RXBUF;
		  ui8DummyRead = UCB0RXBUF;

	    	if (burstMode)
	    	{
	    		g_ui32ByteCount--;
	    		if (g_ui32ByteCount == 1)
	    		{
	    			burstMode = false;

	    			/* Generate STOP */
	    			UCB0CTL1 |= UCTXSTP;
	    		}
	    	}

		  break;
	  }
	  case 0x18:			// DATA - MASTER/SLAVE0_TXIFG
	  {
		  UCB0TXBUF = *pData++;
		  break;
	  }
	  case 0x1A: break;	 	// BYTE COUNTER ZERO IFG - AVAILABLE IN SLAVE MODE ONLY
	  case 0x1C: break; 	// CLOCK LOW TIMEOUT IFG
	  case 0x1E: break; 	// 9TH POSITION IFG
	  default: break;
  }
  // NOT SURE IF BACK GROUND IN LOW POWER MODE SO EXIT LPM3 JUST IN CASE
	LPM3_EXIT;
}
