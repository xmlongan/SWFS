#include "Fab.h"

using namespace std;

Fab::Fab(usint* machineCounts) {
	// machine cumsums in each station
	mcumsums[0] = machineCounts[0];
	for (usint i = 1; i < 12; i++)
	{
		mcumsums[i] = mcumsums[i - 1] + machineCounts[i];
	}
	// initialize machines. 0 (idle): default state
	machines.assign(mcumsums[11], 0);
	// initialize sis;
	sis.assign(mcumsums[11], 0);
	v_usint::iterator it = sis.begin();
	fill(it, it + mcumsums[0], 0);
	for (usint i = 1; i < 12; i++)
	{
		fill(it + mcumsums[i - 1], it + mcumsums[i], i);
	}
	SEL.assign(static_cast<size_t>(mcumsums[11])* 3, inf);
}

void Fab::initialize_state(const usint& targetLots, const float& start_time) {
	t = start_time;
	target_lots = targetLots;
	produced_lots = 0;
	float* ptr_SEL = &(SEL[0]);

	for (usint i = 0; i < mcumsums[0]; i++)
	{
		machines[i] = 1;
		*ptr_SEL = t + rexp(mpts[0]);// event 1
		ptr_SEL++;// point to i*3 + 1
		*ptr_SEL = t + rexp(mtbfs[0]);// event 2
		ptr_SEL++;// point to i*3 + 2
		ptr_SEL++;// point to i*3 + 3 = (i+1)*3 + 0
		//SEL[i * 3u + 0u] = t + rexp(mpts[0]);// event 1
		//SEL[i * 3u + 1u] = t + rexp(mtbfs[0]);// event 2
		////SEL[i * 3 + 2] = t + rexp(mttrs[0]);// event 3
	}
	buffers[0] = target_lots - mcumsums[0];
}

bool Fab::lessOb_moreIb(const usint& i, const usint& j) {
	// select the max ib with the least ob, excluding least ob with 0 ib
	// i: bNo, j: bNo
	if (buffers[i-1] == 0)
	{
		return false;
	}// then buffers[i-1] > 0
	if (buffers[j-1] == 0)
	{
		return true;
	}// then buffers[j-1] > 0
	if (buffers[i] == buffers[j])
	{
		return buffers[i - 1] > buffers[j - 1];
	}
	return buffers[i] < buffers[j];
}

usint Fab::maxIb_minOb(const usint& si) {
	v_usint::const_iterator maxmin = ibNos[si].begin();
	v_usint::const_iterator it = maxmin;
	++it;
	for (; it != ibNos[si].end(); ++it)
		if (lessOb_moreIb(*it, *maxmin))
			maxmin = it;
	return *maxmin;
}

void Fab::update_state(const size_t& ei) {
	size_t mi = ei / 3;
	size_t tpi = ei % 3;
	usint& si = sis[mi];
	int& machine = machines[mi];// machine state
	usint bNo{};

	if (tpi == 1)// a failure
	{
		//machine = j > 0, must be working on j step of some lot: 
		buffers[machine - 1]++;// return the processing lot back to the buffer
		machine = -1;// to state -1 (failure)
		SEL[ei - 1] = inf;// delete event 1
		SEL[ei + 1] = t + rexp(mttrs[si]);// add event 3
		return;
	}

	if (tpi == 2)// a repair-up
	{
		// pick a bNo
		bNo = (ibNos[si].size() == 1) ? ibNos[si][0] : maxIb_minOb(si);
		// works even for the last station
		if (buffers[bNo - 1] == 0)// no waiting lot
		{
			machine = 0;
		}
		else
		{
			machine = bNo;
			buffers[bNo - 1]--;
			SEL[ei - 2] = t + rexp(mpts[si]);// add event 1
			SEL[ei - 1] = t + rexp(mtbfs[si]);// add event 2
		}
		return;
	}

	//Now tp = 1, i.e., completion of current processing step
	//-----------------------------------------------------------
	//step is the last step
	//-----------------------------------------------------------
	if (machine == 60)
	{
		produced_lots++;
		if (buffers[machine - 1] == 0)// only one incoming buffer
		{
			machine = 0;
			SEL[ei + 1] = inf;//del event 2
		}
		else
		{
			buffers[machine - 1]--;
			SEL[ei] = t + rexp(mpts[si]);//a new event 1; keep event 2
		}
		return;
	}

	//-----------------------------------------------------------
	//step having next step
	//-----------------------------------------------------------
	usint nextsi = step_sNo[machine] - 1;// si = sNo - 1; nextsi: next station index
	usint& nextb = buffers[machine];
	nextb++;//increment buffer first
	//pick one possible idle machine in the next station
	usint start = (nextsi == 0) ? 0 : mcumsums[nextsi - 1];
	for (size_t i = start; i < mcumsums[nextsi]; i++)
	{
		if (machines[i] == 0)
		{
			machines[i] = machine + 1;// start to work j(machine + 1) step of a lot 
			nextb--;
			SEL[i * 3] = t + rexp(mpts[nextsi]);//add event 1
			SEL[i * 3 + 1] = t + rexp(mtbfs[nextsi]);//add event 2
			break;
		}
	}

	//update the current machine state
	//select bNo for current machine
	bNo = (ibNos[si].size() == 1) ? ibNos[si][0] : maxIb_minOb(si);
	if (buffers[bNo - 1] == 0)// no waiting lot
	{
		machine = 0;
		SEL[ei + 1] = inf;// delete event 2
	}
	else
	{
		machine = bNo;
		buffers[bNo - 1]--;
		SEL[ei] = t + rexp(mpts[si]);// a new event 1
	}
	return;
}

void Fab::simulation() {
	initialize_state(1000, 0.0f);
	size_t ei;
	while (produced_lots < target_lots)
	{
		ei = min_element(SEL.begin(), SEL.end()) - SEL.begin();
		t = SEL[ei];
		SEL[ei] = inf;// event in ei happens, remove from SEL
		update_state(ei);
	}
	return;
}


void Fab::reset() {
	//reset time t and produced lots
	t = 0;
	produced_lots = 0;
	//reset buffers
	fill(buffers, buffers + 60, 0);
	//reset machines. 0 (idle): default machine state
	machines.assign(mcumsums[11], 0);
	// reset station index for each machine
	sis.assign(mcumsums[11], 0);
	v_usint::iterator it = sis.begin();
	fill(it, it + mcumsums[0], 0);
	for (usint i = 1; i < 12; i++)
	{
		fill(it + mcumsums[i - 1], it + mcumsums[i], i);
	}
	// reset SEL
	SEL.assign(static_cast<size_t>(mcumsums[11]) * 3, inf);
	return;
}

void Fab::add_machine(const usint& sNo) {
	for (usint i = sNo-1; i < 12; i++)
	{
		mcumsums[i]++;
	}
	machines.push_back(0);//0 (idle): machine default state
}

void Fab::del_machine(const usint& sNo) {
	for (usint i = sNo-1; i < 12; i++)
	{
		mcumsums[i]--;
	}
	machines.pop_back();
}

float Fab::rexp(const float& mean) {
	random_device rd;
	mt19937 gen(rd());
	exponential_distribution<float> expd(1.0f / mean);
	return expd(gen);
}


//void Fab::update_state_and_record(const size_t& ei) {
//	size_t mi = ei / 3;
//	size_t tpi = ei % 3;
//	usint& si = sis[mi];
//	//machine: -1: failed; 0: idle; j: working on the j step of some lot;
//	int& machine = machines[mi];// machine state
//	stringstream msg{}, msgb{}, msgm{}, msgiob{};
//
//	if (tpi == 1)// a failure
//	{
//		//machine = j > 0, must be working on j step of some lot: 
//		msg << "<h4>A failure: return processing lot back to buffer</h4>" << endl;
//		msg << "<p>buffer " << machine << " : " << buffers[machine - 1];
//		buffers[machine - 1]++;// return the processing lot back to the buffer
//		msg << " -> " << buffers[machine - 1] << ", ";
//		//
//		msgb << html_buffers(machine);
//		//
//		msg << "machine " << mi + 1 << " : " << machine;
//		machine = -1;// change machine to state -1
//		msg << " -> " << machine << ". </p>" << endl;
//
//		SEL[ei - 1] = inf;// delete event 1
//		SEL[ei + 1] = t + rexp(mttrs[si]);// add event 3
//		//
//		msgm << html_machines_SEL(ei + 1, mi + 1);
//		//
//		message += msg.str() + msgb.str() + msgm.str();
//		return;
//	}
//
//	if (tpi == 2)// a repair-up
//	{
//		// works even for the last station
//		msg << "<h4>A repair-up: pick a lot to work from a buffer</h4>" << endl;
//		//usint bNo = (ibNos[si].size() == 1) ? ibNos[si][0] : pick_bufferNo(si);
//		const v_usint& bNos = ibNos[si];
//		// select to process for current machine
//		usint bNo = (bNos.size() == 1) ? bNos[0] : maxIb_minOb(si);
//		msgiob << html_NoIbOb(si, bNo);
//		msg << "<p>machine " << mi + 1 << " : " << machine;
//		if (buffers[bNo - 1] == 0)// no waiting lot
//		{
//			machine = 0;
//			msg << " -> " << machine << ", ";
//			msg << "buffer " << bNo << " : " << buffers[bNo - 1] << " -> ";
//			msg << buffers[bNo - 1] << ".</p> " << endl;
//		}
//		else
//		{
//			machine = bNo;
//			msg << " -> " << machine << ", ";
//
//			msg << "buffer " << bNo << " : " << buffers[bNo - 1] << " -> ";
//			buffers[bNo - 1]--;
//			msg << buffers[bNo - 1] << ".</p> " << endl;
//
//			SEL[ei - 2] = t + rexp(mpts[si]);// add event 1
//			SEL[ei - 1] = t + rexp(mtbfs[si]);// add event 2
//		}
//		msgb << html_buffers(bNo);
//		msgm << html_machines_SEL(ei + 1, mi + 1);
//		message += msg.str() + msgiob.str() + msgb.str() + msgm.str();
//		return;
//	}
//
//	//Now tp = 1, i.e., completion of current processing step
//	usint& b = buffers[machine - 1];//machine = j, j=1,2,...,60
//	//-----------------------------------------------------------
//	//step is the last step
//	//-----------------------------------------------------------
//	if (machine == 60)
//	{
//		msg << "<h4>A completion: finishing final step of a lot</h4>" << endl;
//		msg << "<p>machine " << mi + 1 << " : " << machine;
//		produced_lots++;
//		if (b == 0)
//		{
//			machine = 0;
//			msg << " -> " << machine << ", ";
//			SEL[ei + 1] = inf;//del event 2
//			msg << "buffer " << 60 << " : " << buffers[60 - 1] << " -> ";
//			msg << buffers[60 - 1] << ".</p> " << endl;
//		}
//		else
//		{
//			msg << " -> " << machine << ", ";
//			msg << "buffer " << 60 << " : " << buffers[60 - 1] << " -> ";
//			b--;
//			msg << buffers[60 - 1] << ".</p> " << endl;
//
//			SEL[ei] = t + rexp(mpts[si]);// a new event 1; keep event 2
//		}
//		msgb << html_buffers(60);
//		msgm << html_machines_SEL(ei + 1, mi + 1);
//		message += msg.str() + msgb.str() + msgm.str();
//		return;
//	}
//
//	//-----------------------------------------------------------
//	//step having next step
//	//-----------------------------------------------------------
//	//msg = "<p>t: " + to_string(t) + " produced lots: " + to_string(produced_lots) + "</p>\n<p>";
//	usint nextsi = step_sNo[machine] - 1;// si = sNo - 1; nextsi: next station index
//	usint& nextb = buffers[machine];
//	usint nextmNo = 0;
//	usint nextbNo = machine + 1;
//	//update the state in the next station
//	//increment buffer first
//	nextb++;
//	//check whether there are at least one idle machine in the next station
//	usint start = (nextsi == 0) ? 0 : mcumsums[nextsi - 1];
//	msg << "<h4>A completion: may or not trigger consequent machine change</h4>" << endl;
//	for (size_t i = start; i < mcumsums[nextsi]; i++)
//	{
//		if (machines[i] == 0)
//		{
//			nextmNo = i + 1;
//			msg << "<h4>A completion: trigger an idle machine in next station to work</h4>" << endl;
//			msg << "<p>Passive machine " << i + 1 << " : " << machines[i];
//			machines[i] = machine + 1;////machine = j
//			msg << " -> " << machines[i] << ", ";
//
//			msg << "passive buffer " << machine << " : " << nextb;
//			nextb--;
//			msg << " -> " << nextb << ". ";
//
//			SEL[i * 3] = t + rexp(mpts[nextsi]);//add event 1
//			SEL[i * 3 + 1] = t + rexp(mtbfs[nextsi]);//add event 2
//			break;
//		}
//	}
//
//	//update the current machine state
//	//usint bNo = (ibNos[si].size() == 1) ? ibNos[si][0] : pick_bufferNo(si);
//	const v_usint& bNos = ibNos[si];
//	// select to process for current machine
//	usint bNo = (bNos.size() == 1) ? bNos[0] : maxIb_minOb(si);
//	msg << "<p>machine " << mi + 1 << " : " << machine;
//	if (buffers[bNo - 1] == 0)// no waiting lot
//	{
//		machine = 0;
//		msg << " -> " << machine << ", ";
//
//		msg << "buffer " << bNo << " : " << buffers[bNo - 1] << " -> ";
//		msg << buffers[bNo - 1] << ".</p> " << endl;
//
//		SEL[ei + 1] = inf;// delete event 2
//	}
//	else
//	{
//		machine = bNo;
//		msg << " -> " << machine << ", ";
//
//		msg << "buffer " << bNo << " : " << buffers[bNo - 1] << " -> ";
//		buffers[bNo - 1]--;
//		msg << buffers[bNo - 1] << ".</p> " << endl;
//
//		SEL[ei] = t + rexp(mpts[si]);// a new event 1
//	}
//	msgb << html_buffers(bNo, nextbNo);
//	msgm << html_machines_SEL(ei + 1, mi + 1, nextmNo);
//	msgiob << html_NoIbOb(si, bNo);
//	message += msg.str() + msgiob.str() + msgb.str() + msgm.str();
//	return;
//}
//
//void Fab::simulation_and_record() {
//	initialize_state(20, 0.0f);
//	size_t ei;
//	stringstream msg{};
//
//	write_to_html();
//
//	while (produced_lots < target_lots)
//	{
//		message = "";
//		msg.str("");
//		msg.clear();
//		msg << endl << "<h3 class='iter'>One iteration</h3>" << endl;
//		msg << "<p>Before current event t: " << fixed << setprecision(2) << t;
//		msg << ", produced lots: " << produced_lots << ". </p>" << endl;
//
//		ei = min_element(SEL.begin(), SEL.end()) - SEL.begin();
//		t = SEL[ei];
//
//		//
//		msg << html_buffers((machines[ei / 3] > 0) ? machines[ei / 3] : 0) << endl;
//		msg << html_machines_SEL(ei + 1, ei / 3) << endl;
//		message += msg.str();
//		write_to_html();
//		message = "";
//
//		SEL[ei] = inf;// event in ei happens, remove from SEL
//		update_state(ei);
//		write_to_html();
//	}
//	message = "";
//	write_to_html();
//	return;
//}
//
//void Fab::write_to_html(string fname) {
//	ofstream fout;
//	fout.open(fname, ios_base::app);
//	//write head
//	if (t < 0.0001)
//	{
//		ifstream fin("head.html");
//		string text;
//		while (getline(fin, text))
//		{
//			fout << text;
//		}
//		fin.close();
//	}
//	//
//	fout << message << endl << endl;
//	//write foot
//	if (produced_lots == target_lots)
//	{
//		ifstream fin("foot.html");
//		string text;
//		while (getline(fin, text))
//		{
//			fout << text;
//		}
//		fin.close();
//	}
//	//
//	fout.close();
//}
//
//string Fab::html_buffers(usint bNo1, usint bNo2) {
//	stringstream ss{};
//	ss << "<table id='buffers'>" << endl;
//	//title
//	ss << "\t<tr>" << endl;
//	ss << "\t\t<th>bNo</th> ";
//	for (usint i = 0; i < 60; i++)
//	{
//		ss << "<td>" << i + 1 << "</td> ";
//	}
//	ss << endl;
//	ss << "\t</tr>" << endl;
//	//data
//	ss << "\t<tr>" << endl;
//	ss << "\t\t<th>content</th> ";
//	for (usint i = 0; i < 60; i++)
//	{
//		ss << (((i == bNo1 - 1) || (i == bNo2 - 1))? "<td class='selected-buffer'>" : "<td>");
//		ss << buffers[i] << "</td> ";
//	}
//	ss << endl;
//	ss << "\t</tr>" << endl;
//	ss << "</table>" << endl;
//	return ss.str();
//}
//
//string Fab::html_machines_SEL(usint eNo, usint mNo1, usint mNo2) {
//	stringstream ss{}, tr0{}, mss{}, tp1{}, tp2{}, tp3{};
//	ss << "<table id='machines'>" << endl;
//	tr0 << "\t<tr>" << endl << "\t\t<th>mNo</th> ";
//	mss << "\t<tr>" << endl << "\t\t<th>state</th> ";
//	tp1 << "\t<tr>" << endl << "\t\t<th>etp 1</th> ";
//	tp2 << "\t<tr>" << endl << "\t\t<th>etp 2</th> ";
//	tp3 << "\t<tr>" << endl << "\t\t<th>etp 3</th> ";
//
//	//title and data
//	for (usint i = 0; i < machines.size(); i++)
//	{
//		tr0 << "<td>" << (i + 1) << "</td> ";
//
//		mss << (((i == mNo1 - 1) || (i == mNo2 - 1))? "<td class='updated-machine'>" : "<td>");
//		mss << machines[i] << "</td> ";
//
//		tp1 << ((i * 3 + 1 != eNo) ? "<td>" : "<td class='occurring-event'>");
//		tp1 << fixed << setprecision(2) << SEL[i * 3] << "</td> ";
//
//		tp2 << ((i * 3 + 2 != eNo) ? "<td>" : "<td class='occurring-event'>");
//		tp2 << fixed << setprecision(2) << SEL[i * 3 + 1] << "</td> ";
//
//		tp3 << ((i * 3 + 3 != eNo) ? "<td>" : "<td class='occurring-event'>");
//		tp3 << fixed << setprecision(2) << SEL[i * 3 + 2] << "</td> ";
//	}
//	tr0 << endl << "\t</tr>" << endl;
//	mss << endl << "\t</tr>" << endl;
//	tp1 << endl << "\t</tr>" << endl;
//	tp2 << endl << "\t</tr>" << endl;
//	tp3 << endl << "\t</tr>" << endl;
//
//	ss << tr0.str() << mss.str() << tp1.str() << tp2.str() << tp3.str();
//	ss << "</table>" << endl;
//	return ss.str();
//}
//
//string Fab::html_NoIbOb(usint si, usint bNo) {
//	stringstream ss{}, ssno{}, ssib{}, ssob{};
//	ss << "<table id='NoIbObs'>" << endl;
//	ssno << "\t<tr>" << endl << "\t\t<th>bNo</th> ";
//	ssib << "\t<tr>" << endl << "\t\t<th>Ib</th> ";
//	ssob << "\t<tr>" << endl << "\t\t<th>Ob</th> ";
//
//	for (size_t i = 0; i < ibNos[si].size(); i++)
//	{
//		ssno << ((ibNos[si][i] != bNo) ? "<td>" : "<td class='selected-buffer'>");
//		ssno << ibNos[si][i] << "</td> ";
//
//		ssib << "<td>" << buffers[ibNos[si][i] - 1] << "</td> ";
//		ssob << "<td>" << buffers[ibNos[si][i]] << "</td> ";
//	}
//	ssno << endl << "\t</tr>" << endl;
//	ssib << endl << "\t</tr>" << endl;
//	ssob << endl << "\t</tr>" << endl;
//	ss << ssno.str() << ssib.str() << ssob.str() << "</table>" << endl;
//	return ss.str();
//}