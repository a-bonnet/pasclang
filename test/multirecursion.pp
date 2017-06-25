program
    var n : integer;

function fib(n : integer) : integer;
begin
    if n = 0 then
        fib := 0
    else if n = 1 then
        fib := 1
    else
        fib := fib1(n) + fib2(n)
end;

function fib1(n : integer) : integer;
begin
    n := n - 1;
    if n = 0 then
        fib1 := 0
    else if n = 1 then
        fib1 := 1
    else fib1 := fib1(n) + fib2(n)
end;

function fib2(n : integer) : integer;
begin
    n := n - 2;
    if n = 0 then
        fib2 := 0
    else if n = 1 then
        fib2 := 1
    else fib2 := fib1(n) + fib2(n)
end;

begin
    n := 1;
    while n <> 0 do
        begin
            n := readln();
            writeln(fib(n))
        end
end.

