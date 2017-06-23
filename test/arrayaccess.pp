program
var a : array of integer;
    b : array of array of integer;
    c : array of boolean;
begin
    a := new array of integer[3];
    a[2] := 2;
    b := new array of array of integer[5];
    c := new array of boolean[3];
    writeln(a[2])
end.
