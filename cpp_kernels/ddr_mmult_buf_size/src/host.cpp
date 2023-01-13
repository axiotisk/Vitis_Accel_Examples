/**
* Copyright (C) 2019-2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

/*****
This example demonstrates how PLRAM feature of the Vitis memory subsystem and
how they
integrate with the Vitis design process.
PLRAMs are small shared memories which are built using the on-chip memory
resources
of the FPGA fabric. They are intended to provide a small amount of data storage
that
application kernels can share and access rapidly. PLRAMs behave just like the
DDR
memory resources managed by the Vitis memory subsystem.
*****/

#include "event_timer.hpp"

// OpenCL utility layer include
#include "xcl2.hpp"
#include <algorithm>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream> //[K]
// Array Size to access
#define DATA_SIZE 639 // 639*639=408321

// Binary File string
std::string binaryFile;

// CPU implementation of Matrix Multiplication
// The inputs are of the size (DATA_SIZE x DATA_SIZE)
void mmult_cpu(int* in1, // Input Matrix 1
               int* in2, // Input Matrix 1
               int* out, // Input Matrix 1
               int dim   // One dimension of matrix
               ) {
    // Performs Matrix multiply Out = In1 x In2
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            for (int k = 0; k < dim; k++) {
                out[i * dim + j] += in1[i * dim + k] * in2[k * dim + j];
            }
        }
    }
}

// Functionality to setup OpenCL context and trigger the Kernel
void init_fpga(cl::CommandQueue q,
               cl::Context context,
               cl::Kernel kernel,
               cl_int err,
               int dim                                                         // One dimension of matrix 
               ) {
    // The get_xil_devices will return vector of Xilinx Devices
    auto devices = xcl::get_xil_devices();
    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));

        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err, kernel = cl::Kernel(program, "mmult", &err));
            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }
}

EventTimer mmult_fpga(std::vector<int, aligned_allocator<int> >& source_in1,          // Input Matrix 1
                std::vector<int, aligned_allocator<int> >& source_in2,          // Input Matrix 2
                std::vector<int, aligned_allocator<int> >& source_fpga_results, // Output Matrix
                cl::CommandQueue q,
                cl::Context context,
                cl::Kernel kernel,
                cl_int err,
                int size,                                                        // One dimension of matrix 
                EventTimer et
                ) {
    //[K] Normaly would return void. Changed to return also the info from the event timer

    EventTimer et_func;
    et_func = et;
    size_t matrix_size_bytes = sizeof(int) * size * size;

    cl::Buffer buffer_in1(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, matrix_size_bytes, source_in1.data(), &err);

    cl::Buffer buffer_in2(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, matrix_size_bytes, source_in2.data());

    cl::Buffer buffer_output(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, matrix_size_bytes,
                             source_fpga_results.data(), &err);

    /*
     * Using setArg(), i.e. setting kernel arguments, explicitly before
     * enqueueMigrateMemObjects(),
     * i.e. copying host memory to device memory,  allowing runtime to associate
     * buffer with correct
     * DDR banks automatically.
    */

    int a_row = size;
    int a_col = size;
    int b_col = size;
    // Set the kernel arguments
    int narg = 0;
    kernel.setArg(narg++, buffer_in1);
    kernel.setArg(narg++, buffer_in2);
    kernel.setArg(narg++, buffer_output);
    kernel.setArg(narg++, a_row);
    kernel.setArg(narg++, a_col);
    kernel.setArg(narg++, b_col);

    et_func.add("Memory object migration enqueue");
    q.enqueueMigrateMemObjects({buffer_in1, buffer_in2}, 0 /* 0 means from host*/);
    et_func.finish();

    // Launch the kernel
    et_func.add("Wait for kernels to complete");
    q.enqueueTask(kernel);

    q.enqueueMigrateMemObjects({buffer_output}, CL_MIGRATE_MEM_OBJECT_HOST);
    q.finish();
    et_func.finish();
    return et_func;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }

    // Initialize an event timer we'll use for monitoring the application
    EventTimer et;

    binaryFile = argv[1];

    // Allocate Memory in Host Memory
    int size = DATA_SIZE;
    size_t matrix_size_bytes = sizeof(int) * size * size; 
    std::vector<int, aligned_allocator<int> > source_in1(matrix_size_bytes);
    std::vector<int, aligned_allocator<int> > source_in2(matrix_size_bytes);
    std::vector<int, aligned_allocator<int> > source_fpga_results(matrix_size_bytes);
    std::vector<int, aligned_allocator<int> > source_cpu_results(matrix_size_bytes);

    // Create the test data
    std::generate(source_in1.begin(), source_in1.end(), std::rand);

    std::generate(source_in2.begin(), source_in2.end(), std::rand);
//    for (int i = 0; i < DATA_SIZE * DATA_SIZE; i++) {
//        source_cpu_results[i] = 0;
//        source_fpga_results[i] = 0;
//    }

    et.add("OpenCL Initialization");
    cl::CommandQueue q;
    cl::Context context;
    cl::Kernel kernel;
    cl_int err;
    // Initialize fpga
    //init_fpga(q, context, kernel, err, size);

    // The get_xil_devices will return vector of Xilinx Devices
    auto devices = xcl::get_xil_devices();
    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));

        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err, kernel = cl::Kernel(program, "mmult", &err));
            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

    et.finish();

    bool match = true;
    std::ofstream target_file; //[K] defining target file
    target_file.open("key_exe_times.csv");
    target_file << "Description,Value,Unit\n";
    for (int k = 10; k <= 639; k += 31){
        //Printing buffer sizes to file
        target_file << "Buffer size" << ",";
        target_file << std::to_string(k) << "\n";
        for (int i = 0; i < DATA_SIZE * DATA_SIZE; i++) {
            source_cpu_results[i] = 0;
            source_fpga_results[i] = 0;
        }
        // Compute CPU Results
        et.add("Software Run");
        mmult_cpu(source_in1.data(), source_in2.data(), source_cpu_results.data(), k);
        et.finish();

        // Compute FPGA Results
        et = mmult_fpga(source_in1, source_in2, source_fpga_results, q, context, kernel, err, k, et);

        // Compare the results of FPGA to CPU
        for (int i = 0; i < k * k; i++) {
            if (source_fpga_results[i] != source_cpu_results[i]) {
                std::cout << "Error: Result mismatch" << std::endl;
                std::cout << "i = " << i << " CPU result = " << source_cpu_results[i]
                          << " FPGA result = " << source_fpga_results[i] << std::endl;
                match = false;
                break;
            }
        }

        std::cout << "Iteration = " << k << " TEST " << (match ? "PASSED" : "FAILED") << std::endl;
    }
    target_file.close();
    et.print(-1, std::to_string(size), "key_exe_times", "./");
    return (match ? EXIT_SUCCESS : EXIT_FAILURE);

}
