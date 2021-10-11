#define QUSETION5
/*
 * 
 * Simulation_Run of A Single Server Queueing System
 * 
 * Copyright (C) 2014 Terence D. Todd Hamilton, Ontario, CANADA,
 * todd@mcmaster.ca
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "output.h"
#include "simparameters.h"
#include "packet_arrival.h"
#include "cleanup_memory.h"
#include "trace.h"
#include "main.h"
#else
#pragma message ("C plus plus required")
#endif

#ifdef __cplusplus
}
#include <fstream>
#include <array>
#include "utils.h"
#endif


/******************************************************************************/

/*
 * main.c declares and creates a new simulation_run with parameters defined in
 * simparameters.h. The code creates a fifo queue and server for the single
 * server queueuing system. It then loops through the list of random number
 * generator seeds defined in simparameters.h, doing a separate simulation_run
 * run for each. To start a run, it schedules the first packet arrival
 * event. When each run is finished, output is printed on the terminal.
 */

#define DATA_PTR(x) (Simulation_Run_Data_Ptr) simulation_run_data(x)


#ifdef QUSETION3

int find_maximum_arrival(double upperbound)
{
    Simulation_Run_Ptr simulation_run;
    Simulation_Run_Data data;
    unsigned RANDOM_SEEDS[] = { RANDOM_SEED_LIST, 0 };
    unsigned random_seed = RANDOM_SEED_LIST;

    for (;;) {

        simulation_run = simulation_run_new(); /* Create a new simulation run. */

        simulation_run_attach_data(simulation_run, (void*)&data);

        data.blip_counter = 0;
        data.arrival_count = 0;
        data.number_of_packets_processed = 0;
        data.accumulated_delay = 0.0;
        data.random_seed = random_seed;

        data.buffer = fifoqueue_new();
        data.link = server_new();

        random_generator_initialize(random_seed);

        schedule_packet_arrival_event(simulation_run,
            simulation_run_get_time(simulation_run));

        while (data.number_of_packets_processed < RUNLENGTH) 
            simulation_run_execute_event(simulation_run);

        output_results(simulation_run);
        cleanup_memory(simulation_run);

        if (double(LATE_PACKETS) / double(RUNLENGTH) > upperbound)
        {
            printf("current arrival rate: %f\n", PACKET_ARRIVAL_RATE);
            printf("current LATE_PACKETS rate: %f\n", double(LATE_PACKETS) / double(RUNLENGTH));
            break;
        }
        else
        {
            printf("current LATE_PACKETS: %d\n", LATE_PACKETS);
            printf("current max delay: %f\n", max_delay);
            PACKET_ARRIVAL_RATE += 0.001;
        }
    }

    return 0;
}
#endif

#ifdef QUSETION4



int find_maximum_arrival(double upperbound)
{
    Simulation_Run_Ptr simulation_run;
    Simulation_Run_Data data;
    unsigned RANDOM_SEEDS[] = { RANDOM_SEED_LIST, 0 };
    unsigned random_seed = RANDOM_SEED_LIST;

    utils::FileIO delay_vs_arrival{ "delay_vs_arrival_2s.txt", std::ofstream::out };

    for (auto i : utils::sample<25, 20, 1500>) {
        PACKET_ARRIVAL_RATE = i;
        printf("Servers number: %d\n", 2);
        simulation_run = simulation_run_new(); /* Create a new simulation run. */

        simulation_run_attach_data(simulation_run, (void*)&data);

        data.blip_counter = 0;
        data.arrival_count = 0;
        data.number_of_packets_processed = 0;
        data.accumulated_delay = 0.0;
        data.random_seed = random_seed;

        data.buffer = fifoqueue_new();
        data.link = ServerGroup{ server_new(), server_new() };

        random_generator_initialize(random_seed);

        schedule_packet_arrival_event(simulation_run,
            simulation_run_get_time(simulation_run));

        while (data.number_of_packets_processed < RUNLENGTH)
            simulation_run_execute_event(simulation_run);

        delay_vs_arrival.FileIO::fprint(DATA_PTR(simulation_run));
        output_results(simulation_run);
        cleanup_memory(simulation_run);

  }

    return 0;
}
#endif

#ifdef TEST
int
main(void)
{


  Simulation_Run_Ptr simulation_run;
  Simulation_Run_Data data;

  //utils::FileIO delay_vs_arrival{ "delay_vs_arrival.txt", std::ofstream::out};

  /*
   * Declare and initialize our random number generator seeds defined in
   * simparameters.h
   */

  unsigned RANDOM_SEEDS[] = {RANDOM_SEED_LIST, 0};
  unsigned random_seed = RANDOM_SEED_LIST;
  int j=0;

  /* 
   * Loop for each random number generator seed, doing a separate
   * simulation_run run for each.
   */

  /*
  * construct sample points
  */

  for (auto i : utils::sample<21,100>) {
      PACKET_ARRIVAL_RATE = i;

    simulation_run = simulation_run_new(); /* Create a new simulation run. */

    /*
     * Set the simulation_run data pointer to our data object.
     */

    simulation_run_attach_data(simulation_run, (void *) & data);

    /* 
     * Initialize the simulation_run data variables, declared in main.h.
     */
    
    data.blip_counter = 0;
    data.arrival_count = 0;
    data.number_of_packets_processed = 0;
    data.accumulated_delay = 0.0;
    data.random_seed = random_seed;
 
    /* 
     * Create the packet buffer and transmission link, declared in main.h.
     */

    data.buffer = fifoqueue_new();
    data.link   = server_new();

    /* 
     * Set the random number generator seed for this run.
     */

    random_generator_initialize(random_seed);

    /* 
     * Schedule the initial packet arrival for the current clock time (= 0).
     */

    schedule_packet_arrival_event(simulation_run, 
				  simulation_run_get_time(simulation_run));

    /* 
     * Execute events until we are finished. 
     */

    while(data.number_of_packets_processed < RUNLENGTH) {
      simulation_run_execute_event(simulation_run);
    }

    /*
     * Output results and clean up after ourselves.
     */

    output_results(simulation_run);
    //delay_vs_arrival.FileIO::fprint(DATA_PTR(simulation_run));
    cleanup_memory(simulation_run);
  }
  find_maximum_arrival(0.02);
  return 0;
}

#endif

#ifdef QUSETION5



#endif // QUSETION5









