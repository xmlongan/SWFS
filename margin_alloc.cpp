#include "margin_alloc.h"

using namespace std;

vector<iandj> comb_ij() {
	vector<iandj> comb{};
	for (usint i = 0; i < 12; i++)
	{
		for (usint j = i; j < 12; j++)
		{
			comb.push_back(make_pair(i, j));
		}
	}
	return comb;
}

usint_float add_one(Fab& fab, const usint& rep) {
	v_float samples{};
	usint n;
	if (rep != 1)
	{
		cerr << "Please rewrite function add_one to acommodate more than 1 time rep!" << endl;
	}
	// replicate rep =1 times, then pick the smallest
	for (usint sNo = 1; sNo < 13; sNo++)
	{
		fab.add_machine(sNo);//sNo
		fab.reset();
		fab.simulation();
		samples.push_back(fab.t);
		fab.del_machine(sNo);
	}
	n = min_element(samples.begin(), samples.end()) - samples.begin();
	fab.add_machine(n + 1);
	return make_pair(n + 1, samples[n]);
}

iandj_float add_two(Fab& fab, const usint& rep) {
	vector<float> samples{};
	usint n;
	vector<iandj> ijs = comb_ij();
	if (rep != 1)
		cerr << "Please rewrite function add_two to acommodate more than 1 time rep!" << endl;
	for (usint i = 0; i < ijs.size(); i++)
	{
		fab.add_machine(ijs[i].first + 1);
		fab.add_machine(ijs[i].second + 1);
		fab.reset();
		fab.simulation();
		samples.push_back(fab.t);
		fab.del_machine(ijs[i].second + 1);
		fab.del_machine(ijs[i].first + 1);
	}
	n = min_element(samples.begin(), samples.end()) - samples.begin();
	fab.add_machine(ijs[n].first + 1);
	fab.add_machine(ijs[n].second + 1);
	return make_pair(ijs[n], samples[n]);
}

vector<usint_float> greedy_search(const usint& Nmore, Fab& fab, const usint& rep) {
	vector<usint_float> records{};
	for (usint i = 0; i < Nmore; i++)
	{
		records.push_back(add_one(fab, rep));//(stationNo,t)
	}
	return records;
}

vector<iandj_float> greedy_search2(const usint& Nmore, Fab& fab, const usint& rep) {
	vector<iandj_float> records{};
	usint K = Nmore / 2;
	usint left = Nmore % 2;
	for (usint i = 0; i < K; i++)
	{
		records.push_back(add_two(fab, rep));// ((sNo,sNo),t)
	}
	if (left == 1)
	{
		pair<usint, float> result1 = add_one(fab, rep);
		//ij =(sNo, nullstationNo), fab.stations.size() + 1 = nullstationNo
		iandj ij = make_pair(get<0>(result1), 13);
		records.push_back(make_pair(ij, get<1>(result1)));//((sNo,sNo),t)
	}
	return records;
}

void mass_run_margin_one(Fab& fab, usint N_more, usint Rep, usint rep) {
	vector<vector<usint_float>> records{};//vector<pair<usint, float>> result{};
	ofstream fout;
	string fname = "margin-one-" + to_string(N_more) + "-more-" + to_string(Rep) + "-rep.csv";
	fout.open(fname, ios_base::app);
	//title
	for (usint n = 0; n < N_more; n++)
	{
		fout << "stationNo,production time" << ((n == N_more - 1) ? "" : ",");
	}
	fout << endl;
	//generate and record
	for (usint i = 0; i < Rep; i++)
	{
		cout << i + 1 << "th greedy_searching allocating " << N_more << " more machines!" << endl;
		chrono::steady_clock::time_point begin = chrono::steady_clock::now();
		Fab fab_new = fab;
		records.push_back(greedy_search(N_more, fab_new, rep));
		chrono::steady_clock::time_point end = chrono::steady_clock::now();
		cout << "ºÄÊ± = " << chrono::duration_cast<chrono::seconds>(end - begin).count();
		cout << "[Ãë]" << endl;
		//
		for (usint j = 0; j < records[i].size(); j++)
		{
			fout << records[i][j].first << "," << records[i][j].second;
			fout << ((j== (records[i].size()-1))? "" : ",");
		}
		fout << endl;
	}
	fout.close();
	cout << fname << " finished appending!" << endl;
}

void mass_run_margin_two(Fab& fab, usint N_more, usint Rep, usint rep) {
	vector<vector<iandj_float>> records{};//vector<pair<iandj, float>> result{};
	ofstream fout;
	string fname = "margin-two-" + to_string(N_more) + "-more-" + to_string(Rep) + "-rep.csv";
	fout.open(fname, ios_base::app);
	//title
	for (usint n = 0; n < (N_more/2); n++)
	{
		fout << "stationNo,stationNo,production time" << ((n == (N_more / 2 - 1)) ? "" : ",");
	}
	fout << endl;
	//generate and record
	for (usint i = 0; i < Rep; i++)
	{
		cout << i + 1 << "th greedy_searching allocating " << N_more << " more machines!" << endl;
		chrono::steady_clock::time_point begin = chrono::steady_clock::now();
		Fab fab_new = fab;
		records.push_back(greedy_search2(N_more, fab_new, rep));
		chrono::steady_clock::time_point end = chrono::steady_clock::now();
		cout << "ºÄÊ± = " << chrono::duration_cast<chrono::seconds>(end - begin).count();
		cout << "[Ãë]" << endl;
		//
		for (usint j = 0; j < records[i].size(); j++)
		{
			fout << records[i][j].first.first << "," << records[i][j].first.second << ",";
			fout << records[i][j].second << ((j == (records[i].size()-1)) ? "" : ",");
		}
		fout << endl;
	}
	fout.close();
	cout << fname << " finished appending!" << endl;
}
