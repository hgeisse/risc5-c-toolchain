//
// hcv.v -- video controller for a 1024x768 high color screen
//


`timescale 1ns / 1ps
`default_nettype none


module hcv(pclk, clk, rst,
           stb, we, addr,
           data_in, data_out, ack,
           sram_addr, sram_dq, sram_ce_n,
           sram_oe_n, sram_we_n,
           sram_ub_n, sram_lb_n,
           hsync, vsync, pxclk,
           sync_n, blank_n, r, g, b);
    // internal interface
    input pclk;
    input clk;
    input rst;
    input stb;
    input we;
    input [19:0] addr;
    input [31:0] data_in;
    output [31:0] data_out;
    output ack;
    // external SRAM interface
    output [19:0] sram_addr;
    inout [15:0] sram_dq;
    output sram_ce_n;
    output sram_oe_n;
    output sram_we_n;
    output sram_ub_n;
    output sram_lb_n;
    // external video interface
    output reg hsync;
    output reg vsync;
    output pxclk;
    output sync_n;
    output blank_n;
    output [7:0] r;
    output [7:0] g;
    output [7:0] b;

  //----------------------------
  // bus interface
  //----------------------------

  wire fb_bus_stb;
  wire fb_bus_we;
  wire [19:0] fb_bus_addr;
  wire [15:0] fb_bus_data_wr;
  reg [15:0] fb_bus_data_rd;
  reg fb_bus_ack;

  assign fb_bus_stb = stb;
  assign fb_bus_we = we;
  assign fb_bus_addr[19:0] = addr[19:0];
  assign fb_bus_data_wr[15:0] = data_in[15:0];

  assign data_out[31:0] = { 16'h0000, fb_bus_data_rd[15:0] };
  assign ack = fb_bus_ack;

  //----------------------------
  // frame buffer control
  //----------------------------

  // state transitions

  reg [3:0] fb_state;
  reg [3:0] fb_next;

  always @(posedge pclk) begin
    if (rst) begin
      fb_state[3:0] <= 4'b0000;
    end else begin
      fb_state[3:0] <= fb_next[3:0];
    end
  end

  // next-state and standard output functions

  reg fb_addr_select;
  reg fb_addr_latch;
  reg fb_data_wr_latch;
  reg fb_data_rd_latch;

  always @(*) begin
    case (fb_state[3:0])
      4'b0000:  // idle
        begin
          if (~fb_bus_stb) begin
            fb_next[3:0] = 4'b0000;
            fb_addr_select = 1'b0;
            fb_addr_latch = 1'b1;
            fb_data_wr_latch = 1'b0;
            fb_data_rd_latch = 1'b0;
            fb_bus_ack = 1'b0;
          end else begin
            if (fb_bus_we) begin
              fb_next[3:0] = 4'b0001;
              fb_addr_select = 1'b1;
              fb_addr_latch = 1'b1;
              fb_data_wr_latch = 1'b1;
              fb_data_rd_latch = 1'b0;
              fb_bus_ack = 1'b0;
            end else begin
              fb_next[3:0] = 4'b0101;
              fb_addr_select = 1'b1;
              fb_addr_latch = 1'b1;
              fb_data_wr_latch = 1'b0;
              fb_data_rd_latch = 1'b0;
              fb_bus_ack = 1'b0;
            end
          end
        end
      4'b0001:  // write 1
        begin
          fb_next[3:0] = 4'b0010;
          fb_addr_select = 1'b0;
          fb_addr_latch = 1'b0;
          fb_data_wr_latch = 1'b0;
          fb_data_rd_latch = 1'b0;
          fb_bus_ack = 1'b0;
        end
      4'b0010:  // write 2
        begin
          fb_next[3:0] = 4'b0011;
          fb_addr_select = 1'b0;
          fb_addr_latch = 1'b0;
          fb_data_wr_latch = 1'b0;
          fb_data_rd_latch = 1'b0;
          fb_bus_ack = 1'b0;
        end
      4'b0011:  // write 3
        begin
          fb_next[3:0] = 4'b0100;
          fb_addr_select = 1'b0;
          fb_addr_latch = 1'b1;
          fb_data_wr_latch = 1'b0;
          fb_data_rd_latch = 1'b0;
          fb_bus_ack = 1'b1;
        end
      4'b0100:  // write 4
        begin
          fb_next[3:0] = 4'b0000;
          fb_addr_select = 1'b0;
          fb_addr_latch = 1'b1;
          fb_data_wr_latch = 1'b0;
          fb_data_rd_latch = 1'b0;
          fb_bus_ack = 1'b1;
        end
      4'b0101:  // read 1
        begin
          fb_next[3:0] = 4'b0110;
          fb_addr_select = 1'b0;
          fb_addr_latch = 1'b1;
          fb_data_wr_latch = 1'b0;
          fb_data_rd_latch = 1'b0;
          fb_bus_ack = 1'b0;
        end
      4'b0110:  // read 2
        begin
          fb_next[3:0] = 4'b0111;
          fb_addr_select = 1'b0;
          fb_addr_latch = 1'b1;
          fb_data_wr_latch = 1'b0;
          fb_data_rd_latch = 1'b1;
          fb_bus_ack = 1'b0;
        end
      4'b0111:  // read 3
        begin
          fb_next[3:0] = 4'b1000;
          fb_addr_select = 1'b0;
          fb_addr_latch = 1'b1;
          fb_data_wr_latch = 1'b0;
          fb_data_rd_latch = 1'b0;
          fb_bus_ack = 1'b1;
        end
      4'b1000:  // read 4
        begin
          fb_next[3:0] = 4'b0000;
          fb_addr_select = 1'b0;
          fb_addr_latch = 1'b1;
          fb_data_wr_latch = 1'b0;
          fb_data_rd_latch = 1'b0;
          fb_bus_ack = 1'b1;
        end
      default:  // never reached
        begin
          fb_next[3:0] = 4'b0000;
          fb_addr_select = 1'b0;
          fb_addr_latch = 1'b0;
          fb_data_wr_latch = 1'b0;
          fb_data_rd_latch = 1'b0;
          fb_bus_ack = 1'b0;
        end
    endcase
  end

  // look-ahead output functions

  reg fb_oe_la;
  reg fb_we_la;
  reg fb_dqe_la;

  always @(*) begin
    case (fb_next[3:0])
      4'b0000:  // idle
        begin
          fb_oe_la = 1'b1;
          fb_we_la = 1'b0;
          fb_dqe_la = 1'b0;
        end
      4'b0001:  // write 1
        begin
          fb_oe_la = 1'b0;
          fb_we_la = 1'b0;
          fb_dqe_la = 1'b0;
        end
      4'b0010:  // write 2
        begin
          fb_oe_la = 1'b0;
          fb_we_la = 1'b1;
          fb_dqe_la = 1'b1;
        end
      4'b0011:  // write 3
        begin
          fb_oe_la = 1'b0;
          fb_we_la = 1'b0;
          fb_dqe_la = 1'b1;
        end
      4'b0100:  // write 4
        begin
          fb_oe_la = 1'b0;
          fb_we_la = 1'b0;
          fb_dqe_la = 1'b0;
        end
      4'b0101:  // read 1
        begin
          fb_oe_la = 1'b1;
          fb_we_la = 1'b0;
          fb_dqe_la = 1'b0;
        end
      4'b0110:  // read 2
        begin
          fb_oe_la = 1'b1;
          fb_we_la = 1'b0;
          fb_dqe_la = 1'b0;
        end
      4'b0111:  // read 3
        begin
          fb_oe_la = 1'b1;
          fb_we_la = 1'b0;
          fb_dqe_la = 1'b0;
        end
      4'b1000:  // read 4
        begin
          fb_oe_la = 1'b1;
          fb_we_la = 1'b0;
          fb_dqe_la = 1'b0;
        end
      default:  // never reached
        begin
          fb_oe_la = 1'b0;
          fb_we_la = 1'b0;
          fb_dqe_la = 1'b0;
        end
    endcase
  end

  reg fb_oe_n;
  reg fb_we_n;
  reg fb_dqe;

  always @(posedge pclk) begin
    fb_oe_n <= ~fb_oe_la;
    fb_we_n <= ~fb_we_la;
    fb_dqe <= fb_dqe_la;
  end

  //----------------------------
  // monitor interface
  //----------------------------

  // stage 1: timing generator

  reg [10:0] hcount_1;
  reg hblnk_1;
  reg hsync_1;

  always @(posedge pclk) begin
    if (rst) begin
      hcount_1 <= 11'd0;
      hblnk_1 <= 1'b0;
      hsync_1 <= 1'b0;
    end else begin
      if (hcount_1 == 11'd1327) begin
        hcount_1 <= 11'd0;
        hblnk_1 <= 1'b0;
      end else begin
        hcount_1 <= hcount_1 + 11'd1;
      end
      if (hcount_1 == 11'd1023) begin
        hblnk_1 <= 1'b1;
      end
      if (hcount_1 == 11'd1047) begin
        hsync_1 <= 1'b1;
      end
      if (hcount_1 == 11'd1183) begin
        hsync_1 <= 1'b0;
      end
    end
  end

  reg [9:0] vcount_1;
  reg vblnk_1;
  reg vsync_1;

  always @(posedge pclk) begin
    if (rst) begin
      vcount_1 <= 10'd0;
      vblnk_1 <= 1'b0;
      vsync_1 <= 1'b0;
    end else begin
      if (hcount_1 == 11'd1327) begin
        if (vcount_1 == 10'd805) begin
          vcount_1 <= 10'd0;
          vblnk_1 <= 1'b0;
        end else begin
          vcount_1 <= vcount_1 + 10'd1;
        end
        if (vcount_1 == 10'd767) begin
          vblnk_1 <= 1'b1;
        end
        if (vcount_1 == 10'd770) begin
          vsync_1 <= 1'b1;
        end
        if (vcount_1 == 10'd776) begin
          vsync_1 <= 1'b0;
        end
      end
    end
  end

  wire [19:0] fb_pix_addr_1;
  wire [19:0] fb_addr_1;

  assign fb_pix_addr_1[19:0] = { vcount_1[9:0], hcount_1[9:0] };
  assign fb_addr_1[19:0] =
    fb_addr_select ? fb_bus_addr[19:0] : fb_pix_addr_1[19:0];

  // stage 2: video memory access

  reg hsync_2;
  reg vsync_2;
  reg blank_2;

  always @(posedge pclk) begin
    hsync_2 <= hsync_1;
    vsync_2 <= vsync_1;
    blank_2 <= hblnk_1 | vblnk_1;
  end

  reg [19:0] fb_addr_2;
  reg [15:0] fb_data_wr_2;
  wire [15:0] fb_data_rd_2;

  always @(posedge pclk) begin
    if (fb_addr_latch) begin
      fb_addr_2[19:0] <= fb_addr_1[19:0];
    end
  end

  always @(posedge pclk) begin
    if (fb_data_wr_latch) begin
      fb_data_wr_2[15:0] <= fb_bus_data_wr[15:0];
    end
  end

  assign sram_addr[19:0] = fb_addr_2[19:0];
  assign sram_dq[15:0] = fb_dqe ? fb_data_wr_2[15:0] : 16'hzzzz;

  assign sram_oe_n = fb_oe_n;
  assign sram_we_n = fb_we_n;
  assign sram_ce_n = 1'b0;
  assign sram_ub_n = 1'b0;
  assign sram_lb_n = 1'b0;

  assign fb_data_rd_2[15:0] = sram_dq[15:0];

  // stage 3: video memory data latch

  reg hsync_3;
  reg vsync_3;
  reg blank_3;

  always @(posedge pclk) begin
    hsync_3 <= hsync_2;
    vsync_3 <= vsync_2;
    blank_3 <= blank_2;
  end

  reg [15:0] fb_data_rd_3;

  // 13.333 ns (75 MHz) is not enough to access the SRAM on the board
  // (address leaving FPGA + SRAM access time + data entering FPGA).
  // Using the negative clock edge for latching the data does the
  // trick: it stretches the data window to 1.5 clock cycles (20 ns).
  // Consequently, this stage must be followed by a data resync stage.
  always @(negedge pclk) begin
    fb_data_rd_3[15:0] <= fb_data_rd_2[15:0];
  end

  // stage 4: data resync

  reg hsync_4;
  reg vsync_4;
  reg blank_4;

  always @(posedge pclk) begin
    hsync_4 <= hsync_3;
    vsync_4 <= vsync_3;
    blank_4 <= blank_3;
  end

  always @(posedge pclk) begin
    if (fb_data_rd_latch) begin
      fb_bus_data_rd[15:0] <= fb_data_rd_3[15:0];
    end
  end

  reg [15:0] fb_data_rd_4;

  always @(posedge pclk) begin
    if (~fb_data_rd_latch) begin
      fb_data_rd_4[15:0] <= fb_data_rd_3[15:0];
    end
  end

  // stage 5: pixel stage for sync signals
  // these are not buffered externally and thus must be buffered here

  always @(posedge pclk) begin
    hsync <= ~hsync_4;
    vsync <= ~vsync_4;
  end

  // stage 5: pixel stage for color signals
  // these are buffered by the DAC and thus must not be buffered here

  assign pxclk = pclk;

  assign sync_n = 1'b0;
  assign blank_n = ~blank_4;

  assign r[7:0] = { fb_data_rd_4[14:10], 3'b000 };
  assign g[7:0] = { fb_data_rd_4[ 9: 5], 3'b000 };
  assign b[7:0] = { fb_data_rd_4[ 4: 0], 3'b000 };

endmodule
