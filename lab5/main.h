#pragma once

struct CellPhone_packet;
struct CellPhone;
class Simulation;
class BaseStation;

typedef struct _simulation_run_data_
{
	Fifoqueue_Ptr buffer;
	long int processed_packet;
	long int total_arrrival;
	BaseStation* station;
} Simulation_Run_Data, * Simulation_Run_Data_Ptr;

struct CellPhone_packet {
	double m_execution_time;
	double arrival_time;
	double upload_time;
	CellPhone* Cellphone_identifier;
};

struct CellPhone {
	double total_delay = 0.0;
	double last_packet_time = 0.0;
	long int arrival_count = 0;

	static void end_processing(Simulation_Run_Ptr , void*);
	static void print_result(CellPhone* );
	double new_packet(double, double);
};

class Simulation {
private:
	static Simulation_Run_Ptr simulation_run;
	std::unique_ptr<BaseStation> station_ptr;
public:
	Simulation();
	~Simulation();

	void fill_data(std::unique_ptr<BaseStation>& station);
	void run(long int max_iteration);
	void reset();
	void find_threhold();
	static void OutputData();
};


class BaseStation {
private:
	std::vector<CellPhone> cellphones;
	Fifoqueue_Ptr m_station_buffer;
	CellPhone_packet* packet_in_processing;
	double m_arrival_rate;
	Server_State server;

public:
	BaseStation();
	~BaseStation();

	static void Cloud_processing(Simulation_Run_Ptr, CellPhone_packet*);
	friend void Simulation::fill_data(std::unique_ptr<BaseStation>&);

	void Schedule_Arrival_At_Station(Simulation_Run_Ptr simulation_run, double event_time);
	CellPhone_packet* GetPacket();
	CellPhone_packet* GenerateNewPacket(Simulation_Run_Ptr);
	void process_packet(CellPhone_packet* packet);
	void schedule_packet_arrival_event(Simulation_Run_Ptr, double);
	void Arrival_event(Simulation_Run_Ptr);
	double get_max_delay();
	void Print_result();
};