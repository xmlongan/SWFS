#include <iostream>
#include "Fab.h"
#include "margin_alloc.h"

using namespace std;
void print_result(const Fab& fab);

int main()
{
	chrono::steady_clock::time_point begin = chrono::steady_clock::now();

	//usint machineCounts_exist[12] = { 4,3,10,1,1,2,1,4,1,9,2,2 };
	//Fab fab_exist = Fab(machineCounts_exist);

	//vector<float> samples{};
	//for (size_t i = 0; i < 200; i++)
	//{
	//	fab_exist.simulation();
	//	samples.push_back(fab_exist.t);
	//	fab_exist.reset();
	//}
	//ofstream fout;
	//fout.open("200-run-exist-fab.csv");
	//for (size_t i = 0; i < 200; i++)
	//{
	//	fout << samples[i] << endl;
	//}
	//fout.close();

	//fab_exist.simulation();
	//print_result(fab_exist);
	//mass_run_margin_one(fab_exist, 4, 200u, 1u);
	//mass_run_margin_two(fab_exist, 4, 200u, 1u);
	
	usint machineCounts_new[12] = { 1,1,1,1,1,1,1,1,1,1,1,1 };
	Fab fab_new = Fab(machineCounts_new);
	mass_run_margin_one(fab_new, 28, 200u, 1u);
	//mass_run_margin_two(fab_new, 28, 200u, 1u);

	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	cout << "总耗时 = " << chrono::duration_cast<chrono::minutes>(end - begin).count();
	cout << "[分钟]" << endl;
}

void print_result(const Fab& fab) {
	cout << "target: " << fab.target_lots << " lots;" << endl;
	cout << "produced: " << fab.produced_lots << " lots;" << endl;
	cout << "t: " << fab.t << " units" << endl;
	cout << "mcumsums: ";
	for (usint i = 0; i < 11; i++)
	{
		cout << fab.mcumsums[i] << ",";
	}
	cout << fab.mcumsums[11] << "; " << endl;
	cout << "buffers[60]: ";
	for (usint i = 0; i < 60-1; i++)
	{
		cout << fab.buffers[i] << ",";
	}
	cout << fab.buffers[59] << "; " << endl;
	cout << "machine states: " << endl;
	for (usint i = 0; i < fab.mcumsums[11]-1; i++)
	{
		cout << fab.machines[i] << ",";
	}
	cout << fab.machines[fab.machines.size()-1] << "; " << endl;
	cout << "SEL: ";
	for (usint i = 0; i < fab.SEL.size()-1; i++)
	{
		cout << fab.SEL[i] << ",";
	}
	cout << fab.SEL[fab.SEL.size()-1] << "; " << endl;
}