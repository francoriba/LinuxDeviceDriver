import RPi.GPIO as GPIO
import time
import matplotlib.pyplot as plt

# Configuración de pines GPIO
pin_trigger = 23
pin_echo = 24

GPIO.setmode(GPIO.BCM)
GPIO.setup(pin_trigger, GPIO.OUT)
GPIO.setup(pin_echo, GPIO.IN)

def medir_distancia():
    # Generar pulso de trigger
    GPIO.output(pin_trigger, GPIO.HIGH)
    time.sleep(0.00001)
    GPIO.output(pin_trigger, GPIO.LOW)
    
    # Esperar a que el pin de echo se active
    while GPIO.input(pin_echo) == 0:
        pass
    
    inicio_pulso = time.time()
    
    # Esperar a que el pin de echo se desactive
    while GPIO.input(pin_echo) == 1:
        pass
    
    fin_pulso = time.time()
    
    # Calcular duración del pulso
    duracion_pulso = fin_pulso - inicio_pulso
    
    # Calcular distancia
    distancia = duracion_pulso * 17150
    
    # Redondear distancia a dos decimales
    distancia = round(distancia, 2)
    
    return distancia

def graficar_distancia_tiempo(tiempo, distancias):
    plt.plot(tiempo, distancias)
    plt.xlabel('Tiempo (s)')
    plt.ylabel('Distancia (cm)')
    plt.title('Gráfico de distancia en función del tiempo')
    plt.draw()
    plt.pause(0.001)
    plt.clf()

try:
    tiempo = []
    distancias = []
    
    plt.ion()
    fig = plt.figure()
    
    while True:
        distancia = medir_distancia()
        print("Distancia:", distancia, "cm")
        tiempo.append(time.time())
        distancias.append(distancia)
        time.sleep(0.1)
        
        # Graficar distancia en función del tiempo en tiempo real
        graficar_distancia_tiempo(tiempo, distancias)

except KeyboardInterrupt:
    print("Programa terminado por el usuario")

finally:
    plt.ioff()
    plt.close(fig)
    GPIO.cleanup()
