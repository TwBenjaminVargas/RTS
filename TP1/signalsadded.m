% --- PUNTO 3: SUMA DE DOS SENALES ---

% 1. Parametros compartidos
Fs = 1000;           % Frecuencia de muestreo (1000 puntos por segundo)
T = 1;               % Duracion de 1 segundo
t = 0 : 1/Fs : T;    % Vector de tiempo
A = 1;               % Amplitud igual para ambas

% 2. Definimos dos frecuencias diferentes
f1 = 5;              % Senal lenta (5 Hz)
f2 = 40;             % Senal rapida (40 Hz)

% 3. Generamos las dos senales por separado
s1 = A * sin(2 * pi * f1 * t);
s2 = A * sin(2 * pi * f2 * t);

% 4. Realizamos la SUMA (punto a punto)
s_resultante = s1 + s2;

% 5. Graficacion
figure(3);

% Grafico de la primera senal (Lenta)
subplot(3,1,1);
plot(t, s1, 'b');
title('Senal 1: 5 Hz');
grid on;

% Grafico de la segunda senal (Rapida)
subplot(3,1,2);
plot(t, s2, 'g');
title('Senal 2: 40 Hz');
grid on;

% Grafico de la SUMA RESULTANTE
subplot(3,1,3);
plot(t, s_resultante, 'r', 'LineWidth', 1.5);
title('Suma Resultante (s1 + s2)');
xlabel('Tiempo [segundos]');
grid on;
