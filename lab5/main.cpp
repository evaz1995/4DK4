#ifdef __cplusplus
extern "C" {
#include "simlib.h"
	void BaseStation_arrival(Simulation_Run_Ptr , void* );
	void end_event_wrapper(Simulation_Run_Ptr , void* );
	void Cellphone_arrival(Simulation_Run_Ptr , void*);
	#pragma message("C++ defined")
#endif
#ifdef __cplusplus
}
#endif


#define Jg 1.0
#define Jb 1.0
#define Ug 1.0
#define Ub 10*Ug
#define MAX_ITERATION 100
#define D_MAX 1000
double ARRIVAL_RATE = 0.05;

#include <array>
#include <vector>
#include<memory>
#include<iostream>
#include "main.h"



void CellPhone::end_processing(Simulation_Run_Ptr simulation_run, void* _)
{
	Simulation_Run_Data_Ptr data_ptr = static_cast<Simulation_Run_Data_Ptr>(simulation_run->data);
	CellPhone_packet* current_packet,* next_packet;
	BaseStation*station = data_ptr->station;
		
	current_packet = station->GetPacket();;
	current_packet->Cellphone_identifier->total_delay +=
		simulation_run_get_time(simulation_run) - current_packet->arrival_time;
	delete current_packet;
	data_ptr->processed_packet++;

	//check if there is any packet in the fifo queue
	if (fifoqueue_size(data_ptr->buffer) > 0)
	{
		next_packet = static_cast<CellPhone_packet*>(fifoqueue_get(data_ptr->buffer));
		station->Cloud_processing(simulation_run,next_packet);
	}
		
}

void CellPhone::print_result(CellPhone* phone)
{
	std::cout << "total processed packet: " << phone->arrival_count <<"\t"
		<< "mean delay: " <<phone->total_delay / phone->arrival_count<<"\t"
		<< "\n";
}

double CellPhone::new_packet(double upload_time, double current_time)
{
	if(current_time >= last_packet_time)
		this->last_packet_time = upload_time + current_time;	//no delay
	else 
		this->last_packet_time = upload_time + last_packet_time;

	arrival_count++;
	return last_packet_time;
}

Simulation::Simulation()
{
	Simulation_Run_Data* data = new Simulation_Run_Data{};
	simulation_run_attach_data(simulation_run, (void*)data);
	station_ptr = std::make_unique<BaseStation>();
	fill_data(station_ptr);

	random_generator_initialize(400176017);
	station_ptr->schedule_packet_arrival_event(simulation_run, 0.0);
}

Simulation::~Simulation()
{
	simulation_run_free_memory(simulation_run);
}

void Simulation::reset() 
{
	simulation_run_free_memory(simulation_run);
	Simulation::simulation_run = simulation_run_new();
	Simulation_Run_Data* data = new Simulation_Run_Data{};
	simulation_run_attach_data(simulation_run, (void*)data);

	station_ptr = std::make_unique<BaseStation>();
	fill_data(station_ptr);
	random_generator_initialize(400176);
	station_ptr->schedule_packet_arrival_event(simulation_run, 0.0);
}

void Simulation::fill_data(std::unique_ptr<BaseStation>& station) 
{
	((Simulation_Run_Data_Ptr)(simulation_run->data)) -> processed_packet = 0;
	((Simulation_Run_Data_Ptr)(simulation_run->data))->total_arrrival = 0;
	((Simulation_Run_Data_Ptr)(simulation_run->data)) -> buffer =  station->m_station_buffer;
	((Simulation_Run_Data_Ptr)(simulation_run->data))->station = station.get();
}

void Simulation::run(long int max_iteration) 
{
	long int& tmp = ((Simulation_Run_Data_Ptr)(simulation_run->data))->total_arrrival;
	while (tmp < max_iteration) 
		simulation_run_execute_event(simulation_run);
}

void Simulation::find_threhold()
{
	double max_delay;
	do {
		run(MAX_ITERATION);
		max_delay = station_ptr->get_max_delay();
		ARRIVAL_RATE += 0.01;
		OutputData();
		if (max_delay < D_MAX) {
			reset();
			std::cout << "\n\n";
		}
	} while (max_delay < D_MAX);
}

void Simulation::OutputData()
{
	std::cout << "arrival rate: " << ARRIVAL_RATE << "\n";
	std::cout << "running time: " << simulation_run_get_time(simulation_run) << "\n";
	std::cout << "total arrival count: " << ((Simulation_Run_Data_Ptr)(simulation_run->data))->total_arrrival<< "\n";
}

BaseStation::BaseStation():
	m_station_buffer(fifoqueue_new()),
	m_arrival_rate(ARRIVAL_RATE),
	packet_in_processing(nullptr),
	server(FREE)
{
	cellphones.emplace_back(); cellphones.emplace_back();
}

BaseStation::~BaseStation()
{
	while (fifoqueue_size(m_station_buffer) > 0) /* Clean out the queue. */
		xfree(fifoqueue_get(m_station_buffer));
	xfree(m_station_buffer);

	if (server == BUSY) delete packet_in_processing;

	Print_result();
	std::cout << "basestation destroyed\n";
}

CellPhone_packet* BaseStation::GenerateNewPacket(Simulation_Run_Ptr simulation_run) 
{
	double excution_time, upload_time;
	CellPhone* Cellphone_identifier;
	double current_time = simulation_run_get_time(simulation_run);

	if (uniform_generator() > 0.5) {
		excution_time = Jb;
		upload_time = Ub;
		Cellphone_identifier = &cellphones[1];
	}
	else
	{
		excution_time = Jg;
		upload_time = Ug;
		Cellphone_identifier = &cellphones[0];
	}
	
	double upload_finsih_time = Cellphone_identifier->new_packet(upload_time, current_time);

	CellPhone_packet* new_packet = new CellPhone_packet{ excution_time, current_time, upload_finsih_time,Cellphone_identifier };
	return new_packet;
}


CellPhone_packet* BaseStation::GetPacket() 
{
	this->server = FREE;
	return this->packet_in_processing;
}
//a new packet arrives at a cellphone
void BaseStation::Arrival_event(Simulation_Run_Ptr simulation_run) 
{
	double current_time = simulation_run_get_time(simulation_run);

	Schedule_Arrival_At_Station(simulation_run, current_time);

	double next_packet_time = current_time +
		exponential_generator((double)1 /m_arrival_rate);
	schedule_packet_arrival_event(simulation_run, next_packet_time);
}

void BaseStation::Cloud_processing(Simulation_Run_Ptr simulation_run, CellPhone_packet* packet)
{
	std::cout << "cloud process called\n";

	//check if server is empty
	Simulation_Run_Data_Ptr data = (Simulation_Run_Data_Ptr)simulation_run->data;
	BaseStation* station = data->station;

	if (station->server == FREE) {
		station->process_packet(packet);

		Event event;
		event.description = "Packet Processing End";
		event.function = end_event_wrapper;
		event.attachment = nullptr;

		double event_time = simulation_run_get_time(simulation_run) + packet->m_execution_time;
		simulation_run_schedule_event(simulation_run, event, event_time); //schedule depart
	}
	else
	{
		fifoqueue_put(data->buffer, packet);
	}
}

//generating a arrival event placeholder
void BaseStation::schedule_packet_arrival_event(Simulation_Run_Ptr simulation_run, double event_time)
{
	Event event;
	event.description = "Packet Arrival at cellphone";
	event.function = Cellphone_arrival;
	event.attachment = this;

	simulation_run_schedule_event(simulation_run, event, event_time);
}

void BaseStation::Schedule_Arrival_At_Station(Simulation_Run_Ptr simulation_run, double current_time)
{
	CellPhone_packet* new_packet = GenerateNewPacket(simulation_run);
	((Simulation_Run_Data_Ptr)(simulation_run->data))->total_arrrival++;

	Event event;
	event.description = "Packet Arrival at base station";
	event.function = BaseStation_arrival;
	event.attachment = new_packet;
	

	simulation_run_schedule_event(simulation_run, event, new_packet->upload_time);
}

double BaseStation::get_max_delay()
{
	double max_delay = 0.0;
	for (auto& i : cellphones) {
		double delay = i.total_delay / i.arrival_count;
		max_delay = max_delay > delay ? max_delay : delay;
	}
	return max_delay;
}

void BaseStation::process_packet(CellPhone_packet* packet)
{
	this->packet_in_processing = packet;
	this->server = BUSY;
}

void BaseStation::Print_result()
{
	for (int i = 0; i < cellphones.size(); i++) {
		//printf("cellphone: %ld, total processed packet: %d, mean delay: %f\n",
			//i, cellphones[i].arrival_count, cellphones[i].total_delay/ cellphones[i].arrival_count);
		CellPhone::print_result(&cellphones[i]);
	}
}


extern "C"
void Cellphone_arrival(Simulation_Run_Ptr simulation_run, void* attachment) {
	static_cast<BaseStation*>(attachment)->Arrival_event(simulation_run);
}

extern "C"
void BaseStation_arrival(Simulation_Run_Ptr simulation_run, void* attachment)
{
	BaseStation::Cloud_processing(simulation_run, (CellPhone_packet*)attachment);
}

extern "C"
void end_event_wrapper(Simulation_Run_Ptr simulation_run, void* attachment)
{
	CellPhone::end_processing(simulation_run, nullptr);
}


Simulation_Run_Ptr Simulation::simulation_run = simulation_run_new();

int main() {
	Simulation debug{};
	debug.run(MAX_ITERATION);
	debug.OutputData();
	return 0;
}