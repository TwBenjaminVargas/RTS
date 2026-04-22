import sys
import math
import tkinter as tk
import threading

# Variables globales para los datos y la orientación
ax_raw, ay_raw, az_raw = 0.0, 0.0, 0.0
roll_deg, pitch_deg = 0.0, 0.0
vx, vy, vz = [1, 0, 0], [0, 1, 0], [0, 0, 1]

def leer_datos_sensor():
    """Lee la salida del pipe en un hilo separado."""
    global vx, vy, vz, ax_raw, ay_raw, az_raw, roll_deg, pitch_deg
    for linea in sys.stdin:
        try:
            ax_acc, ay_acc, az_acc = map(float, linea.strip().split())
            
            if ax_acc == 0 and ay_acc == 0 and az_acc == 0:
                continue

            # Guardar valores crudos para mostrar en pantalla
            ax_raw, ay_raw, az_raw = ax_acc, ay_acc, az_acc

            # 1. Calcular Pitch y Roll (en radianes)
            roll_rad = math.atan2(ay_acc, az_acc)
            pitch_rad = math.atan2(-ax_acc, math.sqrt(ay_acc**2 + az_acc**2))
            
            # Convertir a grados para la lectura humana
            roll_deg = math.degrees(roll_rad)
            pitch_deg = math.degrees(pitch_rad)

            # 2. Función de rotación para los ejes visuales
            def rotar(v, r, p):
                # Rotación Roll (X)
                y1 = v[1] * math.cos(r) - v[2] * math.sin(r)
                z1 = v[1] * math.sin(r) + v[2] * math.cos(r)
                x1 = v[0]
                # Rotación Pitch (Y)
                x2 = x1 * math.cos(p) + z1 * math.sin(p)
                z2 = -x1 * math.sin(p) + z1 * math.cos(p)
                return [x2, y1, z2]

            vx = rotar([1, 0, 0], roll_rad, pitch_rad)
            vy = rotar([0, 1, 0], roll_rad, pitch_rad)
            vz = rotar([0, 0, 1], roll_rad, pitch_rad)
            
        except ValueError:
            pass

def proyectar_a_2d(v):
    """Proyección isométrica simple para el Canvas."""
    centro_x, centro_y = 200, 200
    escala = 100
    px = centro_x + (v[0] - v[1] * 0.4) * escala
    py = centro_y - (v[2] - v[1] * 0.4) * escala
    return px, py

def actualizar_pantalla():
    """Actualiza los gráficos y los textos a 50 FPS."""
    cx, cy = 200, 200
    
    # Actualizar líneas de los ejes
    px, py = proyectar_a_2d(vx)
    canvas.coords(linea_x, cx, cy, px, py)
    
    px, py = proyectar_a_2d(vy)
    canvas.coords(linea_y, cx, cy, px, py)
    
    px, py = proyectar_a_2d(vz)
    canvas.coords(linea_z, cx, cy, px, py)
    
    # Actualizar el texto de datos en la esquina inferior derecha
    texto_datos = (
        f"X: {ax_raw:>5.2f} g\n"
        f"Y: {ay_raw:>5.2f} g\n"
        f"Z: {az_raw:>5.2f} g\n"
        f"-----------\n"
        f"Roll:  {roll_deg:>6.1f}°\n"
        f"Pitch: {pitch_deg:>6.1f}°"
    )
    canvas.itemconfigure(label_valores, text=texto_datos)
    
    root.after(20, actualizar_pantalla)

# --- Configuración de la Ventana ---
root = tk.Tk()
root.title("Visor MPU6050")
root.geometry("400x400")
root.resizable(False, False)

canvas = tk.Canvas(root, width=400, height=400, bg="black", highlightthickness=0)
canvas.pack(fill="both", expand=True)

# Dibujar ejes (inicialmente en el centro)
linea_x = canvas.create_line(0, 0, 0, 0, fill="#FF4444", width=3, capstyle=tk.ROUND)
linea_y = canvas.create_line(0, 0, 0, 0, fill="#44FF44", width=3, capstyle=tk.ROUND)
linea_z = canvas.create_line(0, 0, 0, 0, fill="#4444FF", width=3, capstyle=tk.ROUND)

# Etiquetas de leyenda (Superior Izquierda)
canvas.create_text(10, 10, text="Rojo: Eje X", fill="#FF4444", anchor="nw", font=("Courier", 10))
canvas.create_text(10, 25, text="Verde: Eje Y", fill="#44FF44", anchor="nw", font=("Courier", 10))
canvas.create_text(10, 40, text="Azul: Eje Z", fill="#4444FF", anchor="nw", font=("Courier", 10))

# Etiqueta de Datos (Inferior Derecha)
# Usamos una fuente monoespaciada (Courier) para que los números no salten al cambiar
label_valores = canvas.create_text(
    390, 390, 
    text="", 
    fill="white", 
    anchor="se", 
    justify=tk.RIGHT,
    font=("Courier", 11, "bold")
)

# Iniciar hilo de lectura y loop de animación
threading.Thread(target=leer_datos_sensor, daemon=True).start()
actualizar_pantalla()

root.mainloop()