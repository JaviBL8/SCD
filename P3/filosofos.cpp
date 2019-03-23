// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------
//mpicxx -std=c++11 filosofos.cpp -o filosofos
//mpirun -np 10 prodcons


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos  = 2*num_filosofos,
   et_coger=0,
   et_soltar=1 ;

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

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_procesos, //id. tenedor izq.
      id_ten_der = (id+num_procesos-1) % num_procesos; //id. tenedor der.

  while ( true )
  {
  	if(id==0){
	    cout << GREEN << "Filósofo " <<id <<" solicita ten. der." << id_ten_der << BLACK << endl;
	    // ... solicitar tenedor derecho (completar)
	    MPI_Ssend(NULL, 0, MPI_INT, id_ten_der, et_coger, MPI_COMM_WORLD);
       
    	cout << RED << "Filósofo " <<id << " solicita ten. izq." << id_ten_izq << BLACK << endl;
	    // ... solicitar tenedor izquierdo (completar)
	    MPI_Ssend(NULL, 0, MPI_INT, id_ten_izq, et_coger, MPI_COMM_WORLD);
    }
  	else{
  		cout << RED << "Filósofo " <<id << " solicita ten. izq." << id_ten_izq << endl;
	    // ... solicitar tenedor izquierdo (completar)
	    MPI_Ssend(NULL, 0, MPI_INT, id_ten_izq, et_coger, MPI_COMM_WORLD);

	    cout << GREEN<<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der<<BLACK <<endl;
	    // ... solicitar tenedor derecho (completar)
	    MPI_Ssend(NULL, 0, MPI_INT, id_ten_der, et_coger, MPI_COMM_WORLD);
  	}
    cout <<"Filósofo " <<id <<" comienza a comer"<<BLACK <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<RED <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<BLACK <<endl;
    // ... soltar el tenedor izquierdo (completar)
    MPI_Ssend(NULL, 0, MPI_INT, id_ten_izq, et_soltar, MPI_COMM_WORLD);

    cout<< GREEN<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der<<BLACK <<endl;
    // ... soltar el tenedor derecho (completar)
    MPI_Ssend(NULL, 0, MPI_INT, id_ten_der, et_soltar, MPI_COMM_WORLD);

    cout << "Filosofo " << id << " comienza a pensar" << BLACK<< endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones
  int buf;

  while ( true )
  {
     // ...... recibir petición de cualquier filósofo (completar)
  	 MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, et_coger, MPI_COMM_WORLD, &estado);
     
     // ...... guardar en 'id_filosofo' el id. del emisor (completar)
     id_filosofo=estado.MPI_SOURCE;

     cout<< BLUE <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo<<BLACK <<endl;

     // ...... recibir liberación de filósofo 'id_filosofo' (completar)
     MPI_Recv(&id_filosofo, 1, MPI_INT, id_filosofo, et_soltar, MPI_COMM_WORLD, &estado);
     cout <<BLUE <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo<<BLACK <<endl ;
  }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
