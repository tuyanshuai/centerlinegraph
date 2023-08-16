clear;
bw=niftiread("label_big_big_small_colon.nii")>0;
radius = 4;
se = strel("disk",radius);
bw = imdilate(bw, se);

bw1=endpoints(padarray(bw,[1,0,0],0,"both"));

bw2=padarray(bw,[0,1,0],0,"both");
bw2=permute(bw2,[2,1,3]);
bw2=endpoints(bw2);
bw2=ipermute(bw2,[2,1,3]);
niftiwrite(uint8(bw1),"matlabbw1.nii");

bw3=padarray(bw,[0,0,1],0,"both");
bw3=permute(bw3,[3,1,2]);
bw3=endpoints(bw3);
bw3=ipermute(bw3,[3,1,2]);
niftiwrite(uint8(bw2),"matlabbw2.nii");
niftiwrite(uint8(bw3),"matlabbw3.nii");
bw4=cat(4,bw1,bw2,bw3);
bw4=sum(bw4,4)>1;
niftiwrite(uint8(bw4),"matlab_line.nii");
  

function maxIndices=endpoints(bw)
sz=size(bw);
bw=reshape(bw,[],1);
dis=bwdist(~bw,"cityblock");
maxIndices = islocalmax(dis);
maxIndices=reshape(maxIndices,sz);
maxIndices=maxIndices(2:end-1,:,:);
end
