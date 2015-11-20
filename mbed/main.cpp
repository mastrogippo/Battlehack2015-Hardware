//Hello code reviewres. The code running on my mbed board is based on an old 
//project of mine, a CAN bus sniffer.
//This project should compile on a standard mbed environment.
//I'm copying and pasting code here since the mbed online compile doesn't
//support github. I can show you the mbed's internal repository if needed.

//For refefence, modified code will be enclosed in "//BH" and "//-BH" tags

#include "mbed.h"
#include <vector>
#include <algorithm>

DigitalOut mgled(LED2);
DigitalOut rx_led(LED3);
DigitalOut filter_led(LED4);
Serial pc(USBTX, USBRX);

//CAN can1(p30, p29);
CAN can1(p9, p10);
CAN can2(p30, p29);

Timer uptime;
bool filterPackets = false;
vector<int> filteredPackets;
extern "C" void mbed_reset();

const char Ban[8] = {0x09, 0x04, 0x00, 0x00, 0x07, 0x08};
//const char Ban[8] = {0x07, 0x97, 0xD0, 0x54, 0x6B, 0xF0, 0x00, 0x6F};
//char Ban[8] = {0,0,0,0,0,0,0,0};

//BH
Serial particle(p13,p14);
DigitalOut Windows(p20);

struct car_s{
    unsigned int Wheel_pos;
    int Wheel_tor;
    char Handbrake;
    unsigned int Accel;
    unsigned char Gear;
    unsigned char Doors;
    unsigned char D_op;
    };
    
car_s car;

//-BH

DigitalOut myled(LED1);

int ParZ = 0;

char smsg[50];
int cn = 0;
bool flagser_getend;

void SerGetline() {
    char temp;
    while(pc.readable())
    {
        temp = pc.getc();
        
        if(temp != '\n')
        {
            smsg[cn] = temp;
            if(cn >= 49) cn = 0; //reset
            
            if(temp == '\r'){
                smsg[cn] = '\0';
                flagser_getend = 1;
            }
            cn ++;
        }
    }
}

void SerInt() {
    int num;
    char tc;
    SerGetline();
/*    if(flagser_getend){
        if(sscanf(smsg, "%c%i", &tc, &num) >= 1) {
            switch(tc)
            {
                case 'S':
                    switch(num)
                    {
                        case 4: can1.frequency(125000); break;
                        case 5: can1.frequency(250000); break;
                        case 6: can1.frequency(500000); break;
                        case 8: can1.frequency(1000000); break;
                    }
                break;
                case 'Z': ParZ = num; break;
                case 'C': can1.monitor(true); break;
                case 'O': 
                    //can1.reset();
                    can1.monitor(false); 
                break;
            }
            pc.printf("Cmd: %c n: %d\r", tc, num);
        }
        cn = 0;
        flagser_getend = 0;
    }*/
}

void SerialStart(void) {
    pc.baud(115200);
    //pc.printf("$PSRF103,2,0,0,1*26\r\n");
    flagser_getend = 0;
    pc.attach(&SerInt,Serial::RxIrq);
    cn = 0;
}

void processMessage(){
    // Turn on rx_led to indicate we are receiving data
    rx_led = 1;
    
    // Create a CANMessage object and read the buffer into it
    CANMessage rxmsg;
    can1.read(rxmsg);
    
    //BH
    short tmpi;
    /*unsigned int Wheel_pos;
    int Wheel_tor;
    char Handbrake;
    unsigned int Accel;
    unsigned char Gear;*/
    switch(rxmsg.id)
    {
        case 0x156:
            tmpi = rxmsg.data[0];
            tmpi <<= 8;
            tmpi += rxmsg.data[1];
            //range:e9e6-1612 (-5658 +5650)
            //center: 0x150 (weird?)
            tmpi = (int)((float)(tmpi+5660-0x150)/56.5f);
            if(tmpi < 1) tmpi = 1; //bad hack, weird stuff happening
            car.Wheel_pos = tmpi;
            
            car.Wheel_tor = rxmsg.data[2];
            car.Wheel_tor <<= 8;
            car.Wheel_tor += rxmsg.data[3];
            
            //pc.printf("W%d-%d\r\n", car.Wheel_pos, car.Wheel_tor);
            break;
        
        case 0x1A6:
            car.Handbrake = (rxmsg.data[0] & 0x04) >> 2;
            //pc.printf("H%d\r\n", car.Handbrake);
            break;
            
        case 0x17C:
            car.Accel = rxmsg.data[0];
            car.Accel <<= 8;
            car.Accel += rxmsg.data[1];
            //pc.printf("Acc%d\r\n", car.Accel);
            break;
            
        case 0x188:
            car.Gear = rxmsg.data[3];
            //pc.printf("G%02X\r\n", car.Gear);
            break;
            
        case 0x405:
            //car.D_op
            car.Doors = (rxmsg.data[4] & 0xE0) + (rxmsg.data[5] & 0x03);
            //pc.printf("D%X\r\n", Car.Doors);
            break;
        
    }
    //pc.printf("W%0.4X|G%02X\r\n",car.Wheel_pos, car.Gear);
    //-BH
    
    /*
    pc.printf("t%03X", rxmsg.id);                 // Full header for legacy reasons (4 bytes)
    pc.printf("%d", rxmsg.len);                    // Length of message (0-8 bytes typically)        
    for (int i = 0; i < rxmsg.len; i++)
        pc.printf("%02X", rxmsg.data[i]);  // Print additional byte(s) with a preceeding space
    pc.printf("%04X", (int)(uptime.read()*100));    
    pc.printf("\r");                      // Print carriage return and newline
    */   
    // Turn off our rx_led as we have finished reading data
    rx_led = 0;
}

int main() {
    //BH
    Windows = 0;
    particle.baud(9600);
    //-BH
    
    SerialStart(); //pc
    
    pc.printf("c1(%d)\n", can1.frequency(500000));
    pc.printf("c2(%d)\n", can2.frequency(500000));
    
    // Start capturing packets
    uptime.start();
    can1.attach(&processMessage);
        
    unsigned short pp = 0;
    while(1) {        
        //BH
        
        
        //test
        pc.printf("W%04X (%d)|D%02X\r",pp>>2,pp>>2, car.Doors);
        
        //particle.printf("U%d %d\r",pp>>2, car.Doors);//, Rpm,wheel,temp,pot
        //parsing issues in particle? :(
        particle.putc(0xFF);//start
        //particle.putc((unsigned char)(pp>>2));//wheel
        //particle.putc((unsigned char)(0xF0-pp>>2));//doors
        particle.putc((unsigned char)(car.Wheel_pos));//wheel
        particle.putc((unsigned char)(car.Doors));//doors
        particle.putc(0xFE);//end
        
        
        /*pp++;
        if(pp > (200<<2))
            pp = 0;  
        //test*/
            
        wait(0.1); 
        //pc.printf("W%04X (%d)|D%02X\r",car.Wheel_pos,car.Wheel_pos, car.Doors);
        //particle.printf("U%d|%d\r",car.Wheel_pos, car.Doors);//, Rpm,wheel,temp,pot
        
        
        //      pc.printf("U%d|%d\r",car.Wheel_pos, car.Doors);//, Rpm,wheel,temp,pot
        //if(can2.write(CANMessage(0x156, Ban, 6))) 
        {
            //pc.printf("TX_"); 
        }
        //--BH
    }
}