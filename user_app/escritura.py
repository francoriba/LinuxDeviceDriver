file_path = "/dev/gpio_device"  # Reemplaza con la ruta de tu archivo

# Solicitar al usuario el texto a escribir en el archivo
texto1 = "out"
try:
    with open(file_path, "w") as archivo:
        archivo.write(texto1)
    print("Escritura exitosa")
except IOError:
    print("No se pudo escribir en el archivo")
