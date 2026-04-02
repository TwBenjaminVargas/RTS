% --- PUNTO 1: SEÑAL SINUSOIDAL Y DIFERENTES RESOLUCIONES ---

% 1. Configuración de parámetros iniciales
Fs = 1000;           % Frecuencia de muestreo
T = 1;               % Tiempo total
t = 0 : 1/Fs : T;    % Vector de tiempo
f = 5;               % Frecuencia de la onda
A = 1;               % Amplitud

% 2. Generación de la senal original
senal_pura = A * sin(2 * pi * f * t);

% 3. Simulación de diferentes resoluciones
bits_baja = 2;
niv_baja = 2^bits_baja - 1;
% Formula de cuantizacion
senal_baja = round((senal_pura + A) * niv_baja / (2*A)) * (2*A) / niv_baja - A;

bits_media = 4;
niv_media = 2^bits_media - 1;
senal_media = round((senal_pura + A) * niv_media / (2*A)) * (2*A) / niv_media - A;

% 4. Creación de gráficos
figure(1);

% Gráfico Superior
subplot(2,1,1);
plot(t, senal_pura, 'r--'); hold on;
stairs(t, senal_baja, 'b', 'LineWidth', 2);
title(['Resolución Baja: ', num2str(bits_baja), ' bits']);
ylabel('Amplitud');
grid on;

% Gráfico Inferior
subplot(2,1,2);
plot(t, senal_pura, 'r--'); hold on;
stairs(t, senal_media, 'g', 'LineWidth', 2);
title(['Resolución Media: ', num2str(bits_media), ' bits']);
xlabel('Tiempo (segundos)');
ylabel('Amplitud');
grid on;
