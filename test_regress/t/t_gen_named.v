// DESCRIPTION: Verilator: Verilog Test module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty.

interface intf
#(parameter PARAM = 0)
();
   logic val;
endinterface

module t1(intf mod_intf);
   initial begin
      $display("%d", mod_intf.val);
   end
endmodule

module t();
   generate

      if(1) begin // : Test
         intf #(.PARAM(1)) my_intf;
         t1 t (.mod_intf(my_intf));
         initial begin
            $write("*-* All Finished *-*\n");
            $finish;
         end
      end
   endgenerate
endmodule
