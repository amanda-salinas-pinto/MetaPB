#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <vector>
#include <climits>
#include <time.h>

//COMPILE: gcc static_features.cpp -lstdc++ -std=c++11 -o build/static_features

using namespace std;

int main(int argc, char *argv[]) {
	string inst_name(argv[1]);
	ifstream inst_file(inst_name);
	
    string line;
	string name;
	int r,v;
	double r_u=0,r_b=0,r_t=0,r_q=0;
	double t_fo=0,t_res=0,t_pos_r=0,t_pos_fo=0;

	name = inst_name.substr(inst_name.find_last_of("/\\")+1);
    name = name.substr(0,name.find_last_of("."));

	// line1: * #variable= 8904 #constraint= 823
	string line1;
	getline(inst_file,line1);
	string aux="";
	int pos = line1.find("="); 
	for(pos=pos+1;pos<line1.size();pos++) {
		if(line1[pos]=='#') break;
		if(line1[pos]==' ') continue;
		aux+=line1[pos];
	}
	v=stoi(aux);
	aux="";
	pos = line1.find("=", pos+1); 
	for(pos=pos+1;pos<line1.size();pos++) {
		if(line1[pos]=='#') break;
		if(line1[pos]==' ') continue;
		aux+=line1[pos];
	}
	r=stoi(aux);

	while(getline(inst_file,line)) {
		if(line[0]=='*') continue;

		// obj function
		else if(line.substr(0,3)=="min") {
			for(int i=3; i<line.length(); i++) {
				if(line[i]=='+') {
					t_fo++;
					t_pos_fo++;
				}
				else if(line[i]=='-') {
					t_fo++;
				}
			}
		}

		// Constraints
		else {
			int t=0;
			for(int i=0; i<line.length(); i++) {
				if(line[i]=='+') { 
					t++;
					t_res++;
					t_pos_r++;
				}
				else if(line[i]=='-') {
					t++;
					t_res++;
				}
				else if(line[i]=='>' || line[i]=='=' || line[i]=='<') {
				    break;
				}
			}
			if(t==1) r_u++;
			else if(t>1) {
				if(t==2) r_b++;
				else if(t==3) r_t++;
				else if(t>=4) r_q++;
			}
		}

	}

	int t_tot = t_fo+t_res; 
	t_pos_fo = (t_fo>0) ? t_pos_fo/t_fo : 0;
	t_pos_r = (t_res>0) ? t_pos_r/t_res : 0;
	t_fo = (t_tot>0) ? t_fo/t_tot : 0;
	t_res = (t_tot>0) ? t_res/t_tot : 0;
	r_u = (r>0) ? r_u/r : 0;
	r_b = (r>0) ? r_b/r : 0;
	r_t = (r>0) ? r_t/r : 0;
	r_q = (r>0) ? r_q/r : 0;	

	cout << r << " " << v << " " << r_u << " " << r_b << " " << r_t << " " << r_q << " " << t_fo << " " << t_res << " " << t_pos_r << " " << t_pos_fo << endl;
	inst_file.close();

	return 0;
}