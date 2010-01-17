#include "uart.h"
extern volatile tU32 msClock;
#define SM_INIT '1'
#define SM_POLL '2'
#define SM_OPERATE '3'
#define SM_FIND '4'
#define SM_ACK '5'
#define SM_ERROR '6'

#define SM_EOM "EOM"

typedef struct smarthome_op { //total of 14 bytes
	tU8 op;
	tU8 to;
	tU8 from;
	tU8 data[8];
	tU8 EOM[3];	//unused..
} SM_OP;

void SM_resetSMOP(SM_OP* op);

extern volatile tU32 msClock;

void SM_parseOpFromUART(SM_OP* op) {
  tU32 rxChars = 0;
  tU8 rxChar = 0;
  tU32 timeStamp = msClock;

  SM_resetSMOP(op);
    
	//TODO: use timer to prevent from timeout
  while((msClock - timeStamp) < 1000)
  {
    //check if any character has been received
    while (TRUE == uart1GetChar(&rxChar))
    {
      if (rxChar == '\r')
	//end of op
	printf("\n%c",rxChar);
      else {
	printf("%c",rxChar);
        ((tU8*)op)[rxChars] = rxChar;
      }

      rxChars++;
    }
  }

  if (rxChars > 0 && !(op->EOM[0] == 'E' && op->EOM[1] == 'O' && op->EOM[2] == 'M'))
	printf("ERROR READING OP");
}

void SM_sendOpOnUART(SM_OP* op) {
  uart1SendChars((char*)op,14);
  uart1SendCh('\n');
}

void SM_printOp(SM_OP* op) {
  printf("SM_OP: type: %c, to: %c, from: %c, data: %s\n",op->op,op->to,op->from,op->data);
}

tU8 SM_isGoodOp(SM_OP* op) {
  return (op->EOM[0] == 'E' && op->EOM[1] == 'O' && op->EOM[2] == 'M');
}

void SM_resetSMOP(SM_OP* op) {
volatile tS8 i;
  op->op = 0;
  op->to = 0;
  op->from = 0;
  i=7;
  while(i>=0) op->data[i--] = 0;
  i=2;
  while(i>=0) op->EOM[i--] = 0;
}

void SM_newOP(SM_OP* op) {
  SM_resetSMOP(op);
  op->EOM[0] = 'E'; op->EOM[1] = 'O'; op->EOM[2] = 'M';
}

void SM_createINIT_OP(SM_OP* op, tU8 to) {
  SM_newOP(op); 
  op->op = INIT;
  op->from = '1'; // only  master sends INIT
  op->to = to;
}

void SM_crateACK_OP(SM_OP* op, tU8 from) {

}
