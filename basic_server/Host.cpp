#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include "common/EventTimer.h"
#include <CL/cl2.hpp>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "Pipeline.h"
#include "common/Utilities.h"

#define STAGES (4)

int main(int argc, char *argv[])
{
  unsigned char *Input_data = (unsigned char *)malloc(FRAMES * FRAME_SIZE);
  //unsigned char *Temp_data[STAGES - 1];
  unsigned char *Output_data = (unsigned char *)malloc(MAX_OUTPUT_SIZE);
   /*for (int Stage = 0; Stage < STAGES - 1; Stage++)
   {
       Temp_data[Stage] = (unsigned char *)malloc(FRAME_SIZE);
       if (Temp_data[Stage] == NULL)
       Exit_with_error("malloc failed at main for Temp_data");
   }*/

   Load_data(Input_data);

  EventTimer timer1, timer2;
  timer1.add("Main program");

  std::cout << "Running" << std::endl;
  // ------------------------------------------------------------------------------------
  // Step 1: Initialize the OpenCL environment
   // ------------------------------------------------------------------------------------
  timer2.add("OpenCL Initialization");
  cl_int err;
  std::string binaryFile = argv[1];
  unsigned fileBufSize;
  std::vector<cl::Device> devices = get_xilinx_devices();
  devices.resize(1);
  cl::Device device = devices[0];
  cl::Context context(device, NULL, NULL, NULL, &err);
  char *fileBuf = read_binary_file(binaryFile, fileBufSize);
  cl::Program::Binaries bins{{fileBuf, fileBufSize}};
  cl::Program program(context, devices, bins, NULL, &err);
  cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE
, &err);
  cl::Kernel krnl_filter(program, "Filter_HW", &err);

  // ------------------------------------------------------------------------------------
  // Step 2: Create buffers and initialize test values
  // ------------------------------------------------------------------------------------
  timer2.add("Allocate contiguous OpenCL buffers");

  cl::Buffer a_buf[FRAMES];
  cl::Buffer b_buf[FRAMES];
  for(int i = 0; i < FRAMES; i++)
  {
      a_buf[i] = cl::Buffer(context, CL_MEM_READ_ONLY, SCALED_FRAME_SIZE * sizeof(unsigned char), NULL, &err); // kernel reads inputs from a?
      b_buf[i] = cl::Buffer(context, CL_MEM_WRITE_ONLY, OUTPUT_FRAME_SIZE * sizeof(unsigned char), NULL, &err); // kernel write outputs from b?
  }

  unsigned char *a[FRAMES];
  unsigned char *b[FRAMES];
  for(int i = 0; i < FRAMES; i++)
  {
      a[i] = (unsigned char*)q.enqueueMapBuffer(a_buf[i], CL_TRUE, CL_MAP_WRITE, 0,  SCALED_FRAME_SIZE * sizeof(unsigned char)); // we write inputs to a?
      b[i] = (unsigned char*)q.enqueueMapBuffer(b_buf[i], CL_TRUE, CL_MAP_READ, 0, OUTPUT_FRAME_SIZE * sizeof(unsigned char)); // we read outputs from b?
  }

  //timer2.add("Populating buffer inputs");
  //init_arrays(a, b);)


  // ------------------------------------------------------------------------------------
  // Step 3: Run the kernel
  // ------------------------------------------------------------------------------------

  timer2.add("Scale");
 
  for (int i = 0; i < FRAMES; i++)
  {
      //Scale_SW(Input_data + i * FRAME_SIZE, Temp_data[0]); // allocating a lot of mem ifl
      Scale_SW(Input_data + i * FRAME_SIZE, a[i]);
  }

  timer2.add("Filter");
 //cl::Event read_events[FRAMES];
 for (int i = 0; i < FRAMES; i++)
  {
      //memcpy(a[i], Temp_data[0], SCALED_FRAME_SIZE);

      std::vector<cl::Event> exec_events, write_events;
      cl::Event write_ev, exec_ev, read_ev;

      krnl_filter.setArg(0, a_buf[i]);
      krnl_filter.setArg(1, b_buf[i]);
    
      std::vector<cl::Event> write_deps;
      //if (read_events[i]()) write_deps.push_back(read_events[i]);

      q.enqueueMigrateMemObjects({a_buf[i]}, 0 /* 0 means from host*/, write_deps.empty() ? nullptr : &write_deps, &write_ev);

      write_events.push_back(write_ev);
      q.enqueueTask(krnl_filter, &write_events, &exec_ev);
      exec_events.push_back(exec_ev);
      q.enqueueMigrateMemObjects({b_buf[i]}, CL_MIGRATE_MEM_OBJECT_HOST, &exec_events, &read_ev);
    
      //read_events[i] = read_ev;

      //q.enqueueUnmapMemObject(a_buf[i], a[i]);
      //q.enqueueUnmapMemObject(b_buf[i], b[i]);
      //memcpy(Temp_data[1], b[i], OUTPUT_FRAME_SIZE);
  }

  q.finish();

  timer2.add("Diff");
  int Size = 0;
  unsigned char *Temp_data[FRAMES];
  for (int i = 0; i < FRAMES; i++)
  {
     Temp_data[i] = (unsigned char *)malloc(OUTPUT_FRAME_SIZE);
     //unsigned char Temp_data[OUTPUT_FRAME_SIZE];
      Differentiate_SW(b[i], Temp_data[i]);
  }

  timer2.add("Compress");
  for (int i = 0; i < FRAMES; i++)
  {
    Size = Compress_SW(Temp_data[i], Output_data);
    free(Temp_data[i]);
  }




  // ------------------------------------------------------------------------------------
  // Step 4: Release Allocated Resources
  // ------------------------------------------------------------------------------------
  timer2.add("Writing output to output");
 Store_data("Output.bin", Output_data, Size);
 Check_data(Output_data, Size);

 free(Input_data);
 free(Output_data);
 /*
  timer2.add("Writing output to output_fpga.bin");
  FILE *file = fopen("output_fpga.bin", "wb");

  for (int i = 0; i < NUM_MAT; i++)
  {
    fwrite(c[i], 1, bytes_per_iteration, file);
  }
  fclose(file);

  */
  delete[] fileBuf;
 
  timer2.finish();
 
  std::cout << "--------------- Key execution times ---------------"
  << std::endl;
  timer2.print();

  timer1.finish();
  std::cout << "--------------- Total time ---------------"
  << std::endl;
  timer1.print();
  return 0;
}