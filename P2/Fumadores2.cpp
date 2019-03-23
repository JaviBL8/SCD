//Compilar usando g++ -std=c++11 -pthread -g -o Fumadores2 Fumadores2.cpp HoareMonitor.cpp
/*Para solucionar este problema he declarado una variable atomica de timo int que tome como valor
el último fumador(i) y he añadido un if en la funcion "ObtenerIngrediente" que comprueba que último fumador
no es el mismo que se dispone a fumar ahora (no son consecutivos) después de ese if cambio el valor de
mostrador y de ultimo en fumar (los reseteo) y prosigue la ejecución hasta que vuelva a fumar el mismo*/ 
#include "HoareMonitor.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <mutex>
#include <atomic>

using namespace std;
using namespace HM;

mutex mtx;
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

void Fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  :"<< " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    mtx.unlock(); 
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
    mtx.unlock();
}

class Estanco : public HoareMonitor{
	private:
	
		int num_ing, num_fum, num_est, mostrador;//mostrador = {-1:vacio,0:papel,1:cerillas,2:tabaco}
		CondVar colaEst;
		CondVar colaFum[3];
		atomic<int> ult_fum;
	
	public:
		
		Estanco(){
			mostrador=-1;
			num_fum=3;
			colaEst=newCondVar();

			for (int i = 0; i < num_fum; ++i)
			{
				colaFum[i]=newCondVar();
			}
			ult_fum=-1;
		}

		void PonerIngrediente(int i){
			mostrador=i;
			mtx.lock();
			cout << "\nEl estanquero esta repartiendo el ingrediente " << i << flush<< endl ;
			mtx.unlock();
			colaFum[i].signal();
		}

		void ObtenerIngrediente(int i){

			if (mostrador!=i) 
			colaFum[i].wait();
			if (ult_fum==i){
			mtx.lock();			
			cout<<"Mismo fumador, se retiran todos los ingredientes\n";
			mtx.unlock();
			mostrador=-1;
			ult_fum=-1;
			colaEst.signal();
			colaFum[i].wait();
			}
			else{
			mtx.lock();
			cout << "\nEl fumador " << i << " puede fumar" << flush << endl;
			ult_fum=i;
			mtx.unlock();
			mostrador=-1;
			colaEst.signal();
			}
		}
		
		void EsperarRecogida(){
			if (mostrador!=-1) colaEst.wait();
		}
};

int ProducirIngrediente()
{
   return aleatorio<0,2>();
}

void Fumador(MRef<Estanco> monitor, int i){

	while(true){
		monitor->ObtenerIngrediente( i );
		Fumar(i);
	}
}

void Estanquero(MRef<Estanco> monitor){

int ingre;
	
	while(true){
		ingre = ProducirIngrediente();
		monitor->PonerIngrediente( ingre );
		monitor->EsperarRecogida();
	}
}

int main()
{
	const int n_i=3,
			  n_f=3,
			  n_e=1,
			  m=3;

    MRef<Estanco> monitor = Create<Estanco>();

   thread Estanqueros(Estanquero, monitor);
   thread Fumadores[n_f];
   

   for (int i = 0; i < n_f; ++i)
      Fumadores[i]=thread(Fumador, monitor, i);
      

   for (int i = 0; i < n_f ; ++i)
      Fumadores[i].join();

      Estanqueros.join();

   //test_contadores(n_i);
}

