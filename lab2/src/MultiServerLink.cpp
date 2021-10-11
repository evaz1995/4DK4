#ifdef __cplusplus
extern "C" {
#include "simlib.h"
#include "trace.h"
#include "packet_transmission.h"
#include "simparameters.h"
#include "output.h"
#else
#pragma message ("C plus plus required")
#endif

#ifdef __cplusplus
}
#endif

#include <math.h>
#include <fstream>
#include <array>
#include <vector>
#include "utils.h"

/*
* Inintilize global variable
*/

#define DATA_PTR(x) (Simulation_Run_Data_Ptr) simulation_run_data(x)
double SWITCH_PROBILITY;

/*
* Function declaration 
*/
extern "C"
static void ERROR(struct _simulation_run_*, void*);
extern "C"
void SpecialArrivalScheduler(struct _simulation_run_* , void* );
extern "C"
void end_packet_transmission_event_handle(Simulation_Run_Ptr, void*);
static inline double GetServiceTime(double bitrate) { return PACKET_LENGTH / bitrate; }

class SingleServer;
class Simulation;
/*
* END
*/
class SingleServer {
public:
	Server* m_LocalServer;
	Fifoqueue_Ptr local_buffer;
	double m_bandwidth;
	double m_arrival_rate;
	int m_ID;

public:
	SingleServer(double bandwidth, double arrival_rate,int ID) :
		m_LocalServer(new Server{ FREE , nullptr }),
		local_buffer(fifoqueue_new()),
		m_bandwidth(bandwidth),
		m_arrival_rate(arrival_rate),
		m_ID(ID)
		{}

	~SingleServer() {
		std::cout << "destructor called\n";
		if(m_LocalServer->state == BUSY) xfree(server_get(m_LocalServer), sizeof(Server));
		cleanup_memory();
	}

	void cleanup_memory() noexcept
	{
		while (fifoqueue_size(local_buffer) > 0) /* Clean out the queue. */
			xfree(fifoqueue_get(local_buffer),sizeof(_queue_container_));
		xfree(local_buffer, sizeof(Fifoqueue));
		xfree(m_LocalServer, sizeof(Server));
	}


	//called when packet arrival event
	//and schedule next arrival event
	void schedule_packet_arrival_event(Simulation_Run_Ptr simulation_run, double event_time) 
	{
		Event event;

		event.description = "Packet Arrival";
		event.function = SpecialArrivalScheduler;
		event.attachment = this;

		//add to global event list
		simulation_run_schedule_event(simulation_run, event, event_time);
	}

	void bypass(Simulation_Run_Ptr simulation_run, Packet_Ptr _previous)
	{
		_previous->source_id = this->m_ID;

		if (m_LocalServer->state == FREE)
			transmission(simulation_run, _previous);
		else
			fifoqueue_put(local_buffer, (void*)_previous);
	}


	void arrival(Simulation_Run_Ptr simulation_run) {
		Simulation_Run_Data_Ptr data;
		Packet_Ptr new_packet;

		data = (Simulation_Run_Data_Ptr)simulation_run_data(simulation_run);
		data->arrival_count++;

		new_packet = (Packet_Ptr)xmalloc(sizeof(Packet));
		new_packet->arrive_time = simulation_run_get_time(simulation_run);
		new_packet->service_time = GetServiceTime(this->m_bandwidth);
		new_packet->status = WAITING;
		new_packet->source_id = m_ID;
		new_packet->destination_id = 2;

		if (m_LocalServer->state == FREE)
			transmission(simulation_run, new_packet);
		else 
			fifoqueue_put(local_buffer, (void*)new_packet);

		schedule_packet_arrival_event(simulation_run,
			simulation_run_get_time(simulation_run) +
			exponential_generator((double)1 / PACKET_ARRIVAL_RATE));
	}

	 void transmission(Simulation_Run_Ptr GlobalData, Packet* this_packet) {
		// start processing the packet
		// schedule departure time at the end
		server_put(m_LocalServer, (void*)this_packet);
		this_packet->status = XMTTING;

		Event event;
		event.description = "Packet Xmt End";
		event.function = end_packet_transmission_event_handle;
		event.attachment = this;

		double time = simulation_run_get_time(GlobalData) + this_packet->service_time;
		simulation_run_schedule_event(GlobalData, event, time); //schedule depart
	}
};


class Simulation {
private: 
	std::vector<SingleServer*> servers;
	Simulation_Run_Ptr simulation_run;
	long int* counter;

private:
	inline static utils::FileIO OUT{ "delay_vs_p12.txt", std::ofstream::out };

public:
	// (bit rate, arrival rate, level)
	Simulation(std::initializer_list<std::tuple<double,double,int>> server_config)
	{
		initialize(server_config);
	
		for(const auto& i : servers)
			i-> schedule_packet_arrival_event(simulation_run, simulation_run_get_time(simulation_run));
	}

	~Simulation()
	{
		output_results(simulation_run);
		terminate();
	}

	static void OutputData(Simulation_Run_Data_Ptr DATA) noexcept
	{
		try {
			OUT.FileIO::fprint(DATA);
		}
		catch (const std::exception&) {
			std::cerr << "Output file failed\n";
		}		
	}
	
	void terminate() noexcept
	{
		try {
			for (auto& i : servers)
				free(i);
			free(simulation_run->data);
			simulation_run_free_memory(simulation_run);
		}
		catch (const std::exception& e) {
			std::cerr << e.what();
		}
		
	}

	void ScheduleEvent(SingleServer*  server) {
		server->arrival(simulation_run);		
	}

	Simulation_Run_Data_Ptr GetData(){
		return (Simulation_Run_Data_Ptr)simulation_run_data(simulation_run);
	}

	//lazy scheduling
	//TO DO: add support for more servers
	SingleServer* GetAvaibleServer(SingleServer* current)
	{
		double prob = uniform_generator();
		for (auto& i : servers) {
			if (i->m_ID > current->m_ID && prob < SWITCH_PROBILITY)
				return i;
		}

		//none of the servers are free
		return servers[servers.size()-1];
	}

	void initialize( std::initializer_list<std::tuple<double,double, int>>& server_config) {
		for(const auto& elem : server_config)
			servers.emplace_back(new SingleServer{ std::get<0>(elem), std::get<1>(elem),std::get<2>(elem) });

		//servers[0]->m_IsEndServer = false;
		simulation_run = simulation_run_new();
		simulation_run->_SimulationClass_ = this;

		Simulation_Run_Data* data = new Simulation_Run_Data{};
		data->blip_counter = 0;
		data->arrival_count = 0;
		data->number_of_packets_processed = 0;
		data->accumulated_delay = 0.0;
		data->random_seed = 400176017;

		counter = &(data->number_of_packets_processed);
		random_generator_initialize(400176017);
		simulation_run_attach_data(simulation_run, (void*)data);

		std::cout << "SWITCH PROBABILITY = " << SWITCH_PROBILITY << "\n";
	}


	void run(long int max_iteration) {
		while (*counter < max_iteration) simulation_run_execute_event(simulation_run);	
	}

};


extern "C"
void* GetAvaibleServer(void* _Simulation_, void* _SingleServer_) {
	return static_cast<Simulation*>(_Simulation_)->GetAvaibleServer(static_cast<SingleServer*>(_SingleServer_));
}


extern "C"
static void ERROR(struct _simulation_run_*, void*) {
	TRACEF(printf("This function should not be called !!!\n"));
	exit(-1);
}

extern "C"
void SpecialArrivalScheduler(struct _simulation_run_* simulation_run, void* attachment) {
	static_cast<SingleServer*> (attachment)->arrival(simulation_run);
}

//Simulation_Run_Ptr, SingleServer*
//Caution: FIFO inside LocalServer should be used instead of simulation_run's
extern "C"
void
end_packet_transmission_event_handle(Simulation_Run_Ptr simulation_run, void* LocalServer)
{
	SingleServer* link = static_cast<SingleServer*> (LocalServer);

	Simulation_Run_Data_Ptr data;
	Packet_Ptr this_packet, next_packet;
	this_packet = static_cast<Packet_Ptr>(server_get(static_cast<Server_Ptr>(link->m_LocalServer)));

	TRACE(printf("End Of Packet.\n"););

	//not finish yet
	//add to the next avaible server
	if (this_packet->source_id < this_packet->destination_id)
	{
		SingleServer* next = static_cast<SingleServer*> (GetAvaibleServer(simulation_run->  _SimulationClass_, link));
		next->bypass(simulation_run, this_packet);
	}
	else
	{
		data = (Simulation_Run_Data_Ptr)simulation_run_data(simulation_run);
		/* Collect statistics. */
		data->number_of_packets_processed++;
		data->accumulated_delay += simulation_run_get_time(simulation_run) - this_packet->arrive_time;

		xfree(this_packet, sizeof(Packet));
		output_progress_msg_to_screen(simulation_run);
	}


	if (fifoqueue_size(link->local_buffer) > 0) {
		next_packet = (Packet_Ptr)fifoqueue_get(link->local_buffer);
		link->transmission(simulation_run, next_packet);
	}
}




double PACKET_ARRIVAL_RATE = 750.0;
int LATE_PACKETS = 0;
double max_delay = 0.0;
double min_delay = 0.0;

int main()
{// //(bit rate, arrival rate, level)
	utils::FileIO OUT{ "delay_vs_p12.txt", std::ofstream::out };

	for (auto samples : utils::sample<0, 1, 20>)
	{
		SWITCH_PROBILITY = samples;
		Simulation new_simulation{ {2e6, 750,1}, {1e6, 500,2}, {1e6, 500,2} };
		new_simulation.run(RUNLENGTH);
		Simulation::OutputData(new_simulation.GetData());
	}

}
	

	

