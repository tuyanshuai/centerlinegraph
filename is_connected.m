function flag = is_connected(fun,p1,p2)

dis = norm(p1-p2);
xs = linspace(p1(1),p2(1),round(dis)+1);
ys = linspace(p1(2),p2(2),round(dis)+1);
zs = linspace(p1(3),p2(3),round(dis)+1);

for i=1:length(xs)
    if(fun(xs(i),ys(i),zs(i)) == 0 )
        flag = 0;
        return;
    end
end

flag =1;

end