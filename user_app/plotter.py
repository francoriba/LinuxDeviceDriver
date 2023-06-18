import RPi.GPIO as GPIO
import time
import matplotlib.pyplot as plt

# Configuración de pines GPIO
pin_trigger_s1 = 23 #echo24
pin_trigger_s2 = 13 #echo16
pin_pulsador = 12


GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)  # Deshabilitar las advertencias de GPIO
GPIO.setup(pin_trigger_s1, GPIO.OUT) #trigger senstor 1 como salida
GPIO.setup(pin_trigger_s2, GPIO.OUT)  #trigger sensor 2 como salida
GPIO.setup(pin_pulsador, GPIO.IN, pull_up_down = GPIO.PUD_UP)


# CDF de lectura
file_path = "/dev/gpio_device"

# Listas para almacenar los valores de tiempo y distancia
tiempos = []
distancias = []

def enviar_comando(comando):
    try:
        with open(file_path, "w") as archivo:
            archivo.write(comando)
        print("Escritura exitosa:", comando)
    except IOError:
        print("No se pudo escribir en el archivo")

def decodificar_lectura(valor_leido):
    if len(valor_leido) > 1: #es un comando
        return None
    else:      # Es un valor de pin
        return int(valor_leido)

def resetear_grafico():
    plt.clf()  # Borrar el contenido del gráfico
    tiempos.clear()
    distancias.clear()
    plt.xlabel('Tiempo (sec)')
    plt.ylabel('Distancia (cm)')
    plt.title('Mediciones de distancia con sensor ultrasónico')
    plt.grid(True)  # Agregar grid al gráfico

def pulsador_presionado(channel):
        print("Pulsador presionado")
    
def medir_distancia():
    # Leer el valor del pin echo desde el archivo
    with open(file_path, "r") as archivo:
        valor_leido = archivo.read().strip()

    pin_echo_value = decodificar_lectura(valor_leido)

    if pin_echo_value is None:
        pin_echo_value = 1 #ver
        return None
    
    # Generar pulso de trigger para el sensor 1
    GPIO.output(pin_trigger_s1, GPIO.HIGH)
    time.sleep(0.00001)
    GPIO.output(pin_trigger_s1, GPIO.LOW)

    GPIO.output(pin_trigger_s2, GPIO.HIGH)
    time.sleep(0.00001)
    GPIO.output(pin_trigger_s2, GPIO.LOW)
    
    # Esperar a que el pin de echo se active
    while pin_echo_value == 0:
        with open(file_path, "r") as archivo:
            valor_leido = archivo.read().strip()
        pin_echo_value = decodificar_lectura(valor_leido)
    inicio_pulso = time.time()
    
    # Esperar a que el pin de echo se desactive
    while pin_echo_value == 1:
        with open(file_path, "r") as archivo:
            valor_leido = archivo.read().strip()
        pin_echo_value = decodificar_lectura(valor_leido)
    
    fin_pulso = time.time()
    
    # Calcular duración del pulso
    duracion_pulso = fin_pulso - inicio_pulso
    
    # Calcular distancia
    distancia = duracion_pulso * 17150
    
    # Redondear distancia a dos decimales
    distancia = round(distancia, 2)
    
    return distancia

#GPIO.add_event_detect(pin_pulsador, GPIO.RISING, callback = pulsador_presionado, bouncetime = 50)
enviar_comando("in")
enviar_comando("in2")

flag = GPIO.input(pin_pulsador) #1

try:
    while True:


        if GPIO.input(pin_pulsador) == 0: 
            if flag == 1: # si el pin pasa de 1 a 0
                 flag = 0
                 enviar_comando("toggle")
                 if (pin_trigger_s1 == 23):
                    pin_trigger_s1 = 13
                    pin_trigger_s2 = 23
                 else:
                    pin_trigger_s1 = 23
                    pin_trigger_s2 = 13
                 resetear_grafico()

        else:
             if flag == 0: # si el pin pasa de 0 a 1
                flag = 1
                enviar_comando("toggle")
                resetear_grafico()


        distancia = medir_distancia()
        tiempo_actual = time.time()
        
        print("Distancia:", distancia, "cm")
        
        # Agregar los valores actuales a las listas
        tiempos.append(tiempo_actual)
        distancias.append(distancia)
        
        # Actualizar el gráfico
        plt.plot(tiempos, distancias)
        plt.xlabel('Tiempo(sec)')
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
