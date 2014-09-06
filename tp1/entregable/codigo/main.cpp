#include <vector>
#include <queue>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include "simu.h"
#include "basetask.h"
#include "basesched.h"
#include "tasks.h"

#include "sched_fcfs.h"
#include "sched_rr.h"
#include "sched_rr2.h"
#include "sched_lottery.h"

using namespace std;

ostream& operator<<(ostream &os, const Settings &s)
{
	os << "tasks_file:     " << s.tasks_file << endl
	   << "num_cores:      " << s.num_cores << endl
	   << "switch_cost:    " << s.switch_cost << endl
       << "migrate_cost:   " << s.migrate_cost << endl
	   << "sched_class:    " << s.sched_class << endl
	   << "sched_args:     ";

	for(vector<int>::const_iterator it = s.sched_args.begin();
	    it != s.sched_args.end(); ++it)  os << *it << " ";
	os << "(" << s.sched_args.size() << " ints)" << endl;

	os << "verbose:        " << (s.verbose? "yes" : "no") << endl
	   << "output_log:     " << s.output_log << endl;

	return os;
}

string one_line_summary(const Settings &s)
{
	ostringstream os;
	os << s.tasks_file << " " << s.num_cores << " " << s.switch_cost << " " << s.migrate_cost << " " << s.sched_class;
	vector<int>::const_iterator it = s.sched_args.begin();
	++it; //Saco el primer parametro que es un agregado para que muestre los cores.
	while(it != s.sched_args.end()) os << " " << *it++;
	return os.str();
}


const char *USAGE =
" [-h] [-v] [-o output] tasks_file num_cores switch_cost migrate_cost sched_class args\n"
"\n"
"         tasks_file    define el lote de tareas\n"
"         num_cores     define la cantidad de cores\n"
"         switch_cost   en ticks completos por c/cambio de contexto\n"
"         migrate_cost  en ticks completos por c/cambio de cpu\n"
"         sched_class   nombre de la subclase de SchedBase deseada\n"
"         args          argumentos enteros para pasarle al scheduler\n"
"                       (ver detalles en constructor de sched_class)\n"
"\n"
"         -v    mayor nivel de verborragia\n"
"         -o    nombre base para archivos generados\n"
"         -h    mostrar este texto de ayuda y salir\n"
"\n"
"ejs:  simusched lote.tsk 1 10 2 SchedFCFS\n"
"         donde 1 es la cantidad de núcleos\n"
"         donde 10 es el costo de cambio de contexto\n"
"         donde 2 es el costo de cambio de cpu\n"
"         (el algoritmo FCFS no recibe argumentos)\n"
"\n"
"      simusched -v -o probando lote15.tsk 1 2 3 SchedRR 8\n"
"         donde 1 es la cantidad de núcleos\n"
"         donde 2 es el costo de cambio de contexto\n"
"         donde 3 es el costo de cambio de cpu\n"
"         y 8 es el quantum para el algoritmo RR\n"
"\n"
"      simusched foo.tsk 1 1 2 SchedFCFS | python graphsched.py | png_viewer\n"
"      simusched foo.tsk 2 1 2 SchedFCFS | python graphsched.py > foo.png\n"
"         para graficar (ver script .py para más detalles)\n"
;


bool file_readable(const string pathname)
{
	// Feucho pero bien portable:
	ifstream tf(pathname.c_str());
	if(!tf) return false;
	tf.close();
	return true;
}


int cmdline_parse(int argc, char* argv[], Settings &s)
{
	string prog_name(argv[0]);
	int i = 1;  // #args seen

	/* Opciones y flags */

	s.verbose = false;
	s.output_log = "-";

	while(i < argc && argv[i][0] == '-') {
		string optn(argv[i] + 1);
		if(optn == "h") {
			cerr << "uso: " << prog_name << USAGE << endl;
			return 1;
		} else if(optn == "v") {
			s.verbose = true;
		} else if(optn == "o") {
			if(++i < argc && argv[i][0] != '-') {
				s.output_log = argv[i];
			} else {
				cerr << "error: uso ilegal de -o" << endl;
				cerr << "uso: " << prog_name << USAGE << endl;
				return 2;
			}
		}
		++i;
	}

	/* Argumentos posicionales */

	if(argc - i < 3) {
		cerr << "error: argumentos insuficientes" << endl;
		cerr << "uso: " << prog_name << USAGE << endl;
		return 3;
	}

	s.tasks_file = argv[i++];
	if(!file_readable(s.tasks_file)) {
		cerr << "error: no se pudo leer: " << s.tasks_file << endl;
		return 4;
	}

	char *cptr;
	s.num_cores = static_cast<unsigned int>(strtol(argv[i++], &cptr, 10));
	if(*cptr != '\0') {
		cerr << "error: no es un natural: " << argv[i-1] << endl;
		return 5;
	}

	char *eptr;
	s.switch_cost = static_cast<unsigned int>(strtol(argv[i++], &eptr, 10));
	if(*eptr != '\0') {
		cerr << "error: no es un natural: " << argv[i-1] << endl;
		return 6;
	}

	s.migrate_cost = static_cast<unsigned int>(strtol(argv[i++], &eptr, 10));
	if(*eptr != '\0') {
		cerr << "error: no es un natural: " << argv[i-1] << endl;
		return 7;
	}

	s.sched_class = argv[i++];
	if(s.sched_class[0] != 'S') { // TODO: mejor error checking
		cerr << "error: scheduler desconocido: " << s.sched_class << endl;
		return 8;
	}

	s.sched_args.clear();
	//Agrego la cantidad de cores como primer parametros de los argumentos.
	s.sched_args.push_back(s.num_cores);
	while(i < argc) {
		int argint = static_cast<int>(strtol(argv[i++], &eptr, 10));
		if(*eptr != '\0') {
			cerr << "error: no es un entero: " << argv[i-1] << endl;
			return 9;
		} else s.sched_args.push_back(argint);
	}

	return 0;
}

SchedBase* sched_create(const char* sched, vector<int> argn) {
	#define _sched_create(tipo, prms) if (!strcmp(#tipo, sched)) { if (!(prms == -1 || (int)(argn.size()) == prms)) { cerr << "error: "#tipo" recibe " << prms << " parámetro(s)." << endl; return NULL; } return new tipo(argn); }
	/* Agregue aquí los schedulers nuevos que cree agregando una línea con:
	 *   _sched_create(SchedX, n)
	 * donde "SchedX" es la nueva clase implementada y n es la cantidad de
	 * parámetros que recibe su scheduler como un vector de enteros (vector<int>)
	 * o ponga -1 para una cantidad de parámetros arbitraria. */
	_sched_create(SchedRR, -1)
	_sched_create(SchedRR2, -1)
	_sched_create(SchedFCFS, -1)
	_sched_create(SchedLottery, -1)
	return NULL;
}

int main(int argc, char* argv[]) {

	Settings settings;
	int rc = cmdline_parse(argc, argv, settings);
	if(rc != 0) return rc;

	//Obtengo el scheduler a usar.
	SchedBase *scheduler = sched_create(settings.sched_class.c_str(), settings.sched_args);
	if(!scheduler) {
		cerr << "error: scheduler desconocido: " << settings.sched_class;
		for(int j=0; j<(int)settings.sched_args.size(); j++) cerr << (j?',':'(') << settings.sched_args[j];
		if (!settings.sched_args.size()) cerr << "(";
		cerr << ")" << endl;
		return 3;
	}

	if (settings.verbose) {
		cerr << endl << settings;
	}
	cout << "# SETTINGS " << argv[0] << " " << one_line_summary(settings) << endl;

	//Registro los tipos de tareas.
	tasks_init();
	//Cargo las tareas definidas por el usuario.
	vector<ptsk> tasks = tasks_load(settings.tasks_file.c_str());

	simulate(*scheduler, tasks, settings);

	delete scheduler;

	return rc;

}
