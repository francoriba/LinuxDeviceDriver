import RPi.GPIO as GPIO
import time
import matplotlib.pyplot as plt

# Configuración de pines GPIO
pin_trigger_s1 = 23 #echo24
pin_trigger_s2 = 13 #echo22

GPIO.setmode(GPIO.BCM)
GPIO.setup(pin_trigger_s1, GPIO.OUT)
GPIO.setup(pin_trigger_s2, GPIO.OUT)

# CDF de lectura
file_path = "/dev/gpio_device"  

texto1 = "in" #comando que hay que escribir en el archivo para que el driver configure como entrada el pin echo 24
try:
    with open(file_path, "w") as archivo:
        archivo.write(texto1)
    print("Escritura exitosa")
except IOError:
    print("No se pudo escribir en el archivo")

def medir_distancia():
    # Leer el valor del pin echo desde el archivo
    with open(file_path, "r") as archivo:
        pin_echo_value = int(archivo.read())

        #si lee "request_toggle"
        #escribe "toggle"
        #en el cdd si se lee "toggle" se pasan a escribir valores del otro sensor
    
    # Generar pulso de trigger
    GPIO.output(pin_trigger_s1, GPIO.HIGH)
    time.sleep(0.00001)
    GPIO.output(pin_trigger_s1, GPIO.LOW)
    
    # Esperar a que el pin de echo se active
    while pin_echo_value == 0:
        with open(file_path, "r") as archivo:
            pin_echo_value = int(archivo.read())
    
    inicio_pulso = time.time()
    
    # Esperar a que el pin de echo se desactive
    while pin_echo_value == 1:
        with open(file_path, "r") as archivo:
            pin_echo_value = int(archivo.read().strip())
    
    fin_pulso = time.time()
    
    # Calcular duración del pulso
    duracion_pulso = fin_pulso - inicio_pulso
    
    # Calcular distancia
    distancia = duracion_pulso * 17150
    
    # Redondear distancia a dos decimales
    distancia = round(distancia, 2)
    
    return distancia

# Listas para almacenar los valores de tiempo y distancia
tiempos = []
distancias = []

try:
    while True:
        distancia = medir_distancia()
        tiempo_actual = time.time()
        
        print("Distancia:", distancia, "cm")
        
        # Agregar los valores actuales a las listas
        tiempos.append(tiempo_actual)
        distancias.append(distancia)
        
        # Actualizar el gráfico
        plt.plot(tiempos, distancias)
        plt.xlabel('Tiempo')
        plt.ylabel('Distancia (cm)')
        plt.title('Mediciones de distancia con sensor ultrasónico')
        plt.grid(True)  # Agregar grid al gráfico
        plt.pause(1)
        time.sleep(1)

except KeyboardInterrupt:
    print("Programa terminado por el usuario")
finally:
    GPIO.cleanup()

# Mostrar el gráfico final
plt.show()
