x=0:2*pi/4096:2*pi-2*pi/4096;
y=round(32768*(1+sin(x)));
for x=1:4096
    if(y(x)==65536)
        y(x)=65535;
    end
end
plot(y);

file = fopen('Sin.mif','wt');
fprintf(file, 'depth = %d;\n',4096);
fprintf(file, 'width = %d;\n',16);
fprintf(file, 'address_radix = UNS;\n');
fprintf(file,'data_radix = UNS;\n');
fprintf(file,'content begin\n');
for x=1: 4096
    fprintf(file,'%d:%d;\n',x-1,y(x));
end
fprintf(file, 'end;');
fclose(file);