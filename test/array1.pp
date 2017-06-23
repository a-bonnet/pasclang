program
var array1 : array of integer;
    array2 : array of boolean;
    iter : integer;

begin
    array1 := new array of integer[10];
    array2 := new array of boolean[10];
    iter := 0;
    while iter < 10 do
        begin
           array1[iter] := iter * 4 + 2;
           if (iter / 2 * 2 = iter) then
               array2[iter] := true;
           iter := iter + 1
        end;
    iter := 0;
    while iter < 10 do
        begin
            if array2[iter] then
                writeln(array1[iter]);
            iter := iter + 1
        end
end.
