function plot_center_line_segs(G,V)

 plot3(V(:,1),V(:,2),V(:,3),'r.');axis equal; hold on;

[r,c]=find(G);
plot3([V(r,1)'; V(c,1)'], [V(r,2)'; V(c,2)'],[V(r,3)'; V(c,3)'],'b-','LineWidth',2)      % A bunch of separate lines;


end