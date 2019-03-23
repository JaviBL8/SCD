#include "HoareMonitor.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <mutex>

using namespace std;
using namespace HM;

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

void EsperarFueraBarberia(int i){
	cout << "\nEl cliente " <<i<< " sale de la barberia" << flush;
    const int ms = aleatorio<0,300>(); // duración aleatoria
    this_thread::sleep_for( chrono::milliseconds(ms));
}
 
void CortarPeloACliente(){
    const int ms = aleatorio<0,300>(); // duración aleatoria
    this_thread::sleep_for( chrono::milliseconds(ms));
    cout << "\nEl barbero empieza a pelar al cliente" << flush;
}

class Barberia : public HoareMonitor{

private:
	CondVar silla;
	CondVar sala;
	CondVar barbero;
public:
	
	Barberia(){
		silla=newCondVar();
		sala=newCondVar();
		barbero=newCondVar();
	}

	void cortarPelo(int i){

		if (  !silla.empty()){
 
            sala.wait();
        }

        barbero.signal();
        cout<<"\nEl cliente "<<i<<" se sienta en la silla"<<flush; 
        silla.wait();
	}

	void siguienteCliente(){

		if ( silla.empty() && sala.empty() ){
            cout << "\nEl barbero esta durmiendo" << flush;
            barbero.wait();
        }
 	
 		cout << "\nEl barbero llama a otro cliente de la sala de espera" << flush;
        sala.signal();
    }
 
   
	void finCliente(){

		cout<<"\nEl barbero termina de pelar al cliente"<<flush;
		silla.signal();
	}
};

void funcion_hebra_barbero(MRef <Barberia> monitor){
	while ( true ){

	    monitor->siguienteCliente();
	    CortarPeloACliente();
	    monitor->finCliente();
	}
}
  
void funcion_hebra_cliente(MRef <Barberia> monitor, int n){
    while ( true ){

        monitor->cortarPelo(n);
        EsperarFueraBarberia(n);
	}
}

int main()
{
	const int n_clientes=3;

	auto monitor = Create<Barberia>();
	
	thread Barbero(funcion_hebra_barbero, monitor);
	thread Clientes[n_clientes];

	for (int i = 0; i < n_clientes; ++i)
	{
		Clientes[i]=thread(funcion_hebra_cliente,monitor,i);
	}

	Barbero.join();

	for (int i = 0; i < n_clientes; ++i)
	{
		Clientes[i].join();
	}

	return 0;
}