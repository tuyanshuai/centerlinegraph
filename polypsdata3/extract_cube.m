% function cubes = extract_cube(fn)
fn = '1.3.6.1.4.1.9328.50.4.0011 prone';


dirinfo = dir(fn);

for i=1:length(dirinfo)

    fn2 = dirinfo(i).name;

    if(fn2(1)=='.')
        continue
    else        
        break
    end


end


ctcfolder = [fn '/' fn2 '/ctc'];

polypsfn = [ctcfolder '/polyps.txt' ];

text = fileread(polypsfn);


expression = '\[\d+.\d+ \d+.\d+ \d+.\d+\]'; 
matchStr = regexp(text,expression,'match');
 
for i=1:length(matchStr)
    detectedPolysLocs(i,:) = str2num(matchStr{i}(2:end-1)); 
end


detectedPolysLocs 







% end