#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

const int num_hebras = 3;
//Se usan 3 semáforos (uno por cada identidad)
Semaphore ingr_disp[3]={0,0,0};
//Semaforo estanquero
Semaphore mostr_vacio=1;
mutex fmr,pro,fum;


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

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

int Producir(){

	return -1+aleatorio<1,3>(); 
}

void funcion_hebra_estanquero(  )
{

  int i;

    while(true){

    //Producir ingrediente
    	i=Producir();
    //cout<<"Ingrediente producido: "<<i<<endl;
      	sem_wait( mostr_vacio );
      	pro.lock();
      	cout<<"puesto ingr.: "<<i<<endl;
      	pro.unlock();
      	sem_signal ( ingr_disp[i]);
    }
	
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
    fmr.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    fmr.unlock(); 
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
    fum.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
    fum.unlock();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{	

  int b;

   while( true )
   {
    sem_wait( ingr_disp[num_fumador]);
    fum.lock();
    cout<<"retirado ingr.: "<<num_fumador<<endl;
    fum.unlock();
    b=num_fumador;
    sem_signal( mostr_vacio);
    fumar(num_fumador);
   }
   
}

//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
      << "Problema de los fumadores" << endl
      << "--------------------------------------------------------" << endl
      << flush ;

  thread hebra_estanquero ( funcion_hebra_estanquero );
  thread hebra_fumador[3];

  for (int i = 0; i < num_hebras; ++i)
  {
    hebra_fumador[i] = thread( funcion_hebra_fumador, i );
  }

  hebra_estanquero.join();

  for (int i = 0; i < num_hebras; ++i)
  {
    hebra_fumador[i].join();
  }
}
