%% load a sample image; convert to grayscale; convert to binary
clear
close all

%% nacitanie obrazku
image1 = imread('ciara2.bmp');
image =im2bw(image1);

%% najdenie kontur
image = bwperim(image,8);
[x,y]=size(image);   % zisti velkost matice I
figure ,imshow(image)
figure, imshow(image1)


theta_sample_frequency = 0.01;                                             
[x, y] = size(image);
rho_limit = norm([x y]);                                                
rho = (-rho_limit:1:rho_limit);
theta = (0:theta_sample_frequency:pi);
num_thetas = numel(theta);
num_rhos = numel(rho);
hough_space = zeros(num_rhos, num_thetas);

%% hughova transformacia
for xi = 1:x
    for yj = 1:y
        if image(xi, yj) == 1 
            for theta_index = 1:num_thetas
                THETA = theta(theta_index);
                r  = xi * cos(THETA) + yj * sin(THETA);
                rho_index = round(r + num_rhos/2);                      
                hough_space(rho_index, theta_index) = hough_space(rho_index, theta_index) + 1;
            end
        end
    end
end


figure
imagesc(theta, rho, hough_space);
title('Hough Transform');
xlabel('Theta');
ylabel('Rho');
colormap('gray');


%% detekujeme priesecniky houghovej ransformacie
r = [];
c = [];
[max_in_col, row_number] = max(hough_space);
[rows, cols] = size(image);
difference = 180;
thresh = max(max(hough_space)) - difference ;
for i = 1:size(max_in_col, 2)
   if max_in_col(i) > thresh
       c(end + 1) = i;
       r(end + 1) = row_number(i);
   end
end

hold on;
plot(theta(c), rho(r),'rx');
hold off;


%% deteguje ciary podla vrcholov
figure
imagesc(image);
colormap(gray);
hold on;

for i = 1:size(c,2)
    THETA = theta(c(i));
    RHO = rho(r(i));
    rov_cast1 = -(cos(THETA)/sin(THETA));
    rov_cast2 = RHO/sin(THETA);
    x = 1:cols;
    plot(rov_cast1*x+rov_cast2, x);
    hold on;
end

