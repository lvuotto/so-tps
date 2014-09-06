#include "basetask.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;

#define esta(e, X) ((X).find(e) != (X).end())
map<string, ptski> task_defs;
vector<TaskBase*> tasks;

vector<ptsk> tasks_load(const char* filename) {
	vector<ptsk> ts(0);
	ifstream f(filename);
	string s; int l = 0;
	unsigned int starttm = 0;
	unsigned int deadline = 0;
	while(getline(f, s)) { l++;

		//Salteo comentarios o líneas en blanco
		if (s == "" || s[0] == '#') continue;

		istringstream iss(s);

		//Ready time
		if (s[0] == '@') {
			char c; iss >> c;
			iss >> starttm;
			continue;
		}

		//Deadline time
		if (s[0] == '$') {
			char c; iss >> c;
			iss >> deadline;
			continue;
		}

		int times = 1;
		if (s[0] == '*') {
			char c; iss >> c;
			iss >> times;
			if (times<=0) { cerr << "WARNING: '*n' should have a positive number n." << endl; times = 1; }
		}
		string nom = "";
		vector<int> params; int x;
		if (!(iss >> nom) || !esta(nom, task_defs)) {
			cerr << filename << ":" << l << ": ERROR: Unknow task type (" << nom << "): " << s << endl;
			exit(1);
		}
		while (iss >> x) params.push_back(x);
		ptski ti = task_defs[nom];
		if (ti.second != -1 && ti.second != (int)params.size()) {
			cerr << filename << ":" << l << ": ERROR: expected " << ti.second << " parameters but " << params.size() << " found: " << s << endl;
			exit(1);
		}
		for(int j=0; j<times; j++) ts.push_back(ptsk(ti.first, params, starttm,deadline));
	}
	return ts;
}

ptsk::ptsk(TaskBase* vtsk, const std::vector<int>& vprms, unsigned int vstart, unsigned int vend) : tsk(vtsk), prms(vprms), start(vstart), end(vend) {}
ptsk::ptsk(void) {}
