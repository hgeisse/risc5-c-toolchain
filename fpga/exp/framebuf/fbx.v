//
// fbx.v -- frame buffer experiment
//


`timescale 1ns/1ps
`default_nettype none


//
// test bench
//

module fbx;

  reg clk_in;			// clock, input, 50 MHz
  reg rst_in_n;			// reset, input, active low

  wire clk_ok;
  wire mclk;
  wire pclk;
  wire clk;
  wire rst;

  reg [11:0] addr;
  reg [7:0] data;
  reg wr;

  //
  // simulation control
  //

  initial begin
    #0          $timeformat(-9, 1, " ns", 12);
                $dumpfile("dump.vcd");
                $dumpvars(0, fbx);
                clk_in = 1;
                rst_in_n = 0;
    #145        rst_in_n = 1;
                wr = 0;
    #377        addr = 12'h123;
                data = 8'hab;
                wr = 1;
    #20         addr = 12'hx;
                data = 8'hx;
                wr = 0;
    #80         addr = 12'h789;
                data = 8'hef;
                wr = 1;
    #20         addr = 12'hx;
                data = 8'hx;
                wr = 0;
    #80         addr = 12'h123;
                data = 8'hab;
                wr = 1;
    #80         addr = 12'hx;
                data = 8'hx;
                wr = 0;
    #100        addr = 12'h789;
                data = 8'hef;
                wr = 1;
    #80         addr = 12'hx;
                data = 8'hx;
                wr = 0;
    #3000       $finish;
  end

  //
  // clock and reset generator
  //

  always begin
    #10 clk_in = ~clk_in;	// 20 nsec cycle time
  end

  clk_rst clk_rst_0(
    .clk_in(clk_in),
    .rst_in_n(rst_in_n),
    .clk_ok(clk_ok),
    .clk_100(mclk),
    .clk_75(pclk),
    .clk_50(clk),
    .rst(rst)
  );

  //
  // frame buffer access
  //

  reg [11:0] addr_latch;
  reg [7:0] data_latch;
  reg wr_latch_src;
  reg [1:0] state;
  wire wr_dst;

  always @(posedge clk) begin
    if (wr) begin
      addr_latch <= addr;
      data_latch <= data;
    end
    wr_latch_src <= wr;
  end

  always @(posedge pclk) begin
    if (rst) begin
      state <= 2'b00;
    end else begin
      case (state)
        2'b00:
          state <= ~wr_latch_src ? 2'b00 : 2'b01;
        2'b01:
          state <= 2'b10;
        2'b10:
          state <= ~wr_latch_src ? 2'b00 : 2'b10;
        default:
          state <= 2'b00;
      endcase
    end
  end

  assign wr_dst = (state == 2'b01);

endmodule


//
// clock and reset generator
//

module clk_rst(clk_in, rst_in_n,
               clk_ok, clk_100, clk_75, clk_50, rst);
    input clk_in;
    input rst_in_n;
    output clk_ok;
    output reg clk_100;
    output reg clk_75;
    output clk_50;
    output rst;

  reg clk_25;
  reg rst_p_n;
  reg rst_s_n;
  reg [3:0] rst_counter;
  wire rst_counting;

  assign clk_ok = 1'b1;

  always @(posedge clk_in) begin
    clk_100 = 1'b1;
    #5 clk_100 = 1'b0;
    #5 clk_100 = 1'b1;
    #5 clk_100 = 1'b0;
  end

  initial begin
    clk_25 = 1'b0;
  end

  always @(posedge clk_in) begin
    clk_25 = ~clk_25;
  end

  always @(posedge clk_25) begin
    clk_75 = 1'b1;
    #6.66 clk_75 = 1'b0;
    #6.67 clk_75 = 1'b1;
    #6.67 clk_75 = 1'b0;
    #6.67 clk_75 = 1'b1;
    #6.66 clk_75 = 1'b0;
  end

  assign clk_50 = clk_in;

  always @(posedge clk_50) begin
    rst_p_n <= rst_in_n;
    rst_s_n <= rst_p_n;
    if (~rst_s_n | ~clk_ok) begin
      rst_counter[3:0] <= 4'h0;
    end else begin
      if (rst_counting) begin
        rst_counter[3:0] <= rst_counter[3:0] + 4'h1;
      end
    end
  end

  assign rst_counting =
    (rst_counter[3:0] == 4'hF) ? 1'b0 : 1'b1;
  assign rst = rst_counting;

endmodule
