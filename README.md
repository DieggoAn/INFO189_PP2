#Prueba practica 2 INFO189


## Desripcion

Este es un programa que sirve como buscador de las palabras con mayor recurrencia presente en una serie de libros, existen 3 programas, el backend es un programa que busca y entrega palabras y su recurrencia de un archivo file.txt que sirve como indice de recurrencia de palabras en varios libros, el cache es un programa que guarda en cache palabras que han sido previamente buscadas, si no las encuentra consulta con el backend, frontend es el programa en el cual el usuario interactua, en este se realiza la busqueda de palabras presentes en los libros

##Como compilar

Ubicarse en la carpeta root y ejecutar el siguiente comando
'''
make all
'''

##Como usar

Despues de haber compilado todos los programas se deben ejecutar en este orden

'''
./Backend/backend
./Cache/memcache
./Frontend/searcher

'''

