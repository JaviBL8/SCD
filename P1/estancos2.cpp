#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <atomic>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

const int num_hebras = 6;
//Se usan 3 semáforos (uno por cada identidad)
Semaphore ingr_disp[6]={0,0,0,0,0,0};
//Semaforo estanquero
Semaphore mostr_vacio=1;
mutex fmr,pro,pro2,fum;
Semaphore fumando=0;
int estafumando=0;

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

int Producir1(){

	return -1+aleatorio<1,3>(); 
}

int Producir2(){

	return -1+aleatorio<4,6>();
}
//Estanco 
void funcion_hebra_estanquero(  )
{
   
	  int i,b;

	    while(true){
	
	    b=rand()%1;
	    //cout<<"Ingrediente producido: "<<i<<endl;
	      	sem_wait( mostr_vacio );
		if(b==1){
			i=Producir2();
			sem_signal(fumando);
			if(estafumando==1){
	        	sem_wait(fumando);
			}
			else{
		      	   pro.lock();
		      	   cout<<"puesto ingr.: "<<i<<endl;
		      	   pro.unlock();
			} 
		}
		else{
			i=Producir1();
			pro2.lock();
		      	cout<<"puesto ingr.: "<<i<<endl;
		      	pro2.unlock();		
		
		}
	      	sem_signal ( ingr_disp[i]);
	    }
	
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{
   if(num_fumador>=3){
   	estafumando=1;   
   }

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
   
   if(num_fumador>=3){
   	estafumando=0;
	sem_signal( fumando);   
   }
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
    b=num_fumador;
    fum.unlock();
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

  thread hebra_estanquero[2];
  thread hebra_fumador[num_hebras];
	
  for (int i = 0; i < 2; ++i)
  {
    hebra_estanquero[i] = thread( funcion_hebra_estanquero );
  }  

  for (int i = 0; i < num_hebras; ++i)
  {
    hebra_fumador[i] = thread( funcion_hebra_fumador, i );
  }
  
  for (int i = 0; i < 2; ++i)
  {
    hebra_estanquero[i].join();
  }	
  

  for (int i = 0; i < num_hebras; ++i)
  {
    hebra_fumador[i].join();
  }
}
