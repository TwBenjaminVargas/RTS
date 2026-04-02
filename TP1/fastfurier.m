% --- PUNTO 4: ANALISIS DE FRECUENCIA (FFT) ---

% RECREAMOS LOS DATOS DEL PUNTO 1 (5 Hz, Fs=1000)
Fs1 = 1000; T1 = 1; t1 = 0:1/Fs1:T1; f1 = 5; A1 = 1;
s1 = A1 * sin(2 * pi * f1 * t1);

% RECREAMOS LOS DATOS DEL PUNTO 2 (10 Hz, Fs=200, N=128)
fs2 = 200; N2 = 128; n2 = 0:N2-1; t2 = n2/fs2; f2 = 10; A2 = 2;
s2 = A2 * sin(2 * pi * f2 * t2);

% --- CALCULO DE FFT PARA SENAL 1 ---
L1 = length(s1);
X1 = fft(s1);              % Calculo de la FFT
P2_1 = abs(X1/L1);         % Magnitud normalizada (ambos lados)
P1_1 = P2_1(1:L1/2+1);     % Tomamos solo la mitad positiva
P1_1(2:end-1) = 2*P1_1(2:end-1); % Ajuste de amplitud por energia
freq1 = Fs1 * (0:(L1/2)) / L1;   % Eje de frecuencias en Hz

% --- CALCULO DE FFT PARA SENAL 2 ---
L2 = length(s2);
X2 = fft(s2);
P2_2 = abs(X2/L2);
P1_2 = P2_2(1:L2/2+1);
P1_2(2:end-1) = 2*P1_2(2:end-1);
freq2 = fs2 * (0:(L2/2)) / L2;

% --- GRAFICACION ---
figure(4);

% Espectro Senal 1
subplot(2,1,1);
stem(freq1, P1_1, 'b', 'LineWidth', 1.5);
title('Espectro de Frecuencia - Punto 1 (5 Hz)');
ylabel('Amplitud');
grid on;
xlim([0 20]); % Hacemos zoom en las frecuencias bajas para ver el pico

% Espectro Senal 2
subplot(2,1,2);
stem(freq2, P1_2, 'r', 'LineWidth', 1.5);
title('Espectro de Frecuencia - Punto 2 (10 Hz)');
xlabel('Frecuencia [Hz]');
ylabel('Amplitud');
grid on;
xlim([0 30]); % Hacemos zoom
