clear all;
close all;

%im1=load("image_stat.txt");
im1=load("image.txt");

figure(1)
imagesc(im1);

[npoint, nline] = size(im1);

npoint = npoint - 1;

fe = 125;
f=linspace(0, (npoint-1)/npoint*fe, npoint);
df = f(2)-f(1);

fmin = 1;
fmax = 6;
nmin = floor(fmin/df);
nmax = floor(fmax/df);

im2 = zeros(npoint,nline);
for j=1:nline
  y=fft(im1(2:end,j));
  y(1:nmin) = 0;
  y(nmax+1:end) = 0;
  im2(:,j) = abs(ifft(y));
endfor

figure(2)
surf(im2);
shading interp;
view([0 90]);
axis off;
colormap gray;

figure(3)
plot(f,im1(2:end,28)-mean(im1(2:end,28)),f,2*im2(:,28));