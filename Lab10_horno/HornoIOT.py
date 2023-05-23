import tkinter as tk
from PIL import ImageTk, Image
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from io import BytesIO
from tkinter import ttk
import requests
import mysql.connector
mydb = mysql.connector.connect(
    host="localhost",
    user="root",
    password="",
    database="PRACTICA8db"
)

def load_image_from_url(url, size):
    response = requests.get(url)
    img = Image.open(BytesIO(response.content))
    img = img.resize(size, Image.Resampling.LANCZOS)
   # img = img.resize(size, Image.ANTIALIAS)  # Redimensionar la imagen
    return ImageTk.PhotoImage(img)

def show_inicio():
    global root
    root.destroy()
    root = tk.Tk()
    # color de fondo en hexadecimal
    root.configure(bg='#EEC4F6')


    frame0 = tk.Frame(root)
    frame0.pack(side=tk.TOP, pady=10)

    frame1 = tk.Frame(root)
    frame1.pack(side=tk.TOP, pady=10)

    frame2 = tk.Frame(root)
    frame2.pack(side=tk.TOP, pady=10)

    #LOGO
    url0="https://i.postimg.cc/CKdwwBf4/chic.png"
    size_logo= (100, 70)
    image0 = load_image_from_url(url0, size_logo)
    log = tk.Label(frame0, image=image0, compound=tk.RIGHT, fg="#884395")
    log.pack(side=tk.LEFT, pady=0, padx=0)
    # Crear el título
    title = tk.Label(frame0, text="Practica 8 -- Horno IoT", font=("Arial", 20), fg="#884395")
    title.pack(side=tk.TOP, pady=0)
    title.configure(bg='#EEC4F6')





    # Tamaño de las imágenes
    size = (50, 50)

    # URL de las imágenes de los programadores
    url1 = "https://i.ibb.co/KhqqTYS/m2.jpg"
    url2 = "https://i.ibb.co/ys85qzp/foto-Paola.jpg"
    url3 = "https://i.postimg.cc/MTmVDwZh/dam.jpg"

    # Cargar las imágenes de los programadores
    image1 = load_image_from_url(url1, size)
    image2 = load_image_from_url(url2, size)
    image3 = load_image_from_url(url3, size)

    # Mostrar las imágenes y los nombres de los programadores
    label1 = tk.Label(frame2, image=image1, text="Mayra Vadez\nmayra.valdez@ucb.edu.bo", compound=tk.TOP)
    label1.pack(side=tk.LEFT, padx=10)

    label2 = tk.Label(frame2, image=image2, text="Paola Quispe\npaola.quispe@ucb.edu.bo", compound=tk.TOP)
    label2.pack(side=tk.LEFT, padx=10)

    label3 = tk.Label(frame2, image=image3, text="Damaris Blas\ndamaris.blas@ucb.edu.bo", compound=tk.TOP)
    label3.pack(side=tk.LEFT, padx=10)
    # URL del logo
    url4 = "https://i.postimg.cc/jd9Jg2MB/fotohorno.jpg"

    # Cargar la imagen más grande
    size_big = (200, 200)
    image4 = load_image_from_url(url4, size_big)

    # Mostrar la imagen grande
    label4 = tk.Label(frame1, image=image4)
    label4.pack(side=tk.TOP, padx=10)

    # Crear un botón para mostrar la gráfica
    show_table_button = tk.Button(frame1, text="Tabla", command=show_table)
    show_table_button.pack(side=tk.TOP, padx=10)
    show_graph_button = tk.Button(frame1, text="Mostrar gráfica", command=show_graph)
    show_graph_button.pack(side=tk.TOP, pady=10)
    show_energy_info_button = tk.Button(frame1, text="Mostrar información de consumo energético",
                                        command=show_energy_info)
    show_energy_info_button.pack(side=tk.TOP, pady=10)

    root.mainloop()
def get_seconds(hora):
    partes = hora.split(':')
    horas = int(partes[0])
    minutos = int(partes[1])
    segundos = int(partes[2])

    total_segundos = (horas * 3600) + (minutos * 60) + segundos
    return total_segundos

def show_graph():
    global root
    root.destroy()
    root = tk.Tk()

    frame2 = tk.Frame(root)
    frame2.pack(side=tk.TOP, pady=10)

    # Obtener los datos de la tabla TablaHorno
    mycursor = mydb.cursor()
    mycursor.execute("SELECT HoraActual, Calefactor, Enfriador, Set_Point, TempProceso, SensorLm35 FROM tablahorno")
    result = mycursor.fetchall()

    # Obtener los datos de los campos específicos para la graficación
    horas = []
    calefactor = []
    enfriador = []
    set_point = []
    sensor = []

    for row in result:
        horas.append(get_seconds(str(row[0])))
        calefactor.append(row[1])
        enfriador.append(row[2])
        set_point.append(row[3])
        sensor.append(row[4])

    # Crear un gráfico con los datos de la base de datos
    fig, ax = plt.subplots(figsize=(5, 4), dpi=100)
    ax.plot(horas, calefactor, label="Calefactor")
    ax.plot(horas, enfriador, label="Enfriador")
    ax.plot(horas, set_point, label="Set_Point")
    ax.plot(horas, sensor, label="SensorLM35")
    ax.set_xlabel("Hora Actual")
    ax.set_ylabel("Valor")
    ax.legend()
    ax.grid(True)

    # Añadir el gráfico a la ventana de Tkinter
    canvas = FigureCanvasTkAgg(fig, master=frame2)
    canvas.draw()
    canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

    # Crear un botón para volver a la pantalla de los programadores
    show_inicio_button = tk.Button(frame2, text="Volver al Inicio", command=show_inicio)
    show_inicio_button.pack(side=tk.TOP, pady=10)

    root.mainloop()

def show_table():
    global root
    root.destroy()
    root = tk.Tk()

    frame = tk.Frame(root)
    frame.pack(side=tk.TOP, pady=10)

    # Obtener los datos de la tabla TablaHorno
    mycursor = mydb.cursor()
    mycursor.execute("SELECT * FROM TablaHorno")
    result = mycursor.fetchall()

    # Crear una etiqueta para mostrar los valores de la tabla
    table_label = tk.Label(frame, text="Valores de la tabla TablaHorno", font=("Arial", 16))
    table_label.pack()

    # Crear el Treeview con las columnas correspondientes a tu tabla
    cols = ('NumReg', 'Horno_Id', 'Zona', 'Fecha', 'HoraActual', 'Calefactor', 'Enfriador', 'Set_Point', 'TempProceso', 'SensorLM35')
    listBox = ttk.Treeview(frame, columns=cols, show='headings')

    # Configurar las cabeceras de las columnas
    for col in cols:
        listBox.heading(col, text=col)

    # Insertar los valores en la tabla
    for row in result:
        listBox.insert("", "end", values=row)

    listBox.pack()

    # Crear un botón para volver a la pantalla de los programadores
    show_inicio_button = tk.Button(frame, text="Volver al inicio", command=show_inicio)
    show_inicio_button.pack(side=tk.TOP, pady=10)

    root.mainloop()
def show_energy_info():
    global root
    root.destroy()
    root = tk.Tk()

    frame = tk.Frame(root)
    frame.pack(side=tk.TOP, pady=10)

    # Obtener los datos de la tabla TablaHorno
    mycursor = mydb.cursor()
    mycursor.execute("SELECT HoraActual, Calefactor, Enfriador, Set_Point, SensorLM35 FROM tablahorno")
    result = mycursor.fetchall()

    # Calcular tiempo encendido y apagado
    total_encendido = 0
    total_apagado = 0
    estado_previo = 0

    for row in result:
        estado_actual = row[1] # El calefactor es el segundo campo
        if estado_actual != estado_previo:
            if estado_actual == 1: # El estado 1 significa encendido
                total_encendido += 1
            else:
                total_apagado += 1
        estado_previo = estado_actual

    porcentaje_encendido = (total_encendido / (total_encendido + total_apagado)) * 100 if total_encendido + total_apagado != 0 else 0

    # Calcular consumo energético
    potencia_foco = 0.1 # kW
    tiempo_encendido = total_encendido / 60 # En horas
    consumo_energetico = potencia_foco * tiempo_encendido

    # Mostrar la información de consumo energético
    energy_info_label = tk.Label(frame, text=f"Tiempo encendido (horas): {tiempo_encendido}\n"
                                              f"Porcentaje de tiempo encendido: {porcentaje_encendido}%\n"
                                              f"Consumo energético (kWh): {consumo_energetico}",
                                 font=("Arial", 16))
    energy_info_label.pack()

    # Crear un botón para volver a la pantalla de los programadores
    show_inicio_button = tk.Button(frame, text="Volver al inicio", command=show_inicio)
    show_inicio_button.pack(side=tk.TOP, pady=10)

    root.mainloop()



root = tk.Tk()
show_inicio()