//module my_dut(
//  input logic [31:0] data_in,
//  output logic [31:0] data_out
//);
//  // DUT implementation here
//endmodule

`timescale 1ns / 1ps

module tb;
  // Declare signals for testbench
  logic clk;
  logic rst_n;
  logic [31:0] data_in = 32'hAAAAAAAA;
  logic [31:0] data_out;

  assign data_out = data_in;

// Read data from file
  integer file;
  initial begin
    $display("Opening file");
    file = $fopen("/home/kaxiotis/Vitis_Accel_Examples/rtl_kernels/rtl_hls4ml_scrip_wrapper/src/krnl_vadd/sim/input_data.txt", "r");
    if (file == 0) begin
      $display("Error: Could not open file");
      //$finish;
    end

    // Read data and apply to data_in signal
    repeat(5) begin
      $fscanf(file, "%d", data_in);
      #1; // Advance time for one clock cycle
    end

    $fclose(file);
  end

//  // Instantiate DUT
//  my_dut dut(
//    .data_in(data_in),
//    .data_out(data_out)
//  );
//
//   inst_dut #( 
//     .c_s_axi_control_data_width (),
//     .c_s_axi_control_addr_width (),
//     .c_m_axi_gmem_id_width      (),
//     .c_m_axi_gmem_addr_width    (),
//     .c_m_axi_gmem_data_width    ()
//   )
//  
//  krnl_vadd_rtl_int(
//    // system signals
//    .ap_clk(),
//    .ap_rst_n(),
//    // axi4 master interface 
//    .m_axi_gmem_awvalid(),
//    .m_axi_gmem_awready(),
//    .m_axi_gmem_awaddr(),
//    .m_axi_gmem_awid(),
//    .m_axi_gmem_awlen(),
//    .m_axi_gmem_awsize(),
//    // tie-off axi4 transaction options that are not being used.
//    .m_axi_gmem_awburst(),
//    .m_axi_gmem_awlock(),
//    .m_axi_gmem_awcache(),
//    .m_axi_gmem_awprot(),
//    .m_axi_gmem_awqos(),
//    .m_axi_gmem_awregion(),
//    .m_axi_gmem_wvalid(),
//    .m_axi_gmem_wready(),
//    .m_axi_gmem_wdata(),
//    .m_axi_gmem_wstrb(),
//    .m_axi_gmem_wlast(),
//    .m_axi_gmem_arvalid(),
//    .m_axi_gmem_arready(),
//    .m_axi_gmem_araddr(),
//    .m_axi_gmem_arid(),
//    .m_axi_gmem_arlen(),
//    .m_axi_gmem_arsize(),
//    .m_axi_gmem_arburst(),
//    .m_axi_gmem_arlock(),
//    .m_axi_gmem_arcache(),
//    .m_axi_gmem_arprot(),
//    .m_axi_gmem_arqos(),
//    .m_axi_gmem_arregion(),
//    .m_axi_gmem_rvalid(),
//    .m_axi_gmem_rready(),
//    .m_axi_gmem_rdata(),
//    .m_axi_gmem_rlast(),
//    .m_axi_gmem_rid(),
//    .m_axi_gmem_rresp(),
//    .m_axi_gmem_bvalid(),
//    .m_axi_gmem_bready(),
//    .m_axi_gmem_bresp(),
//    .m_axi_gmem_bid(),
//  
//    // AXI4-Lite slave interface
//    .s_axi_control_AWVALID(),
//    .s_axi_control_AWREADY(),
//    .s_axi_control_AWADDR(),
//    .s_axi_control_WVALID(),
//    .s_axi_control_WREADY(),
//    .s_axi_control_WDATA(),
//    .s_axi_control_WSTRB(),
//    .s_axi_control_ARVALID(),
//    .s_axi_control_ARREADY(),
//    .s_axi_control_ARADDR(),
//    .s_axi_control_RVALID(),
//    .s_axi_control_RREADY(),
//    .s_axi_control_RDATA(),
//    .s_axi_control_RRESP(),
//    .s_axi_control_BVALID(),
//    .s_axi_control_BREADY(),
//    .s_axi_control_BRESP(),
//    .interrupt()
//  );

  // Clock and reset generation
  always #5 clk = ~clk;

  initial begin
    clk = 0;
    rst_n = 0;
    #10;
    rst_n = 1;
  end

  // Monitor data_out signal
  always @(posedge clk) begin
    $display("data_out = %d", data_out);
  end
endmodule
