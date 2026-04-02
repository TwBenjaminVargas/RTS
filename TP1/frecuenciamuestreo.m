% --- PUNTO 2: SENAL PARAMETRIZADA POR MUESTRAS ---

% 1. Definicion de parametros (Puedes cambiar estos valores)
A = 2.5;            % Amplitud (Altura de la onda)
f = 10;             % Frecuencia de la senal en Hz (10 ciclos por segundo)
fs = 200;           % Frecuencia de muestreo en Hz (puntos por segundo)
N = 100;            % Cantidad de muestras del registro (puntos totales)

% 2. Generacion de los ejes
% El intervalo entre muestras es 1/fs
% Creamos un vector de 0 hasta N-1 muestras
n = 0 : N-1;
t = n / fs;         % Tiempo real en segundos para cada muestra

% 3. Generacion de la senal
% Formula: A * sin(2 * pi * f * t)
s_digital = A * sin(2 * pi * f * t);

% 4. Graficacion
figure(2);

% Usamos 'stem' porque es la forma correcta de representar
% una senal "digital" o discreta (puntos con palitos)
stem(t, s_digital, 'filled', 'MarkerSize', 4);

% Configuracion del grafico
title(['Senal Digital: f=', num2str(f), 'Hz, fs=', num2str(fs), 'Hz, N=', num2str(N)]);
xlabel('Tiempo [segundos]');
ylabel('Amplitud');
grid on;

% Limitar el eje X para que se vea ordenado
xlim([0, t(end)]);
