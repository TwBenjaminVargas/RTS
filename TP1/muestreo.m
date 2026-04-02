% --- PUNTO 5: GENERACION DE ARCHIVO DE CABECERA .H ---

% 1. Definimos una señal de ejemplo (usaremos la del Punto 2)
A = 2;              % Amplitud
f = 10;             % Frecuencia 10 Hz
fs = 200;           % Frecuencia de muestreo 200 Hz
N = 128;            % Cantidad de muestras
n = 0:N-1;
t = n/fs;
s_digital = A * sin(2 * pi * f * t); % La señal a exportar

% 2. Crear y abrir el archivo .h
% 'w' significa "write" (escritura). Si el archivo no existe, lo crea.
nombre_archivo = 'senal_datos.h';
id = fopen(nombre_archivo, 'w');

% 3. Escribir el contenido del archivo con formato de C
% Usamos fprintf para escribir texto dentro del archivo
fprintf(id, '/* Archivo de cabecera generado automaticamente en Octave */\n');
fprintf(id, '/* Registro de senal sinusoidal: f=10Hz, fs=200Hz */\n\n');

% Definimos el array (vector) en lenguaje C
fprintf(id, 'float senal_muestras[%d] = {\n', N);

for i = 1:N
    if i == N
        % La ultima muestra no lleva coma al final
        fprintf(id, '    %f\n', s_digital(i));
    else
        % Las demas muestras llevan coma para separar los elementos del array
        fprintf(id, '    %f,\n', s_digital(i));
    end
end

fprintf(id, '};\n');

% 4. Cerrar el archivo (MUY IMPORTANTE para que se guarde en el disco)
fclose(id);

disp(['Exito: Se ha generado el archivo "', nombre_archivo, '" en tu carpeta de trabajo.']);
