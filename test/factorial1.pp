program
var
    a : integer;
function factorial(n : integer) : integer;
var
    i : integer;
begin
    factorial := 1 ;
    i := 1 ;
    while i <= n do
    begin
        factorial := i * factorial ;
        i := i + 1
    end
end;
begin
    a := 1;
    while (a > 0) do
        begin
            a := readln();
            writeln(factorial(a))
        end
end.
