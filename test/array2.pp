program
var matrix : array of array of integer;
    i, j, n : integer;
begin
    n := readln() ;
    matrix := new array of array of integer[n];
    i := 0;
    while i < n do
    begin
        matrix[i] := new array of integer[n];
        i := i + 1
    end;
    i := 0;
    while i < n do
        begin
            j := 0;
            while j < n do
            begin
                matrix[i][j] := i * j;
                j :=  j + 1
            end;
            i := i + 1
        end;
    i := 0;
    j := 0;
    while i < n do
        begin
            j := 0;
            while j < n do
            begin
                writeln(matrix[i][j]);
                j :=  j+1
            end;
            i := i + 1
        end
end.
