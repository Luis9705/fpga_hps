module LCD_control (
	input 		clk,
	input			rst,
	input			start,
	output[7:0] data,
	output      rw,
	output		rs,
	output		en
	);
	
	

	
reg [25:0] counter; 
reg  [7:0]data_count;

always @(posedge clk)
begin
if(counter==24999999) begin
   counter<=0;
   data_count<=data_count+1'b1;
	if(data_count == 9)
		data_count<= 0;
    end
else begin
   counter<=counter+1'b1;
end
end


assign data = data_count;
assign rw = 0;
assign rs = 0;
assign en = 0;
	
	
endmodule
