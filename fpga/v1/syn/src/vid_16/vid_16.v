//
// vid_16.v -- video controller for 1024x768, 16bpp screen
//


`timescale 1ns / 1ps
`default_nettype none


module vid_16(pclk, clk, rst,
              stb, we, addr, data_in,
              hsync, vsync, pxclk,
              sync_n, blank_n, r, g, b);
    // internal interface
    input pclk;
    input clk;
    input rst;
    input stb;
    input we;
    input [18:0] addr;
    input [31:0] data_in;
    // external SRAM interface
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
  // processor interface
  //----------------------------

  //----------------------------
  // monitor interface
  //----------------------------

  // stage 1: timing generator

  reg [10:0] hcount_1;
  reg hblnk_1;
  reg hsync_1;

  reg [9:0] vcount_1;
  reg vblnk_1;
  reg vsync_1;

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

  // stage 2: video memory access

  reg [14:0] viddat_2;
  reg hblnk_2;
  reg hsync_2;
  reg vblnk_2;
  reg vsync_2;

  always @(posedge clk) begin
    viddat_2[14:0] <= { 5'b10000, 5'b11011, 5'b11011 };
    viddat_2[14:0] <= { hcount_1[4:2], 2'b00,
                        vcount_1[4:2], 2'b00,
                        hcount_1[7:5], 2'b00 };
  end

  always @(posedge pclk) begin
    hblnk_2 <= hblnk_1;
    hsync_2 <= hsync_1;
    vblnk_2 <= vblnk_1;
    vsync_2 <= vsync_1;
  end

  // hsync and vsync are directly connected to the monitor

  always @(posedge pclk) begin
    hsync <= ~hsync_2;
    vsync <= ~vsync_2;
  end

  // all other signals are passed through the registered DAC

  assign pxclk = pclk;
  assign sync_n = 1'b0;
  assign blank_n = ~hblnk_2 & ~vblnk_2;
  assign r[7:0] = ~blank_n ? 8'h00 : { viddat_2[14:10], 3'b000 };
  assign g[7:0] = ~blank_n ? 8'h00 : { viddat_2[ 9: 5], 3'b000 };
  assign b[7:0] = ~blank_n ? 8'h00 : { viddat_2[ 4: 0], 3'b000 };

endmodule
