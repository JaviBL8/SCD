// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario
// (versión con un único productor y un único consumidor)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------
//
//
////////////////////////////////////////////////////////////////
//
//  
//  PROBLEMA: PRODUCTOR Y CONSUMIDOR MULTIPLE
//
///////////////////////////////////////////////////////////////



#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int

   id_productor          = 0 ,
   id_buffer             = 4 ,
   id_consumidor         = 5 ,
   
   num_consumidores = 5,
   num_productores = 4,
   
   etiq_buffer_consumidor = 20,
   etiq_productor = 6,
   etiq_consumidor = 7,
   etiq_buffer = 8,
   
   num_procesos_esperado = 10 ,
   num_items             = 20,
   tam_vector            = 10;

  int  contadores[num_productores];

//Colores para facilitar la comprensión de la salida por terminal
	#define BLACK    "\033[0m"
	#define RED      "\033[31m"
	#define GREEN    "\033[32m"
	#define YELLOW   "\033[33m"
	#define BLUE     "\033[34m"
	#define MAGENTA  "\033[35m"
	#define CYAN     "\033[36m"
	#define WHITE    "\033[37m"
  

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}
// ---------------------------------------------------------------------

int producir(int id)
{
  int contador;
  
   sleep_for( milliseconds( aleatorio<10,100>()) );

   contador=contadores[id];
   contadores[id]++;

   cout << BLUE<< "Productor ha producido valor " << contador << endl<< BLACK  << flush;
   return contador ;
}
// ---------------------------------------------------------------------

void funcion_productor(int id)
{
   for ( unsigned int i= 0 ; i < (num_items/num_productores) ; i++ )
   {
      // producir valor
      int valor_prod = producir(id);
      // enviar valor
      cout <<CYAN<< "Productor " << id << " va a enviar valor " << valor_prod << endl << flush;
      MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_productor, MPI_COMM_WORLD );

   }
}
// ---------------------------------------------------------------------

void consumir( int valor_cons )
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout <<MAGENTA << "Consumidor ha consumido valor " << valor_cons << endl<< BLACK << flush ;
}
// ---------------------------------------------------------------------

void funcion_consumidor(int id)
{
   int         peticion = 1,
               valor_rec ;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < (num_items/num_consumidores); i++ )
   {

      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, etiq_consumidor, MPI_COMM_WORLD);

      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, etiq_buffer_consumidor, MPI_COMM_WORLD,&estado );
      cout << RED << "Consumidor ha recibido valor " << valor_rec << endl<< BLACK << flush ;

      consumir( valor_rec );
   }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              num_celdas_ocupadas = 0,
              peticion,
              rama; // número de celdas ocupadas
              MPI_Status estado ;                 // metadatos del mensaje recibido

    for( unsigned int i=0 ; i < num_items*2 ; i++ )
    {

      if ( num_celdas_ocupadas == 0){
        rama = 0;
      }
      else if ( num_celdas_ocupadas == tam_vector){
        rama = 1;
      }
      else{

         MPI_Probe (MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado );
 
          if ( estado.MPI_TAG == etiq_productor ){
            rama = 0;
          }
          else{
            rama = 1;
          }
      }

      switch( rama ) // leer emisor del mensaje en metadatos
      {
         case 0: // si ha sido el productor: insertar en buffer
            MPI_Recv( &buffer[num_celdas_ocupadas], 1, MPI_INT, MPI_ANY_SOURCE, etiq_productor, MPI_COMM_WORLD, &estado );
            cout << "Buffer ha recibido valor " << buffer[num_celdas_ocupadas] << endl ;
            num_celdas_ocupadas++ ;
            break;

         case 1: // si ha sido el consumidor: extraer y enviarle
            MPI_Recv( &peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiq_consumidor, MPI_COMM_WORLD, &estado );

            MPI_Ssend( &buffer[num_celdas_ocupadas-1], 1, MPI_INT, estado.MPI_SOURCE, etiq_buffer_consumidor, MPI_COMM_WORLD);
            cout << "Buffer va a enviar valor " << buffer[num_celdas_ocupadas-1] << endl ;
            num_celdas_ocupadas-- ;
            break;
      }
   }
}


// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;

       //Inicializacion contadores
    for (int i = 0; i < num_productores; ++i)
    {
      contadores[i]=0;
    }	

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual ){
      // ejecutar la operación apropiada a 'id_propio'
      if ( id_propio < id_buffer ){

         funcion_productor(id_propio);
      }
      else if ( id_propio == id_buffer ){

         funcion_buffer();

      }
      else{

         funcion_consumidor(id_propio);
      
      }    
   }
   else{
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
