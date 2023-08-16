step1
 
id=find(bw4(:));
[x, y, z]= ind2sub(size(bw4),id);

 


%% generate graph
pt = [x y z];
% a graph fully connected
dis_mat = squareform(pdist(pt));
funbw =  griddedInterpolant(double(bw));
 

[val,sid] = sort(dis_mat);
 
N = size(dis_mat,1);
candi = ones(1,N);
G = dis_mat*0;
for i=1:N
    % loop over minmal distance
    if(candi(i)==0)
        continue
    end
       

    p1 = pt(i,:);
    for j=1:N
        sj = sid(j,i);
        if(dis_mat(sj,i)==0)
            continue % self point
        end

        if(dis_mat(sj,i)>100)
            break; % too far;
        end

        if(candi(sj)  )
            p2 = pt(sj,:);
            if(is_connected(funbw,p1,p2))
                candi(sj) = 0;
                candi(i) = 0;
                G(i,sj) = 1;
                G(sj, i) = 1;
                break; 
            end            
        end
    end

    fprintf('%d\n',i);
end
 %% show 
  s = isosurface(bw);
 
figure;
p= patch(s); hold on;
alpha(0.3)
plot3(y,x,z,'r.');  
set(p,'FaceColor',[0.5 1 0.5]);  
set(p,'EdgeColor','none');
camlight;
lighting gouraud;
axis equal;axis off;
 plot_center_line_segs(G,pt(:,[2 1 3]))

