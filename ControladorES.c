#include <16F877A.h>

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#use delay(clock=4000000)

#define eeprom_address long int 
#define MODBUS_TYPE MODBUS_TYPE_SLAVE
#define MODBUS_SERIAL_TYPE MODBUS_RTU     //use MODBUS_ASCII for ASCII mode
#define MODBUS_SERIAL_RX_BUFFER_SIZE 64
#define MODBUS_SERIAL_BAUD 9600
#define MODBUS_SERIAL_INT_SOURCE MODBUS_INT_RDA
#define MODBUS_ADDRESS 3
#define use_low_in TRUE
//#define use_low_out TRUE

int seconds;
int minutes;
int hours;
int date;
int mounth;
int year_high = 20;
int year_low;

#include <in_out.c>
#include <modbus.c>
#include <EEPROMmemory.c>
#include <rtcDS1307.c>

EEPROM_ADDRESS address = 0;
EEPROM_ADDRESS access_address = 2;   //direccion corriente de la memoria de accesos
BYTE access_event_count = 0;
int16 mbus_data[16];

//Funcion que obtiene la direccion en la red del controlador seteada a traves de jumpers
int get_modbus_address()
{
	int temp = input_a();
	temp &= 0b00001111;
	return temp;
}

//Funcion que crea la data que hay que pasar como respuesta al master
void regist_data(int index, int offset)
{
   int high, low, i;
   for(i = 0; i < offset; i++){
      high = read_regist_memory(index);
      index++;
      low = read_regist_memory(index);
      index++;
      mbus_data[i] = make16(high, low);
   }
}

//Funcion que crea la data que hay que pasar como respuesta al master
void conf_data(int index, int offset)
{
   int high, low, i;
   for(i = 0; i < offset; i++){
      high = read_white_memory(index);
      index++;
      low = read_white_memory(index);
      index++;
      mbus_data[i] = make16(high, low);
   }
}

//Funcion que limpia los registros
void clean_registers()
{
	write_regist_memory(0, 0);
	write_regist_memory(1, 0);
	access_address = 2;
	access_event_count = 0;
}

void main() 
{
  modbus_init();
  in_out_init();
  
  int i;
  int16 out_register = 0;
  int in_state = 0;
  short action;
  
  clean_registers();
  
  //int MODBUS_ADDRESS = get_modbus_address();

  /*write_white_memory(1, 0b00000001);
  write_white_memory(3, 0b00000010);
  write_white_memory(5, 0b00000101);
  write_white_memory(7, 0b00001000);

  minutes = 8;
  hours = 8;
  date = 9;
  mounth = 1;
  year_low = 14;

  rtc_init(minutes, hours, date, mounth, year_low); */	
  
  while (TRUE){
      //Se actualiza la salida mientras no se reciba mensaje modbus
      while(!modbus_kbhit()){
		  for(i = 0; i < inport_lenght; i++){
			//Buscando cambio con respecto a la encuesta anterior
	        if(bit_test(inport(), i) != bit_test(in_state, i)){
				in_state = inport();
				action = !bit_test(in_state, i);	//Entrada activada, action 0

                //Registrando el acceso
                real_time(seconds, minutes, hours, date, mounth, year_low);

				write_regist_memory(access_address, i);
				access_address++;
				write_regist_memory(access_address, minutes);
				access_address++;
				write_regist_memory(access_address, hours);
				access_address++;
				write_regist_memory(access_address, date);
				access_address++;
				write_regist_memory(access_address, mounth);
				access_address++;
				write_regist_memory(access_address, year_high);
				access_address++;
				write_regist_memory(access_address, year_low);
				access_address++;
				write_regist_memory(access_address, action);
				access_address++;

    			real_time(seconds, minutes, hours, date, mounth, year_low);

   				access_event_count++;
				if(access_address == access_event_count * 8 - 1){
					access_address = 2;
					access_event_count = 0;
				}
				write_regist_memory(1, access_event_count); 
			}
			//Conformando la salida 
		    if(bit_test(inport(), i))
                out_register |= make16(read_eeprom(2 * i), read_eeprom(2 * i + 1));  //Si se usa entrada de 8 bits la parte alta de la configuracion de cada bit es 0
	     }
	     outport(out_register);
         out_register = 0;
      }
      
      delay_us(50);
      //Se ejecuta la funcion modbus recibida
      if((modbus_rx.address == MODBUS_ADDRESS) || modbus_rx.address == 0)
      {
         switch(modbus_rx.func)
         {
            case FUNC_WRITE_SINGLE_COIL:	//Esta funcion se utiliza para limpiar los registros
				  clean_registers();
				  modbus_write_single_coil_rsp(MODBUS_ADDRESS,modbus_rx.data[1],((int16)(modbus_rx.data[2]))<<8);
				  break;
			   case FUNC_READ_HOLDING_REGISTERS:	//Esta funcion sera utilizada para leer la lista blanca
                  conf_data(modbus_rx.data[1],modbus_rx.data[3]);
                  modbus_read_holding_registers_rsp(MODBUS_ADDRESS,(modbus_rx.data[3]*2),mbus_data);
                  break;
               case FUNC_READ_INPUT_REGISTERS:	//Esta funcion sera utilizada para leer los registros de accesos producidos
                  regist_data(modbus_rx.data[1],modbus_rx.data[3]);
                  modbus_read_input_registers_rsp(MODBUS_ADDRESS,(modbus_rx.data[3]*2),mbus_data);
				  break;
            case FUNC_WRITE_MULTIPLE_REGISTERS:   
				  //Actualizando la hora 
				  minutes = modbus_rx.data[5];
				  hours = modbus_rx.data[6];
				  date = modbus_rx.data[7];
				  mounth = modbus_rx.data[8];
				  year_high = modbus_rx.data[9];
				  year_low = modbus_rx.data[10];
				  rtc_init(minutes, hours, date, mounth, year_low); 
                  address = modbus_rx.data[1] * 2; //un registro modbus contiene dos registros de memoria EEPROM
                  for(i = 11; i < modbus_rx.data[4]; i++){
                     write_white_memory(address, modbus_rx.data[i]);
                     address++;
                  }
                  modbus_write_multiple_registers_rsp(MODBUS_ADDRESS, make16(modbus_rx.data[0],modbus_rx.data[1]), make16(modbus_rx.data[2],modbus_rx.data[3]));
                  break; 
            default:    //We don't support the function, so return exception
               modbus_exception_rsp(MODBUS_ADDRESS,modbus_rx.func,ILLEGAL_FUNCTION);
         }
      }
  }
}
