#include "mcc_generated_files/mcc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

//Methods
void I2C_Write(uint8_t address, uint8_t *data, uint8_t length);
void I2C_Read(uint8_t adr, uint8_t *read_data,  uint8_t length);
void I2C_init();

//Global variables
uint8_t temp_address = 0b1000000;    //Si7021 address
uint8_t display_address = 0b0111100; //display address
    
uint8_t read_data[4];
    
uint8_t display_init[] = {0x00, 0x38 ,0x0C ,0x06}; //Display Initiate Command
uint8_t clear_line[] = {0x00, 0x01};    //Display Clear Command
    
uint8_t first_line[] = {0x00, 0x80}; //Display Change to First Line Command
uint8_t second_line[] = {0x00, 0xC0}; //Display Change to Second Line Command
    
uint8_t temp_measure[]  = {0xE0}; //Display Get Temp Command
uint8_t humi_measure[]   = {0xE5}; //Display Get Humid Command

char commands[100];
float index = 0;

char headerOut[128];
char tempOut[64];
char htmlOut[100];

typedef struct {
    const char * name;
    void (*func)(void);
} stateFunctionRow_t;

typedef enum {
    ST_Reset,
    ST_WifiModeOn,
    ST_WifiConnect,
    ST_GetIp,
    ST_SetConnections,
    ST_MaxConnections,
    ST_CreateServer,
    ST_IDLE,
    ST_PrepareHeader,
    ST_SendHeader,
    ST_PrepareHtml,
    ST_SendHtml
            

} state_t;

typedef struct {
    state_t currState;
} stateMachine_t;

/// \brief      All the possible events that can occur for this state machine.
/// \details    Unlike states_t, these do not need to be kept in a special order.
typedef enum {
    EV_READY,
    EV_OK,
    EV_ERROR,
    EV_CONNECTED
} event_t;

void StateMachine_Init(stateMachine_t * stateMachine);
void StateMachine_RunIteration(stateMachine_t *stateMachine, event_t event);
const char * StateMachine_GetStateName(state_t state);

void State_Reset();
void State_WifiModeOn();
void State_WifiConnect();
void State_GetIp();
void State_SetConnections();
void State_MaxConnections();
void State_CreateServer();
void State_Idle();
void State_PrepareHeader();
void State_SendHeader();
void State_PrepareHtml();
void State_SendHtml();

static stateFunctionRow_t stateFunctionA[] = {
      // NAME               // FUNC
    { "ST_Reset",           &State_Reset },         
    { "ST_WifiModeOn",      &State_WifiModeOn },    
    { "ST_WifiConnect",     &State_WifiConnect },        
    { "ST_GetIp",           &State_GetIp },           
    { "ST_SetConnections",  &State_SetConnections },    
    { "ST_MaxConnections",  &State_MaxConnections },
    { "ST_CreateServer",    &State_CreateServer},   
    { "ST_IDLE",            &State_Idle },          
    { "ST_PrepareHeader",   &State_PrepareHeader},
    { "ST_SendHeader",      &State_SendHeader},
    { "ST_PrepareHtml",     &State_PrepareHtml},
    { "ST_SendHtml",        &State_SendHtml}
};

typedef struct {
    state_t currState;
    event_t event;
    state_t nextState;
} stateTransMatrixRow_t;



static stateTransMatrixRow_t stateTransMatrix[] = {
    // CURR STATE    // EVENT        // NEXT STATE
    { ST_Reset,             EV_READY,       ST_WifiModeOn},
    { ST_Reset,             EV_ERROR,       ST_Reset },
    { ST_WifiModeOn,        EV_OK,          ST_WifiConnect},
    { ST_WifiModeOn,        EV_ERROR,       ST_Reset  },
    { ST_WifiConnect,       EV_OK,          ST_GetIp},
    { ST_WifiConnect,       EV_ERROR,       ST_Reset },
    { ST_GetIp,             EV_OK,          ST_SetConnections},
    { ST_GetIp,             EV_ERROR,       ST_Reset },
    { ST_SetConnections,    EV_OK,          ST_MaxConnections},
    { ST_SetConnections,    EV_ERROR,       ST_Reset },
    { ST_MaxConnections,    EV_OK,          ST_CreateServer},
    { ST_MaxConnections,    EV_ERROR,       ST_Reset},
    { ST_CreateServer,      EV_OK,          ST_IDLE},
    { ST_CreateServer,      EV_ERROR,       ST_Reset},
    { ST_IDLE,              EV_CONNECTED,   ST_PrepareHeader  },
    { ST_IDLE,              EV_ERROR,       ST_Reset  },
    { ST_PrepareHeader,     EV_OK,          ST_SendHeader  },
    { ST_PrepareHeader,     EV_ERROR,       ST_Reset  },
    { ST_SendHeader,        EV_OK,          ST_PrepareHtml},
    { ST_SendHeader,        EV_ERROR,       ST_Reset  },
    { ST_PrepareHtml,       EV_OK,          ST_SendHtml},
    { ST_PrepareHtml,       EV_ERROR,       ST_Reset  },
    { ST_SendHtml,          EV_OK,          ST_SendHtml},
    { ST_SendHtml,          EV_ERROR,       ST_Reset  },


};

void StateMachine_Init(stateMachine_t * stateMachine) {
    stateMachine->currState = ST_Reset;
}

//Method to read character input to print to console
/*
char getInput()
{
    if (EUSART1_is_rx_ready()) {
        char tmp = EUSART1_Read();
        printf("%c", tmp);
        PORTB = tmp;

        return tmp;
    } else {
        return false;
    }
}
*/


char* parseEusartInput(void);
stateMachine_t stateMachine;


//Resets device before usage
void State_Reset()
{
    printf("%s\r\n", "AT+RESTORE");
}

//Turns wifi on
void State_WifiModeOn() {
      printf("%s\r\n", "AT+CWMODE=1");
}

//Connects wifi
void State_WifiConnect() {
    printf("%s\r\n", "AT+CWJAP=\"WuggaNet\",\"FredagsBanan\"");
    //printf("%s\r\n", "AT+CWJAP=\"Sahin\",\"Hakan0909\"");

}

//Gets ip
void State_GetIp() {
    printf("%s\r\n", "AT+CIFSR");
}

//multi connection mode
void State_SetConnections(){
    printf("%s\r\n", "AT+CIPMUX=1");
}

//number of connections
void State_MaxConnections(){
    printf("%s\r\n", "AT+CIPSERVERMAXCONN=1");
}

//create the server
void State_CreateServer(){
    printf("%s\r\n", "AT+CIPSERVER=1");
}

//wait for client to connect to host
void State_Idle() {
    I2C_Write(display_address, second_line, 2); 
    I2C_Write(display_address, "@Idle Mode         ", 17); 
}

//prepare to send header before html
void State_PrepareHeader(){
    char str[80];
    I2C_Write(display_address, second_line, 2);
    I2C_Write(display_address, first_line, 2);

    //Humidity
    I2C_Write(temp_address, humi_measure, 1); 
    I2C_Read(temp_address, read_data, 2);
    uint16_t humidity = (read_data[0] << 8 ) + read_data[1];

    I2C_Write(display_address, first_line, 2);
    float RH = ((125 * (float) humidity)/65536) -6;
    float Humid = RH;

    //Temperature
    I2C_Write(temp_address, temp_measure, 1);
    I2C_Read(temp_address, read_data, 2); 
    uint16_t temperature = (read_data[0] << 8 ) + read_data[1];

    I2C_Write(display_address, second_line, 2);
    float Temp = ((175.72 * (float) temperature)/65536)-46.85;
   
    sprintf(htmlOut, "<html><head></head><body><h1>Main Page</h1><h1>Temp: %.1f C</h1><h1>Humid: %.1f &#37; </h1></body></html>", Temp, Humid);
    strncpy(headerOut, "HTTP/1.1 200 OK\nContent-type:text/html\n", 128);
    snprintf(tempOut, 64, "Content-length:%d\nConnection:close\n\n",strlen(htmlOut));
    strncat(headerOut, tempOut, 128);

    printf("AT+CIPSEND=0,%d\r\n", strlen(headerOut));
}

//send the header
void State_SendHeader() {
    printf(headerOut);

    I2C_Write(display_address, second_line, 2);
    I2C_Write(display_address, "@Sending header..", 17); 
}

//preparation of html before header sending
void State_PrepareHtml() {
    printf("AT+CIPSEND=0,%d\r\n", strlen(htmlOut)); 
    
    I2C_Write(display_address, second_line, 2);
    I2C_Write(display_address, "@Prepare HTML..", 17); 
}

//send html
void State_SendHtml() {
    printf(htmlOut);
    I2C_Write(display_address, second_line, 2);
    I2C_Write(display_address, "@Sending html....", 17); 
}

// Main application
void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();
    I2C_init();

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();
    
    char str[80];
    char str1[80];
    
    //Initiate And Clear
    __delay_ms(2000);
    I2C_Write(display_address, display_init, 4);
    I2C_Write(display_address, clear_line, 2);

    char* eusartData;
    StateMachine_Init(&stateMachine);
    __delay_ms(2000);
  //  printf("%s\r\n", "AT+RESTORE");

    State_Reset();

    while (1)
    { 
        if((eusartData = parseEusartInput())){
            if(!strcmp(eusartData, "ready")){
                StateMachine_RunIteration(&stateMachine, EV_READY);
            }else if(!strcmp(eusartData, "ok") || !strcmp(eusartData, "send ok")){
                StateMachine_RunIteration(&stateMachine, EV_OK);
            }
            }else if(!strcmp(eusartData, "error")){
                StateMachine_RunIteration(&stateMachine, EV_ERROR);
            }if(!strcmp(eusartData, "0,connect")){
                StateMachine_RunIteration(&stateMachine, EV_CONNECTED);
            }
        }
}

char* parseEusartInput(){
    static char textIn[42];
    static char index = 0;
    char temp;
    
    if(!EUSART1_is_rx_ready()) return 0x00;
    temp = EUSART1_Read();
    if(isprint(temp)){
        textIn[index++] = tolower(temp);
    }
    
    textIn[index] = 0x00;
    
    if(index >= (sizeof(textIn) - 1)) index--;
    
    if(temp == '\n'){
        index = 0;
        return textIn;
    }
    return 0;
}

void StateMachine_RunIteration(stateMachine_t *stateMachine, event_t event) {

    // Iterate through the state transition matrix, checking if there is both a match with the current state
    // and the event
    for(int i = 0; i < sizeof(stateTransMatrix)/sizeof(stateTransMatrix[0]); i++) {
        if(stateTransMatrix[i].currState == stateMachine->currState) {
            if((stateTransMatrix[i].event == event)) {

                // Transition to the next state
                stateMachine->currState =  stateTransMatrix[i].nextState;

                // Call the function associated with transition
                (stateFunctionA[stateMachine->currState].func)();
                break;
            }
        }
    }
}

const char * StateMachine_GetStateName(state_t state) {
    return stateFunctionA[state].name;
}
void I2C_init()
{
    TRISCbits.RC3 = 1; 
    TRISCbits.RC4 = 1;

    ANSELCbits.ANSC3 = 0;
    ANSELCbits.ANSC4 = 0;

    SSP1ADD = _XTAL_FREQ / (4 + 100000) - 1;  
    SSP1CON1 = 0b00101000;                                
    SSP1CON2 = 0;
}

//I2C Read Data function
void I2C_Read(uint8_t adr, uint8_t *read_data,  uint8_t length) 
{
    SSP1CON2bits.SEN = 1;
    while (SSP1CON2bits.SEN == 1) {};
    
    //Adds Read bit (since adr starts as 7 bit)
    uint8_t targerAdr = (adr << 1) +1; 
    SSP1BUF = targerAdr; 
     
    while(SSP1STATbits.R_nW){};
    if(SSP1CON2bits.ACKSTAT == 0)
    {
        int i;  
        for(i = 0; i < length; i++)
        {
            SSP1CON2bits.RCEN = 1;
            while(SSP1CON2bits.RCEN ==1){}
            read_data[i] = SSP1BUF;

           // printf("%d\r\n", read_data[i]);
            if(i == length -1)
            {
               SSP1CON2bits.ACKDT = 1; 
            }
            else
            {
                SSP1CON2bits.ACKDT = 0; 
            }
            SSP1CON2bits.ACKEN = 1;
            while(SSP1CON2bits.ACKEN == 1);
        }
    }
    else{
     //   printf("Error code 1! Read");
    }
    SSP1CON2bits.PEN = 1;
    while(SSP1CON2bits.PEN == 1){}
}

//I2C Write Data Method
void I2C_Write(uint8_t address, uint8_t *data, uint8_t length) 
{
    SSP1CON2bits.SEN = 1;
    while(SSP1CON2bits.SEN == 1);
    
    //Adds Write bit (since adr starts as 7 bit)
    SSP1BUF = (address << 1) + 0;
    while(SSP1STATbits.R_nW); 
    if(SSP1CON2bits.ACKSTAT == 0)
    {
        int i;
        for (i = 0; i < length; i++) 
        { 
            SSP1BUF = (uint8_t) data[i];
            while(SSP1STATbits.R_nW){};
        
            if(SSP1CON2bits.ACKSTAT == 0)
            {
         //       printf("ACK\r\n");
            }
            else
            {
         //       printf("NACK\r\n");
            }  
        }
        SSP1CON2bits.PEN = 1;
        while(SSP1CON2bits.PEN == 1){}
    }
    else
    {
        //printf("Error code 1! Write");
    }
}
    
