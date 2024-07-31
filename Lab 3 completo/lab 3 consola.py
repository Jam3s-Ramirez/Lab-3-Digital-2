import tkinter as tk
from tkinter import font as tkfont
import serial
import threading
import time

PORT = 'COM6'
BAUDRATE = 9600
ser = serial.Serial(PORT, BAUDRATE, timeout=1)

root = tk.Tk()
root.title("Control de Potenciómetros y LEDs")
root.geometry("750x600")
root.resizable(False, False)
root.configure(bg='#f2f2f2')

custom_font = tkfont.Font(family="Arial", size=12, weight="bold")
large_font = tkfont.Font(family="Arial", size=18, weight="bold")
button_font = tkfont.Font(family="Arial", size=14, weight="bold")

def crear_cuadro(root, texto, color_fondo, color_texto, width=200, height=50):
    frame = tk.Frame(root, bg=color_fondo, bd=2, relief='groove', padx=10, pady=10)
    canvas = tk.Canvas(frame, width=width, height=height, bg=color_fondo, highlightthickness=0, bd=0)
    canvas.create_text(width // 2, height // 2, text=texto, font=custom_font, fill=color_texto, tag="text")
    canvas.pack()
    return frame

# Recuadro en la parte superior
top_frame = tk.Frame(root, bg='#f2f2f2')
top_frame.pack(pady=10, fill=tk.X)

titulo_label = tk.Label(top_frame, text="Poslab 3", font=large_font, bg='#4caf50', fg='#ffffff', padx=20, pady=10, relief='flat')
titulo_label.pack(pady=(10, 20))

# Frame para los cuadros de potenciómetros
pot_frame = tk.Frame(root, bg='#f2f2f2')
pot_frame.pack(pady=20, fill=tk.X)

p1_frame = crear_cuadro(pot_frame, "P1: 0", '#ffffff', '#000000')
p2_frame = crear_cuadro(pot_frame, "P2: 0", '#ffffff', '#000000')

p1_frame.grid(row=0, column=0, padx=50, pady=10)
p2_frame.grid(row=0, column=1, padx=50, pady=10)

pot_frame.grid_columnconfigure(0, weight=1)
pot_frame.grid_columnconfigure(1, weight=1)

# Frame para la parte media (botones y valor de LEDs)
middle_frame = tk.Frame(root, bg='#f2f2f2')
middle_frame.pack(pady=20, fill=tk.X)

led_value_label_var = tk.StringVar(value="Valor de LEDs: 0")
led_value_label = tk.Label(middle_frame, textvariable=led_value_label_var, font=large_font, bg='#4caf50', fg='#ffffff', padx=20, pady=10, relief='flat')
led_value_label.grid(row=0, column=0, padx=20, pady=20)

led_value_entry = tk.Entry(middle_frame, font=large_font, width=10, bd=0, relief='flat', highlightthickness=2, highlightcolor='#4caf50')
led_value_entry.grid(row=0, column=1, padx=20, pady=20)

def crear_boton(root, text, comando, color_fondo, color_texto):
    boton = tk.Button(root, text=text, command=comando, font=button_font, bg=color_fondo, fg=color_texto, relief='raised', width=15, height=2)
    return boton

send_led_button = crear_boton(middle_frame, "Enviar a LEDs", lambda: enviar_comando_led(), '#2196f3', '#ffffff')
send_led_button.grid(row=1, column=0, padx=20, pady=10)

update_pots_button = crear_boton(middle_frame, "Lectura Pots", lambda: actualizar_potenciometros(), '#2196f3', '#ffffff')
update_pots_button.grid(row=1, column=1, padx=20, pady=10)

def actualizar_potenciometros():
    ser.write(b'm')

def enviar_comando_led():
    try:
        valor = int(led_value_entry.get())
        if 0 <= valor <= 255:
            ser.write(f"n{valor}\n".encode())
            led_value_label_var.set(f"Valor de LEDs: {valor}")
        else:
            led_value_label_var.set("Valor fuera de rango (0-255)")
    except ValueError:
        led_value_label_var.set("Entrada no válida")
    finally:
        led_value_entry.delete(0, tk.END)

def leer_datos_serial():
    while True:
        if ser.in_waiting > 0:
            try:
                data = ser.readline().decode('utf-8').strip()
                if data.startswith('P1:'):
                    p1_value = data.split(":")[1].strip()
                    root.after(0, lambda: actualizar_texto_canvas(p1_frame, f"P1: {p1_value}"))
                elif data.startswith('P2:'):
                    p2_value = data.split(":")[1].strip()
                    root.after(0, lambda: actualizar_texto_canvas(p2_frame, f"P2: {p2_value}"))
                elif data.startswith('Valor LEDs:'):
                    valor_leds = data.split(":")[1].strip()
                    root.after(0, lambda: led_value_label_var.set(f"Valor de LEDs: {valor_leds}"))
            except UnicodeDecodeError:
                pass
        time.sleep(0.1)

def actualizar_texto_canvas(frame, texto):
    canvas = frame.winfo_children()[0]
    canvas.delete("text")
    canvas.create_text(canvas.winfo_width() // 2, canvas.winfo_height() // 2, text=texto, font=custom_font, fill='#000000', tag="text")

def iniciar_thread():
    thread = threading.Thread(target=leer_datos_serial, daemon=True)
    thread.start()

iniciar_thread()

root.mainloop()
