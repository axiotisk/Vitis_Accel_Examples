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

////////////////////////////////////////////////////////////////////////////////
// Description: Basic Adder, no overflow. Unsigned. Combinatorial.
////////////////////////////////////////////////////////////////////////////////

//`default_nettype none
`timescale 1 ns / 1 ps 

module hls4ml_rtl_wrapper #(
  parameter integer C_DATA_WIDTH   = 32, // Data width of both input and output data
  parameter integer C_NUM_CHANNELS = 2   // Number of input channels.  Only a value of 2 implemented.
)
(
  input wire                                         aclk,
  input wire                                         areset,

  input wire  [C_NUM_CHANNELS-1:0]                   s_tvalid,
  input wire  [C_NUM_CHANNELS-1:0][C_DATA_WIDTH-1:0] s_tdata,
  output wire [C_NUM_CHANNELS-1:0]                   s_tready,

  output wire                                        m_tvalid,
  output wire [C_DATA_WIDTH-1:0]                     m_tdata,
  input  wire                                        m_tready

);

//timeunit 1ps; 
//timeprecision 1ps; 

//`timescale 1ns/1ps //From krnl_vadd_rtl_control_s_axi.v

wire [15:0] r_out_0;
wire [15:0] r_out_1;
wire [15:0] r_out_2;
wire [15:0] r_out_3;
wire [15:0] r_out_4;

wire [15:0] w_out;

wire r_v_out_0;
wire r_v_out_1;
wire r_v_out_2;
wire r_v_out_3;
wire r_v_out_4;

wire r_ap_idle;
wire r_ap_done;

wire w_v_out;

assign w_v_out = r_v_out_0 | r_v_out_1 | r_v_out_2 | r_v_out_3 | r_v_out_4;
assign m_tvalid = w_v_out;
assign w_out = {r_out_0, r_out_1} | {r_out_2, r_out_3} | {r_out_4, {14'b11111111111111, r_ap_idle, r_ap_done}};

assign m_tdata = w_out;

myproject inst_myproject (
  .ap_clk (aclk),
  .ap_rst (areset),
  .ap_start (m_tready),
  .ap_done (r_ap_done),
  .ap_idle (r_ap_idle),
  .ap_ready (s_tready),
  .input_1_V_ap_vld (&s_tvalid[0]),
  .input_1_V ({s_tdata[1], s_tdata[0], s_tdata[0], s_tdata[0], s_tdata[0], s_tdata[0], s_tdata[0], s_tdata[0], s_tdata[0], s_tdata[0]}),
  .layer9_out_0_V (r_out_0),
  .layer9_out_0_V_ap_vld (r_v_out_0),
  .layer9_out_1_V (r_out_1),
  .layer9_out_1_V_ap_vld (r_v_out_1),
  .layer9_out_2_V (r_out_2),
  .layer9_out_2_V_ap_vld (r_v_out_2),
  .layer9_out_3_V (r_out_3),
  .layer9_out_3_V_ap_vld (r_v_out_3),
  .layer9_out_4_V (r_out_4),
  .layer9_out_4_V_ap_vld (r_v_out_4),
  .const_size_in_1 (),
  .const_size_in_1_ap_vld (),
  .const_size_out_1 (),
  .const_size_out_1_ap_vld ()
);

endmodule : hls4ml_rtl_wrapper

