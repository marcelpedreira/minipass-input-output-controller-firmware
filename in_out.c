//////////////////////////////////////////////////////////////////////////////////////////
////                                      in_out.c                                    ////
////                                                                                  ////
////                          Controlador de entradas/salidas                         ////
////                                                                                  ////
//////////////////////////////////////////////////////////////////////////////////////////
////                                                                                  ////
////  Configuracion de HW:                                                            ////
////                                                                                  ////
////  MODO 1: 8 entradas en el puerto B                                               ////
////          8 salidas en el puerto D                                                ////
////                                                                                  ////
////  MODO 2: Hasta 16 entradas, LSB en el puerto B y MSB en el puerto A              ////
////          Hasta 16 salidas, LSB en el puerto D y MSB en el puerto E               ////
////                                                                                  ////
////  MODO 1 por definicion                                                           ////
////  Entradas/Salidas activas a nivel alto por definicion                            ////
////                                                                                  ////
////  Definir:                                                                        ////
////  use_16bit_port       Configuracion de HW en MODO 2                              ////
////  use_low_in           Entradas activas a nivel bajo                              ////
////  use_low_out          Salidas activas a nivel bajo                               ////
////                                                                                  ////
////  Funciones:                                                                      ////
////  in_out_init()                                                                   ////
////    - Inicializacion del sistema                                                  ////
////                                                                                  ////
////  type inport()                                                                   ////
////    - Lectura del puerto de entrada                                               ////
////                                                                                  ////
////  outport(int16 out_register)                                                     ////
////    - Escritura en el puerto de salida                                            ////
////                                                                                  ////
////  Nota:                                                                           ////
////  El tipo devuelto por la funcion inport depende del modo de trabajo:             ////
////  MODO 1: int                                                                     ////
////  MODO 2: int16                                                                   ////
////  La funcion outport usa el LSB del parametro pasado en caso de MODO 1            ////
////                                                                                  ////
//////////////////////////////////////////////////////////////////////////////////////////

int16 complement(int16 value)
{
   int i;
   for(i = 0; i < 16; i++){
      if(bit_test(value, i))
         bit_clear(value, i);
      else
         bit_set(value, i);
   }
   return value;
}

#ifndef use_16bit_port
   #define inport_lenght 8
   #define set_tris_inport() set_tris_b(0xFF)
   #define set_tris_outport() set_tris_d(0)
   #ifndef use_low_in 
   #define inport() input_b()
   #else
   #define inport() make8(complement(make16(0, input_b())), 0)
   #endif
   #ifndef use_low_out 
   #define outport(x) output_d(make8(x), 0)
   #else
   #define outport(x) output_d(make8(complement(x), 0))
   #endif
#else
   #define inport_lenght 16
   #define set_tris_inport()\
   {\
      set_tris_a(0xFF);\
      set_tris_b(0xFF);\
   }
   #define set_tris_outport()\
   {\
      set_tris_d(0);\
      set_tris_e(0);\
   }
   #ifndef use_low_in 
   #define inport() make16(input_a(), input_b())
   #else
   #define inport() complement(make16(input_a(), input_b()))
   #endif
   #ifndef use_low_in 
   #define outport(x)\
   {\
      output_d(make8(x, 0));\
      output_e(make8(x, 1));\
   }
   #else
   #define outport(x)\
   {\
      output_d(make8(complement(x), 0));\
      output_e(make8(complement(x), 1));\
   }
   #endif
#endif



void in_out_init()
{
   port_b_pullups(TRUE);
   setup_adc_ports(NO_ANALOGS);
   set_tris_inport();
   set_tris_outport();
   #ifdef use_low_out
      outport(0xFFFF);
   #else
      outport(0);
   #endif
}


